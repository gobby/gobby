/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2014 Armin Burgmeier <armin@arbur.net>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "commands/browser-context-commands.hpp"
#include "operations/operation-open-multiple.hpp"
#include "dialogs/connection-info-dialog.hpp"
#include "util/file.hpp"
#include "util/i18n.hpp"

#include <gtkmm/builder.h>
#include <giomm/file.h>
#include <glibmm/fileutils.h>

#include <libinfgtk/inf-gtk-permissions-dialog.h>
#include <libinfinity/common/inf-cert-util.h>

// TODO: Use file tasks for the commands, once we made them public

namespace
{
	std::string make_safe_filename(const Glib::ustring& cn)
	{
		Glib::ustring output(cn);

		for(Glib::ustring::iterator iter = output.begin();
			iter != output.end(); )
		{
			if(Glib::Unicode::isspace(*iter) || *iter == '/' ||
			   *iter == '\\' || *iter == ':' || *iter == '?')
			{
				iter = output.erase(iter);
			}
			else
			{
				++iter;
			}
		}

		return output;
	}

	bool check_file(const std::string& filename)
	{
		return Glib::file_test(filename, Glib::FILE_TEST_EXISTS);
	}

	std::string make_certificate_filename(const Glib::ustring& cn,
	                                      const Glib::ustring& hostname)
	{
		const std::string basename =
			make_safe_filename(hostname) + "-" +
			make_safe_filename(cn);

		const std::string prefname =
			Gobby::config_filename(basename + ".pem");
		if(!check_file(prefname)) return prefname;

		for(unsigned int i = 2; i < 10000; ++i)
		{
			const std::string altname =
				Gobby::config_filename(
					basename + "-" +
					Glib::ustring::compose(
						"%1", i) + ".pem");

			if(!check_file(altname))
				return altname;
		}

		throw std::runtime_error(
			Gobby::_("Could not find a location "
			         "where to store the certificate"));
	}
}

Gobby::BrowserContextCommands::BrowserContextCommands(
	Gtk::Window& parent, InfIo* io, Browser& browser, FileChooser& chooser,
	Operations& operations, const CertificateManager& cert_manager,
	Preferences& prefs)
:
	m_parent(parent), m_io(io), m_browser(browser),
	m_file_chooser(chooser), m_operations(operations),
	m_cert_manager(cert_manager), m_preferences(prefs),
	m_popup_menu(NULL),

	m_action_group(Gio::SimpleActionGroup::create()),
	m_action_remove(m_action_group->add_action("remove")),
	m_action_connection_info(m_action_group->add_action("connection-info")),
	m_action_disconnect(m_action_group->add_action("disconnect")),
	m_action_create_account(m_action_group->add_action("create-account")),
	m_action_create_document(
		m_action_group->add_action("create-document")),
	m_action_create_directory(
		m_action_group->add_action("create-directory")),
	m_action_open_document(m_action_group->add_action("open-document")),
	m_action_permissions(m_action_group->add_action("permissions")),
	m_action_delete(m_action_group->add_action("delete"))
{
	g_object_ref(io);

	m_populate_popup_handler = g_signal_connect(
		m_browser.get_view(), "populate-popup",
		G_CALLBACK(on_populate_popup_static), this);

	gtk_widget_insert_action_group(
		GTK_WIDGET(m_browser.get_view()),
		"browser", G_ACTION_GROUP(m_action_group->gobj()));

	m_action_remove->signal_activate().connect(
		sigc::mem_fun(*this, &BrowserContextCommands::on_remove));
	m_action_connection_info->signal_activate().connect(
		sigc::mem_fun(*this,
			&BrowserContextCommands::on_connection_info));
	m_action_disconnect->signal_activate().connect(
		sigc::mem_fun(*this, &BrowserContextCommands::on_disconnect));
	m_action_create_account->signal_activate().connect(
		sigc::mem_fun(*this,
			&BrowserContextCommands::on_create_account));
	m_action_create_document->signal_activate().connect(
		sigc::bind(sigc::mem_fun(*this,
			&BrowserContextCommands::on_new), false));
	m_action_create_directory->signal_activate().connect(
		sigc::bind(sigc::mem_fun(*this,
			&BrowserContextCommands::on_new), true));
	m_action_open_document->signal_activate().connect(
		sigc::mem_fun(*this, &BrowserContextCommands::on_open));
	m_action_permissions->signal_activate().connect(
		sigc::mem_fun(*this,
			&BrowserContextCommands::on_permissions));
	m_action_delete->signal_activate().connect(
		sigc::mem_fun(*this, &BrowserContextCommands::on_delete));
}

Gobby::BrowserContextCommands::~BrowserContextCommands()
{
	g_signal_handler_disconnect(m_browser.get_view(),
	                            m_populate_popup_handler);

	g_object_unref(m_io);
}

void Gobby::BrowserContextCommands::on_populate_popup(Gtk::Menu* menu)
{
	// TODO: Can this happen? Should we close the old popup here?
	g_assert(m_popup_menu == NULL);
	g_assert(m_popup_watch.get() == NULL);

	// Cancel previous dialogs
	m_dialog.reset(NULL);
	m_dialog_watch.reset(NULL);

	InfBrowser* browser;
	InfBrowserIter iter;
	InfBrowserStatus status;

	if(!m_browser.get_selected_browser(&browser))
		return;

	g_object_get(G_OBJECT(browser), "status", &status, NULL);
	const bool has_iter = m_browser.get_selected_iter(browser, &iter);

	// Watch the node, and close the popup menu when the node
	// it refers to is removed.
	m_popup_watch.reset(new NodeWatch(browser, has_iter ? &iter : NULL));
	m_popup_watch->signal_node_removed().connect(sigc::mem_fun(
		*this, &BrowserContextCommands::on_menu_node_removed));

	// TODO: At the moment, action handlers access the browser item using
	// the m_popup_watch variable. We should change it such that
	// the selected item is passed as an action parameter, as two
	// strings: The browser store item name, and the path to the selected
	// node. This could then also easily allow browser actions to be
	// invoked via d-bus.

	Glib::RefPtr<Gtk::Builder> builder =
		Gtk::Builder::create_from_resource(
			"/de/0x539/gobby/ui/browser-context-menu.ui");

	if(!has_iter)
	{
		InfBrowserStatus browser_status;
		g_object_get(G_OBJECT(browser), "status",
		             &browser_status, NULL);

		Glib::RefPtr<Gio::Menu> menu_model =
			Glib::RefPtr<Gio::Menu>::cast_dynamic(
				builder->get_object(
					"browser-context-menu-inactive"));

		if(browser_status == INF_BROWSER_CLOSED)
		{
			menu->bind_model(menu_model, true);
		}
	}
	else
	{
		InfBrowserIter dummy_iter = iter;
		const bool is_subdirectory =
			inf_browser_is_subdirectory(browser, &iter);
		const bool is_toplevel =
			!inf_browser_get_parent(browser, &dummy_iter);

		m_action_connection_info->set_enabled(
			is_toplevel && status == INF_BROWSER_OPEN);
		m_action_disconnect->set_enabled(
			is_toplevel && INFC_IS_BROWSER(browser));
		m_action_create_account->set_enabled(
			is_toplevel && INFC_IS_BROWSER(browser));

		m_action_create_document->set_enabled(is_subdirectory);
		m_action_create_directory->set_enabled(is_subdirectory);
		m_action_open_document->set_enabled(is_subdirectory);
		m_action_delete->set_enabled(!is_toplevel);

		Glib::RefPtr<Gio::Menu> menu_model =
			Glib::RefPtr<Gio::Menu>::cast_dynamic(
				builder->get_object(
					"browser-context-menu-active"));

		menu->bind_model(menu_model, true);
	}

	m_popup_menu = menu;
	menu->signal_selection_done().connect(
		sigc::mem_fun(
			*this,
			&BrowserContextCommands::on_popdown));
}

void Gobby::BrowserContextCommands::on_menu_node_removed()
{
	g_assert(m_popup_menu != NULL);

	// This calls selection_done, causing the watch to be removed
	// and the m_popup_menu pointer to be reset.
	m_popup_menu->popdown();
}

void Gobby::BrowserContextCommands::on_popdown()
{
	m_popup_menu = NULL;
	m_popup_watch.reset(NULL);
}

// Action handlers

void Gobby::BrowserContextCommands::on_remove(const Glib::VariantBase& param)
{
	InfBrowser* browser = m_popup_watch->get_browser();

	InfBrowserStatus status;
	g_object_get(G_OBJECT(browser), "status", &status, NULL);

	g_assert(status == INF_BROWSER_CLOSED);

	// Reset the watch explicitly here in case we drop the last reference
	// to the browser.
	m_popup_watch.reset(NULL);
	m_browser.remove_browser(browser);
}

void Gobby::BrowserContextCommands::on_connection_info(
	const Glib::VariantBase& param)
{
	InfBrowser* browser = m_popup_watch->get_browser();

	m_dialog.reset(new ConnectionInfoDialog(m_parent, browser));
	m_dialog->add_button(_("_Close"), Gtk::RESPONSE_CLOSE);
	m_dialog->signal_response().connect(
		sigc::mem_fun(*this,
			&BrowserContextCommands::
				on_connection_info_response));
	m_dialog->present();
}

void Gobby::BrowserContextCommands::on_disconnect(
	const Glib::VariantBase& param)
{
	InfcBrowser* browser = INFC_BROWSER(m_popup_watch->get_browser());
	InfXmlConnection* connection = infc_browser_get_connection(browser);
	InfXmlConnectionStatus status;
	g_object_get(G_OBJECT(connection), "status", &status, NULL);

	if(status != INF_XML_CONNECTION_CLOSED &&
	   status != INF_XML_CONNECTION_CLOSING)
	{
		inf_xml_connection_close(connection);
	}
}

void Gobby::BrowserContextCommands::on_create_account(
	const Glib::VariantBase& param)
{
	InfBrowser* browser = m_popup_watch->get_browser();

	InfGtkAccountCreationDialog* dlg =
		inf_gtk_account_creation_dialog_new(
			m_parent.gobj(), static_cast<GtkDialogFlags>(0),
			m_io, browser);
	std::auto_ptr<Gtk::Dialog> account_creation_dialog(
		Glib::wrap(GTK_DIALOG(dlg)));

	account_creation_dialog->add_button(_("_Close"), Gtk::RESPONSE_CLOSE);

	account_creation_dialog->signal_response().connect(
		sigc::mem_fun(*this,
			&BrowserContextCommands::on_create_account_response));

	g_signal_connect(
		G_OBJECT(account_creation_dialog->gobj()), "account-created",
		G_CALLBACK(on_account_created_static), this);

	m_dialog = account_creation_dialog;
	m_dialog->present();
}

void Gobby::BrowserContextCommands::on_new(const Glib::VariantBase& param,
                                           bool directory)
{
	InfBrowser* browser = m_popup_watch->get_browser();
	const InfBrowserIter iter = *m_popup_watch->get_browser_iter();

	m_dialog_watch.reset(new NodeWatch(browser, &iter));
	m_dialog_watch->signal_node_removed().connect(sigc::mem_fun(
		*this, &BrowserContextCommands::on_dialog_node_removed));

	std::auto_ptr<EntryDialog> entry_dialog(
		new EntryDialog(
			m_parent,
			directory ? _("Choose a name for the directory")
			          : _("Choose a name for the document"),
			directory ? _("_Directory Name:")
			          : _("_Document Name:")));

	entry_dialog->add_button(_("_Cancel"), Gtk::RESPONSE_CANCEL);
	entry_dialog->add_button(_("C_reate"), Gtk::RESPONSE_ACCEPT);

	entry_dialog->set_text(directory ? _("New Directory")
	                                   : _("New Document"));
	entry_dialog->signal_response().connect(sigc::bind(
		sigc::mem_fun(*this,
			&BrowserContextCommands::on_new_response),
		browser, iter, directory));

	m_dialog = entry_dialog;
	m_dialog->present();
}

void Gobby::BrowserContextCommands::on_open(const Glib::VariantBase& param)
{
	InfBrowser* browser = m_popup_watch->get_browser();
	const InfBrowserIter iter = *m_popup_watch->get_browser_iter();

	m_dialog_watch.reset(new NodeWatch(browser, &iter));
	m_dialog_watch->signal_node_removed().connect(sigc::mem_fun(
		*this, &BrowserContextCommands::on_dialog_node_removed));

	std::auto_ptr<FileChooser::Dialog> file_dialog(
		new FileChooser::Dialog(
			m_file_chooser, m_parent,
			_("Choose a text file to open"),
			Gtk::FILE_CHOOSER_ACTION_OPEN));
	file_dialog->signal_response().connect(sigc::bind(
		sigc::mem_fun(*this,
			&BrowserContextCommands::on_open_response),
		browser, iter));

	file_dialog->set_select_multiple(true);

	m_dialog = file_dialog;
	m_dialog->present();
}

void Gobby::BrowserContextCommands::on_permissions(
	const Glib::VariantBase& param)
{
	InfBrowser* browser = m_popup_watch->get_browser();
	const InfBrowserIter iter = *m_popup_watch->get_browser_iter();

	m_dialog_watch.reset(new NodeWatch(browser, &iter));
	m_dialog_watch->signal_node_removed().connect(sigc::mem_fun(
		*this, &BrowserContextCommands::on_dialog_node_removed));

	InfGtkPermissionsDialog* dlg = inf_gtk_permissions_dialog_new(
		m_parent.gobj(), static_cast<GtkDialogFlags>(0),
		browser, &iter);
	std::auto_ptr<Gtk::Dialog> permissions_dialog(
		Glib::wrap(GTK_DIALOG(dlg)));

	permissions_dialog->add_button(_("_Close"), Gtk::RESPONSE_CLOSE);

	permissions_dialog->signal_response().connect(
		sigc::mem_fun(*this,
			&BrowserContextCommands::on_permissions_response));

	m_dialog = permissions_dialog;
	m_dialog->present();
}

void Gobby::BrowserContextCommands::on_delete(
	const Glib::VariantBase& param)
{
	InfBrowser* browser = m_popup_watch->get_browser();
	const InfBrowserIter* iter = m_popup_watch->get_browser_iter();

	m_operations.delete_node(browser, iter);
}

// Dialogs

void Gobby::BrowserContextCommands::on_dialog_node_removed()
{
	m_dialog.reset(NULL);
	m_dialog_watch.reset(NULL);
}

void Gobby::BrowserContextCommands::on_create_account_response(
	int response_id)
{
	m_dialog.reset(NULL);
	m_dialog_watch.reset(NULL);
}

void Gobby::BrowserContextCommands::on_connection_info_response(
	int response_id)
{
	m_dialog.reset(NULL);
	m_dialog_watch.reset(NULL);
}

void Gobby::BrowserContextCommands::on_account_created(
	gnutls_x509_privkey_t key,
	InfCertificateChain* certificate,
	const InfAclAccount* account)
{
	InfBrowser* browser;
	g_object_get(G_OBJECT(m_dialog->gobj()), "browser", &browser, NULL);
	const InfAclAccount* own_account =
		inf_browser_get_acl_local_account(browser);
	const InfAclAccountId default_id =
		inf_acl_account_id_from_string("default");
	const bool is_default_account = (own_account->id == default_id);
	g_object_unref(browser);

	gnutls_x509_crt_t cert =
		inf_certificate_chain_get_own_certificate(certificate);
	gchar* name = inf_cert_util_get_dn_by_oid(
		cert, GNUTLS_OID_X520_COMMON_NAME, 0);
	const std::string cn = name;
	g_free(name);

	std::string host;
	if(INFC_IS_BROWSER(browser))
	{
		InfXmlConnection* xml =
			infc_browser_get_connection(INFC_BROWSER(browser));
		if(INF_IS_XMPP_CONNECTION(xml))
		{
			gchar* hostname;
			g_object_get(G_OBJECT(xml), "remote-hostname",
			             &hostname, NULL);
			host = hostname;
			g_free(hostname);
		}
	}

	if(host.empty())
		host = "local";

	gnutls_x509_crt_t* certs = inf_certificate_chain_get_raw(certificate);
	const unsigned int n_certs =
		inf_certificate_chain_get_n_certificates(certificate);

	std::auto_ptr<Gtk::MessageDialog> dlg;

	try
	{
		const std::string filename =
			make_certificate_filename(cn, host);

		GError* error = NULL;
		inf_cert_util_write_certificate_with_key(
			key, certs, n_certs, filename.c_str(), &error);

		if(error != NULL)
		{
			const std::string message = error->message;
			g_error_free(error);
			throw std::runtime_error(message);
		}

		if(is_default_account &&
		   (m_cert_manager.get_private_key() == NULL ||
		    m_cert_manager.get_certificates() == NULL))
		{
			m_preferences.security.key_file = filename;
			m_preferences.security.certificate_file = filename;

			dlg.reset(new Gtk::MessageDialog(
				m_parent, _("Account successfully created"),
				false, Gtk::MESSAGE_INFO,
				Gtk::BUTTONS_CLOSE));
			dlg->set_secondary_text(
				_("When re-connecting to the server, the "
				  "new account will be used."));
		}
		else
		{
			// TODO: Gobby should support multiple certificates
			dlg.reset(new Gtk::MessageDialog(
				m_parent, _("Account successfully created"),
				false, Gtk::MESSAGE_INFO,
				Gtk::BUTTONS_CLOSE));
			dlg->set_secondary_text(Glib::ustring::compose(
				_("The certificate has been stored at %1.\n\n"
				  "To login to this account, set the "
				  "certificate in Gobby's preferences "
				  "and re-connect to the server."),
				filename));
		}
	}
	catch(const std::exception& ex)
	{
		// This is actually a bit unfortunate, because the
		// account was actually created and we have a valid
		// certificate for it, but we cannot keep it...
		dlg.reset(new Gtk::MessageDialog(
			m_parent, _("Failed to create account"),
			false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE));
		dlg->set_secondary_text(Glib::ustring::compose(
			_("Could not save the certificate for the "
			  "account: %1"), ex.what()));
	}

	m_dialog = dlg;
	m_dialog->signal_response().connect(
		sigc::mem_fun(
			*this,
			&BrowserContextCommands::
				on_account_created_response));
	m_dialog->present();
}

void Gobby::BrowserContextCommands::on_account_created_response(
	int response_id)
{
	m_dialog.reset(NULL);
}

void Gobby::BrowserContextCommands::on_new_response(int response_id,
                                                    InfBrowser* browser,
						    InfBrowserIter iter,
						    bool directory)
{
	EntryDialog* entry_dialog = static_cast<EntryDialog*>(m_dialog.get());

	if(response_id == Gtk::RESPONSE_ACCEPT)
	{
		if(directory)
		{
			// TODO: Select the newly created directory in tree
			m_operations.create_directory(
				browser, &iter, entry_dialog->get_text());
		}
		else
		{
			m_operations.create_document(
				browser, &iter, entry_dialog->get_text());
		}
	}

	m_dialog.reset(NULL);
	m_dialog_watch.reset(NULL);
}

void Gobby::BrowserContextCommands::on_open_response(int response_id,
                                                     InfBrowser* browser,
                                                     InfBrowserIter iter)
{
	FileChooser::Dialog* dialog =
		static_cast<FileChooser::Dialog*>(m_dialog.get());
	if(response_id == Gtk::RESPONSE_ACCEPT)
	{
		const std::vector<Glib::RefPtr<Gio::File> > files =
			dialog->get_files();

		m_operations.create_documents(
			browser, &iter, m_preferences, files);
	}

	m_dialog.reset(NULL);
	m_dialog_watch.reset(NULL);
}

void Gobby::BrowserContextCommands::on_permissions_response(int response_id)
{
	m_dialog.reset(NULL);
	m_dialog_watch.reset(NULL);
}
