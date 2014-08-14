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
#include "util/file.hpp"
#include "util/i18n.hpp"

#include <gtkmm/icontheme.h>
#include <gtkmm/imagemenuitem.h>
#include <gtkmm/separatormenuitem.h>
#include <gtkmm/stock.h>
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
	m_popup_menu(NULL)
{
	g_object_ref(io);

	m_populate_popup_handler = g_signal_connect(
		m_browser.get_view(), "populate-popup",
		G_CALLBACK(on_populate_popup_static), this);
}

Gobby::BrowserContextCommands::~BrowserContextCommands()
{
	g_signal_handler_disconnect(m_browser.get_view(),
	                            m_populate_popup_handler);

	g_object_unref(m_io);
}

void Gobby::BrowserContextCommands::on_menu_node_removed()
{
	g_assert(m_popup_menu != NULL);

	// This calls deactivate, causing the watch to be removed.
	m_popup_menu->popdown();
}

void Gobby::BrowserContextCommands::on_menu_deactivate()
{
	m_watch.reset(NULL);
}

void Gobby::BrowserContextCommands::on_populate_popup(Gtk::Menu* menu)
{
	// TODO: Can this happen? Should we close the old popup here?
	g_assert(m_popup_menu == NULL);

	// Cancel previous attempts
	m_watch.reset(NULL);
	m_dialog.reset(NULL);

	InfBrowser* browser;
	InfBrowserIter iter;

	if(!m_browser.get_selected_browser(&browser))
		return;

	if(!m_browser.get_selected_iter(browser, &iter))
	{
		InfBrowserStatus browser_status;
		g_object_get(G_OBJECT(browser), "status",
		             &browser_status, NULL);

		if(browser_status == INF_BROWSER_CLOSED)
		{
			Gtk::ImageMenuItem *remove_item = Gtk::manage(
				new Gtk::ImageMenuItem(_("_Remove"), true));
			remove_item->set_image(*Gtk::manage(new Gtk::Image(
				Gtk::Stock::CLOSE, Gtk::ICON_SIZE_MENU)));
			remove_item->signal_activate().connect(sigc::bind(
				sigc::mem_fun(*this,
					&BrowserContextCommands::on_remove),
				browser));
			remove_item->show();
			menu->append(*remove_item);
		}

		return;
	}

	InfBrowserIter dummy_iter = iter;
	bool is_subdirectory = inf_browser_is_subdirectory(browser, &iter);
	bool is_toplevel = !inf_browser_get_parent(browser, &dummy_iter);

	// Watch the node, and close the popup menu when the node
	// it refers to is removed.
	m_watch.reset(new NodeWatch(browser, &iter));
	m_watch->signal_node_removed().connect(sigc::mem_fun(
		*this, &BrowserContextCommands::on_menu_node_removed));

	menu->signal_deactivate().connect(sigc::mem_fun(
		*this, &BrowserContextCommands::on_menu_deactivate));

	bool have_toplevel_entries = false;

	// Add "Disconnect" menu option if the connection
	// item has been clicked at
	if(is_toplevel && INFC_IS_BROWSER(browser))
	{
		Gtk::ImageMenuItem* disconnect_item = Gtk::manage(
			new Gtk::ImageMenuItem(
				_("_Disconnect from Server"), true));
		disconnect_item->set_image(*Gtk::manage(new Gtk::Image(
			Gtk::Stock::DISCONNECT, Gtk::ICON_SIZE_MENU)));
		disconnect_item->signal_activate().connect(sigc::bind(
			sigc::mem_fun(*this,
				&BrowserContextCommands::on_disconnect),
			INFC_BROWSER(browser)));
		disconnect_item->show();
		menu->append(*disconnect_item);
		have_toplevel_entries = true;
	}

	// Add create account option if the connection item has been clicked at
	if(is_toplevel)
	{
		const InfAclAccount* account =
			inf_browser_get_acl_local_account(browser);
		const InfAclAccountId acc_id =
			(account != NULL) ? account->id : 0;

		InfAclMask mask;
		inf_acl_mask_set1(&mask, INF_ACL_CAN_CREATE_ACCOUNT);
		inf_browser_check_acl(browser, &iter, acc_id, &mask, &mask);
		if(inf_acl_mask_has(&mask, INF_ACL_CAN_CREATE_ACCOUNT))
		{
			Gtk::Image* image = Gtk::manage(new Gtk::Image);
			image->set_from_icon_name("application-certificate",
			                          Gtk::ICON_SIZE_MENU);
			Gtk::ImageMenuItem* item =
				Gtk::manage(new Gtk::ImageMenuItem(
	                                _("Create _Account..."), true));
			item->set_image(*image);
			item->signal_activate().connect(sigc::bind(
				sigc::mem_fun(
					*this,
					&BrowserContextCommands::
						on_create_account),
				browser));
			item->show();
			menu->append(*item);
			have_toplevel_entries = true;
		}
	}

	// Separator, if we have entries dedicated to the browser itself
	if(have_toplevel_entries)
	{
		Gtk::SeparatorMenuItem* sep_item =
			Gtk::manage(new Gtk::SeparatorMenuItem);
		sep_item->show();
		menu->append(*sep_item);
	}

	// Create Document
	Gtk::ImageMenuItem* new_document_item = Gtk::manage(
		new Gtk::ImageMenuItem(_("Create Do_cument..."),
		                       true));
	new_document_item->set_image(*Gtk::manage(new Gtk::Image(
		Gtk::Stock::NEW, Gtk::ICON_SIZE_MENU)));
	new_document_item->signal_activate().connect(sigc::bind(
		sigc::mem_fun(*this,
			&BrowserContextCommands::on_new),
		browser, iter, false));
	new_document_item->set_sensitive(is_subdirectory);
	new_document_item->show();
	menu->append(*new_document_item);

	// Create Directory

	// Check whether we have the folder-new icon, fall back to
	// Stock::DIRECTORY otherwise
	Glib::RefPtr<Gdk::Screen> screen = menu->get_screen();
	Glib::RefPtr<Gtk::IconTheme> icon_theme(
		Gtk::IconTheme::get_for_screen(screen));

	Gtk::Image* new_directory_image = Gtk::manage(new Gtk::Image);
	if(icon_theme->lookup_icon("folder-new",
	                           Gtk::ICON_SIZE_MENU,
	                           Gtk::ICON_LOOKUP_USE_BUILTIN))
	{
		new_directory_image->set_from_icon_name(
			"folder-new", Gtk::ICON_SIZE_MENU);
	}
	else
	{
		new_directory_image->set(
			Gtk::Stock::DIRECTORY, Gtk::ICON_SIZE_MENU);
	}

	Gtk::ImageMenuItem* new_directory_item = Gtk::manage(
		new Gtk::ImageMenuItem(_("Create Di_rectory..."),
		                       true));
	new_directory_item->set_image(*new_directory_image);
	new_directory_item->signal_activate().connect(sigc::bind(
		sigc::mem_fun(*this,
			&BrowserContextCommands::on_new),
		browser, iter, true));
	new_directory_item->set_sensitive(is_subdirectory);
	new_directory_item->show();
	menu->append(*new_directory_item);

	// Open Document
	Gtk::ImageMenuItem* open_document_item = Gtk::manage(
		new Gtk::ImageMenuItem(_("_Open Document..."), true));
	open_document_item->set_image(*Gtk::manage(new Gtk::Image(
		Gtk::Stock::OPEN, Gtk::ICON_SIZE_MENU)));
	open_document_item->signal_activate().connect(sigc::bind(
		sigc::mem_fun(*this,
			&BrowserContextCommands::on_open),
		browser, iter));
	open_document_item->set_sensitive(is_subdirectory);
	open_document_item->show();
	menu->append(*open_document_item);

	// Separator
	Gtk::SeparatorMenuItem* sep_item =
		Gtk::manage(new Gtk::SeparatorMenuItem);
	sep_item->show();
	menu->append(*sep_item);

	// Permissions
	Gtk::ImageMenuItem* permissions_item = Gtk::manage(
		new Gtk::ImageMenuItem(_("_Permissions..."), true));
	permissions_item->set_image(*Gtk::manage(new Gtk::Image(
		Gtk::Stock::PROPERTIES, Gtk::ICON_SIZE_MENU)));
	permissions_item->signal_activate().connect(sigc::bind(
		sigc::mem_fun(*this,
			&BrowserContextCommands::on_permissions),
		browser, iter));
	permissions_item->show();
	menu->append(*permissions_item);

	// Delete
	Gtk::ImageMenuItem* delete_item = Gtk::manage(
		new Gtk::ImageMenuItem(_("D_elete"), true));
	delete_item->set_image(*Gtk::manage(new Gtk::Image(
		Gtk::Stock::DELETE, Gtk::ICON_SIZE_MENU)));
	delete_item->signal_activate().connect(sigc::bind(
		sigc::mem_fun(*this,
			&BrowserContextCommands::on_delete),
		browser, iter));
	delete_item->set_sensitive(!is_toplevel);
	delete_item->show();
	menu->append(*delete_item);
	
	m_popup_menu = menu;
	menu->signal_selection_done().connect(
		sigc::mem_fun(
			*this,
			&BrowserContextCommands::on_popdown));
}

void Gobby::BrowserContextCommands::on_popdown()
{
	m_popup_menu = NULL;
}

void Gobby::BrowserContextCommands::on_disconnect(InfcBrowser* browser)
{
	InfXmlConnection* connection = infc_browser_get_connection(browser);
	InfXmlConnectionStatus status;
	g_object_get(G_OBJECT(connection), "status", &status, NULL);

	if(status != INF_XML_CONNECTION_CLOSED &&
	   status != INF_XML_CONNECTION_CLOSING)
	{
		inf_xml_connection_close(connection);
	}
}

void Gobby::BrowserContextCommands::on_create_account(InfBrowser* browser)
{
	InfGtkAccountCreationDialog* dlg =
		inf_gtk_account_creation_dialog_new(
			m_parent.gobj(), static_cast<GtkDialogFlags>(0),
			m_io, browser);
	std::auto_ptr<Gtk::Dialog> account_creation_dialog(
		Glib::wrap(GTK_DIALOG(dlg)));

	account_creation_dialog->add_button(
		Gtk::Stock::CLOSE, Gtk::RESPONSE_CLOSE);

	account_creation_dialog->signal_response().connect(
		sigc::mem_fun(*this,
			&BrowserContextCommands::on_create_account_response));

	g_signal_connect(
		G_OBJECT(account_creation_dialog->gobj()), "account-created",
		G_CALLBACK(on_account_created_static), this);

	m_dialog = account_creation_dialog;
	m_dialog->present();
}

void Gobby::BrowserContextCommands::on_remove(InfBrowser* browser)
{
	InfBrowserStatus status;
	g_object_get(G_OBJECT(browser), "status", &status, NULL);

	g_assert(status == INF_BROWSER_CLOSED);

	m_browser.remove_browser(browser);
}

void Gobby::BrowserContextCommands::on_new(InfBrowser* browser,
                                           InfBrowserIter iter,
                                           bool directory)
{
	m_watch.reset(new NodeWatch(browser, &iter));
	m_watch->signal_node_removed().connect(sigc::mem_fun(
		*this, &BrowserContextCommands::on_dialog_node_removed));

	std::auto_ptr<EntryDialog> entry_dialog(
		new EntryDialog(
			m_parent,
			directory ? _("Choose a name for the directory")
			          : _("Choose a name for the document"),
			directory ? _("_Directory Name:")
			          : _("_Document Name:")));

	entry_dialog->add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	entry_dialog->add_button(_("C_reate"), Gtk::RESPONSE_ACCEPT)
		->set_image(*Gtk::manage(new Gtk::Image(
			Gtk::Stock::NEW, Gtk::ICON_SIZE_BUTTON)));

	entry_dialog->set_text(directory ? _("New Directory")
	                                   : _("New Document"));
	entry_dialog->signal_response().connect(sigc::bind(
		sigc::mem_fun(*this,
			&BrowserContextCommands::on_new_response),
		browser, iter, directory));

	m_dialog = entry_dialog;
	m_dialog->present();
}

void Gobby::BrowserContextCommands::on_open(InfBrowser* browser,
                                            InfBrowserIter iter)
{
	m_watch.reset(new NodeWatch(browser, &iter));
	m_watch->signal_node_removed().connect(sigc::mem_fun(
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

void Gobby::BrowserContextCommands::on_permissions(InfBrowser* browser,
                                                   InfBrowserIter iter)
{
	m_watch.reset(new NodeWatch(browser, &iter));
	m_watch->signal_node_removed().connect(sigc::mem_fun(
		*this, &BrowserContextCommands::on_dialog_node_removed));

	InfGtkPermissionsDialog* dlg = inf_gtk_permissions_dialog_new(
		m_parent.gobj(), static_cast<GtkDialogFlags>(0),
		browser, &iter);
	std::auto_ptr<Gtk::Dialog> permissions_dialog(
		Glib::wrap(GTK_DIALOG(dlg)));

	permissions_dialog->add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CLOSE);

	permissions_dialog->signal_response().connect(
		sigc::mem_fun(*this,
			&BrowserContextCommands::on_permissions_response));

	m_dialog = permissions_dialog;
	m_dialog->present();
}

void Gobby::BrowserContextCommands::on_delete(InfBrowser* browser,
                                              InfBrowserIter iter)
{
	m_operations.delete_node(browser, &iter);
}

void Gobby::BrowserContextCommands::on_dialog_node_removed()
{
	m_watch.reset(NULL);
	m_dialog.reset(NULL);
}

void Gobby::BrowserContextCommands::on_create_account_response(int response_id)
{
	m_watch.reset(NULL);
	m_dialog.reset(NULL);
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
		const std::string filename = make_certificate_filename(cn, host);

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

	m_watch.reset(NULL);
	m_dialog.reset(NULL);
}

void Gobby::BrowserContextCommands::on_open_response(int response_id,
                                                     InfBrowser* browser,
                                                     InfBrowserIter iter)
{
	FileChooser::Dialog* dialog =
		static_cast<FileChooser::Dialog*>(m_dialog.get());
	if(response_id == Gtk::RESPONSE_ACCEPT)
	{
		Glib::SListHandle<Glib::ustring> uris = dialog->get_uris();

		OperationOpenMultiple::uri_list uri_list(
			uris.begin(), uris.end());

		m_operations.create_documents(
			browser, &iter, m_preferences, uri_list);
	}

	m_watch.reset(NULL);
	m_dialog.reset(NULL);
}

void Gobby::BrowserContextCommands::on_permissions_response(int response_id)
{
	m_watch.reset(NULL);
	m_dialog.reset(NULL);
}
