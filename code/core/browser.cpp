/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2011 Armin Burgmeier <armin@arbur.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include "dialogs/password-dialog.hpp"
#include "core/browser.hpp"
#include "util/gtk-compat.hpp"
#include "util/file.hpp"
#include "util/i18n.hpp"

#include <libinfinity/inf-config.h>

#include <atkmm/relationset.h>

#include <gtkmm/stock.h>
#include <gtkmm/image.h>

#ifndef G_OS_WIN32
# include <sys/socket.h>
# include <net/if.h>
#endif

gint compare_func(GtkTreeModel* model, GtkTreeIter* first, GtkTreeIter* second, gpointer user_data)
{
	gint result;
	InfcBrowser* br_one;
	InfcBrowser* br_two;
	InfcBrowserIter* bri_one;
	InfcBrowserIter* bri_two;
	GtkTreeIter parent;

	result = 0;
	if(gtk_tree_model_iter_parent(model, &parent, first))
	{
		g_assert(gtk_tree_model_iter_parent(model, &parent, second));

		gtk_tree_model_get(
			model, first,
			INF_GTK_BROWSER_MODEL_COL_BROWSER, &br_one,
			INF_GTK_BROWSER_MODEL_COL_NODE, &bri_one,
			-1);
		gtk_tree_model_get(
			model, second,
			INF_GTK_BROWSER_MODEL_COL_BROWSER, &br_two,
			INF_GTK_BROWSER_MODEL_COL_NODE, &bri_two,
			-1);

		if(infc_browser_iter_is_subdirectory(br_one, bri_one) &&
		   !infc_browser_iter_is_subdirectory(br_two, bri_two))
		{
			result = -1;
		}
		else if(!infc_browser_iter_is_subdirectory(br_one, bri_one) &&
		        infc_browser_iter_is_subdirectory(br_two, bri_two))
		{
			result = 1;
		}

		g_object_unref(br_one);
		g_object_unref(br_two);
		infc_browser_iter_free(bri_one);
		infc_browser_iter_free(bri_two);
	}

	if(!result)
	{
		gchar* name_one;
		gchar* name_two;

		gtk_tree_model_get(
			model, first,
			INF_GTK_BROWSER_MODEL_COL_NAME, &name_one,
			-1);
		gtk_tree_model_get(
			model, second,
			INF_GTK_BROWSER_MODEL_COL_NAME, &name_two,
			-1);

		gchar* one = g_utf8_casefold(name_one, -1);
		gchar* two = g_utf8_casefold(name_two, -1);

		result = g_utf8_collate(one, two);

		g_free(name_one);
		g_free(name_two);
		g_free(one);
		g_free(two);
	}

	return result;
}

Gobby::Browser::Browser(Gtk::Window& parent,
                        const InfcNotePlugin* text_plugin,
                        StatusBar& status_bar,
                        Preferences& preferences):
	m_parent(parent),
	m_text_plugin(text_plugin),
	m_status_bar(status_bar),
	m_preferences(preferences),
	m_sasl_context(NULL),
	m_expander(_("_Direct Connection"), true),
	m_hbox(false, 6),
	m_label_hostname(_("Host Name:")),
	m_entry_hostname(config_filename("recent_hosts"), 5)
{
	m_label_hostname.show();
	m_entry_hostname.get_entry()->signal_activate().connect(
		sigc::mem_fun(*this, &Browser::on_hostname_activate));
	m_entry_hostname.show();

	m_hbox.pack_start(m_label_hostname, Gtk::PACK_SHRINK);
	m_hbox.pack_start(m_entry_hostname, Gtk::PACK_EXPAND_WIDGET);
	m_hbox.show();

	m_expander.add(m_hbox);
	m_expander.show();
	m_expander.property_expanded().signal_changed().connect(
		sigc::mem_fun(*this, &Browser::on_expanded_changed));
	
	m_io = inf_gtk_io_new();
	InfCommunicationManager* communication_manager =
		inf_communication_manager_new();

	m_browser_store = inf_gtk_browser_store_new(INF_IO(m_io),
	                                            communication_manager);
	g_object_unref(communication_manager);
	
	m_sort_model = inf_gtk_browser_model_sort_new(INF_GTK_BROWSER_MODEL(m_browser_store));
	gtk_tree_sortable_set_default_sort_func(GTK_TREE_SORTABLE(m_sort_model), compare_func, NULL, NULL);

	m_xmpp_manager = inf_xmpp_manager_new();
#ifdef LIBINFINITY_HAVE_AVAHI
	m_discovery = inf_discovery_avahi_new(INF_IO(m_io), m_xmpp_manager,
	                                      NULL, NULL, NULL);
	inf_discovery_avahi_set_security_policy(
		m_discovery, m_preferences.security.policy);
	inf_gtk_browser_store_add_discovery(m_browser_store,
	                                    INF_DISCOVERY(m_discovery));
#endif

	Glib::ustring known_hosts_file = config_filename("known_hosts");

	const std::string trust_file = m_preferences.security.trust_file;
	m_cert_manager = inf_gtk_certificate_manager_new(
		parent.gobj(), m_xmpp_manager,
		trust_file.empty() ? NULL : trust_file.c_str(),
		known_hosts_file.c_str());

	m_browser_view =
		INF_GTK_BROWSER_VIEW(
			inf_gtk_browser_view_new_with_model(
				INF_GTK_BROWSER_MODEL(m_sort_model)));

	gtk_widget_show(GTK_WIDGET(m_browser_view));
	gtk_container_add(GTK_CONTAINER(m_scroll.gobj()),
	                  GTK_WIDGET(m_browser_view));
	m_scroll.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	m_scroll.set_shadow_type(Gtk::SHADOW_IN);
	m_scroll.show();

	g_signal_connect(
		m_browser_store,
		"set-browser",
		G_CALLBACK(&on_set_browser_static),
		this
	);

	g_signal_connect(
		m_browser_view,
		"activate",
		G_CALLBACK(&on_activate_static),
		this
	);

	m_preferences.security.policy.signal_changed().connect(
		sigc::mem_fun(*this, &Browser::on_security_policy_changed));
	m_preferences.security.trust_file.signal_changed().connect(
		sigc::mem_fun(*this, &Browser::on_trust_file_changed));

	set_spacing(6);
	pack_start(m_scroll, Gtk::PACK_EXPAND_WIDGET);
	pack_start(m_expander, Gtk::PACK_SHRINK);

	init_accessibility();

	set_focus_child(m_expander);
}

Gobby::Browser::~Browser()
{
	for(ResolvMap::iterator iter = m_resolv_map.begin();
	    iter != m_resolv_map.end();
	    ++ iter)
	{
		cancel(iter->first);
	}

	if(m_sasl_context)
		inf_sasl_context_unref(m_sasl_context);

	g_object_unref(m_browser_store);
	g_object_unref(m_sort_model);
	g_object_unref(m_cert_manager);
	g_object_unref(m_xmpp_manager);
#ifdef LIBINFINITY_HAVE_AVAHI
	g_object_unref(m_discovery);
#endif
	g_object_unref(m_io);
}

void Gobby::Browser::init_accessibility()
{
	Glib::RefPtr<Atk::RelationSet> relation_set;
	Glib::RefPtr<Atk::Relation> relation;
	std::vector<Glib::RefPtr<Atk::Object> > targets;

	// Associate the hostname label with the corresponding entry field.
	Glib::RefPtr<Atk::Object> entry_hostname_acc = m_entry_hostname.get_accessible();
	Glib::RefPtr<Atk::Object> label_hostname_acc = m_label_hostname.get_accessible();

	relation_set = entry_hostname_acc->get_relation_set();
	targets.push_back(label_hostname_acc);

	relation = Atk::Relation::create(targets, Atk::RELATION_LABELLED_BY);
	relation_set->set_add(relation);
}

void Gobby::Browser::on_expanded_changed()
{
	if(m_expander.get_expanded())
	{
		if(GtkCompat::is_realized(m_entry_hostname))
		{
			m_entry_hostname.grab_focus();
		}
		else
		{
			m_entry_hostname.signal_realize().connect(
				sigc::mem_fun(m_entry_hostname,
					&Gtk::Entry::grab_focus));
		}
	}
}

void Gobby::Browser::on_set_browser(GtkTreeIter* iter,
                                    InfcBrowser* browser)
{
	if(browser)
		infc_browser_add_plugin(browser, m_text_plugin);
}

void Gobby::Browser::on_activate(GtkTreeIter* iter)
{
	InfcBrowser* browser;
	InfcBrowserIter* browser_iter;

	gtk_tree_model_get(GTK_TREE_MODEL(m_sort_model), iter,
	                   INF_GTK_BROWSER_MODEL_COL_BROWSER, &browser,
	                   INF_GTK_BROWSER_MODEL_COL_NODE, &browser_iter,
	                   -1);

	m_signal_activate.emit(browser, browser_iter);

	infc_browser_iter_free(browser_iter);
	g_object_unref(browser);
}

void Gobby::Browser::on_hostname_activate()
{
	Glib::ustring str = m_entry_hostname.get_entry()->get_text();
	if(str.empty()) return;

	connect_to_host(str);

	m_entry_hostname.commit();
	m_entry_hostname.get_entry()->set_text("");
}

void Gobby::Browser::on_resolv_done(ResolvHandle* handle,
                                    InfIpAddress* address, guint port,
                                    const Glib::ustring& hostname,
                                    unsigned int device_index)
{
	ResolvMap::iterator iter = m_resolv_map.find(handle);
	g_assert(iter != m_resolv_map.end());

	m_status_bar.remove_message(iter->second.message_handle);
	m_resolv_map.erase(iter);

	InfXmppConnection* xmpp =
		inf_xmpp_manager_lookup_connection_by_address(m_xmpp_manager,
		                                              address,
		                                              port);

	if(!xmpp)
	{
		InfTcpConnection* connection = inf_tcp_connection_new(
			INF_IO(m_io),
			address,
			port);

		g_object_set(G_OBJECT(connection),
			"device-index", device_index,
			NULL);

		GError* error = NULL;
		if(!inf_tcp_connection_open(connection, &error))
		{
			m_status_bar.add_error_message(
				Glib::ustring::compose(
					_("Connection to \"%1\" failed"),
					hostname), error->message);
			g_error_free(error);
		}
		else
		{
			xmpp = inf_xmpp_connection_new(
				connection, INF_XMPP_CONNECTION_CLIENT,
				NULL, hostname.c_str(),
				m_preferences.security.policy,
				NULL,
				m_sasl_context,
				m_sasl_mechanisms.empty()
					? ""
					: m_sasl_mechanisms.c_str());

			inf_xmpp_manager_add_connection(m_xmpp_manager, xmpp);
			g_object_unref(xmpp);
		}

		g_object_unref(connection);
	}

	// TODO: Check whether there is already an item with this
	// connection in the browser. If yes, don't add, but highlight for
	// feedback.

	// TODO: Remove erroneous entry with same name, if any, before
	// adding.

	if(xmpp != NULL)
	{
		inf_gtk_browser_store_add_connection(
			m_browser_store, INF_XML_CONNECTION(xmpp),
			hostname.c_str());

		/* TODO: Initial root node expansion for the newly added node.
		 * This probably requires additional API in
		 * InfGtkBrowserView */
	}
}

void Gobby::Browser::on_resolv_error(ResolvHandle* handle,
                                     const std::runtime_error& error,
                                     const Glib::ustring& hostname)
{
	ResolvMap::iterator iter = m_resolv_map.find(handle);
	g_assert(iter != m_resolv_map.end());

	m_status_bar.remove_message(iter->second.message_handle);
	m_resolv_map.erase(iter);

	m_status_bar.add_error_message(
		Glib::ustring::compose(_("Could not resolve \"%1\""),
		                       hostname),
		error.what());
}

bool Gobby::Browser::get_selected(InfcBrowser** browser,
                                  InfcBrowserIter* iter)
{
	GtkTreeIter tree_iter;
	if(!inf_gtk_browser_view_get_selected(m_browser_view, &tree_iter))
		return false;

	InfcBrowser* tmp_browser;
	InfcBrowserIter* tmp_iter;

	gtk_tree_model_get(
		GTK_TREE_MODEL(m_sort_model), &tree_iter,
		INF_GTK_BROWSER_MODEL_COL_BROWSER, &tmp_browser,
		-1);

	if(tmp_browser == NULL)
		return false;

	gtk_tree_model_get(
		GTK_TREE_MODEL(m_sort_model), &tree_iter,
		INF_GTK_BROWSER_MODEL_COL_NODE, &tmp_iter,
		-1);

	*browser = tmp_browser;
	*iter = *tmp_iter;

	infc_browser_iter_free(tmp_iter);
	g_object_unref(tmp_browser);

	return true;
}

void Gobby::Browser::set_selected(InfcBrowser* browser, InfcBrowserIter* iter)
{
	GtkTreeIter tree_iter;

	gboolean has_iter = inf_gtk_browser_model_browser_iter_to_tree_iter(
		INF_GTK_BROWSER_MODEL(m_sort_model),
		browser, iter, &tree_iter);
	g_assert(has_iter == TRUE);

	inf_gtk_browser_view_set_selected(m_browser_view, &tree_iter);
}

void Gobby::Browser::connect_to_host(Glib::ustring str)
{
	Glib::ustring host;
	Glib::ustring service = "6523"; // Default
	unsigned int device_index = 0; // Default

	// Strip away device name
	Glib::ustring::size_type pos;
	if( (pos = str.rfind('%')) != Glib::ustring::npos)
	{
		Glib::ustring device_name = str.substr(pos + 1);
		str.erase(pos);

#ifdef G_OS_WIN32
		// TODO: Implement
		device_index = 0;
#else
		device_index = if_nametoindex(device_name.c_str());
		if(device_index == 0)
		{
			m_status_bar.add_error_message(
				Glib::ustring::compose(
					_("Connection to \"%1\" failed"),
					host),
				Glib::ustring::compose(
					_("Device \"%1\" does not exist"),
					device_name));
		}
#endif
	}

	if(str[0] == '[' && ((pos = str.find(']', 1)) != Glib::ustring::npos))
	{
		// Hostname surrounded by '[...]'
		host = str.substr(1, pos-1);
		++ pos;
		if(pos < str.length() && str[pos] == ':')
			service = str.substr(pos + 1);
	}
	else
	{
		pos = str.find(':');
		if(pos != Glib::ustring::npos)
		{
			host = str.substr(0, pos);
			service = str.substr(pos + 1);
		}
		else
			host = str;
	}

	ResolvHandle* resolv_handle = resolve(host, service,
	        sigc::bind(
			sigc::mem_fun(*this, &Browser::on_resolv_done),
			host, device_index),
	        sigc::bind(
			sigc::mem_fun(*this, &Browser::on_resolv_error),
			host));

	StatusBar::MessageHandle message_handle =
		m_status_bar.add_info_message(
			Glib::ustring::compose(_("Resolving \"%1\"..."),
			                       host));

	m_resolv_map[resolv_handle].message_handle = message_handle;
}

void Gobby::Browser::set_sasl_context(InfSaslContext* sasl_context,
                                      const char* mechanisms)
{
	if(m_sasl_context) inf_sasl_context_unref(m_sasl_context);
	m_sasl_context = sasl_context;
	if(m_sasl_context) inf_sasl_context_ref(m_sasl_context);
	m_sasl_mechanisms = mechanisms ? mechanisms : "";

#ifdef LIBINFINITY_HAVE_AVAHI
	g_object_set(G_OBJECT(m_discovery),
		"sasl-context", m_sasl_context,
		"sasl-mechanisms", mechanisms,
		NULL);
#endif
}

void Gobby::Browser::on_security_policy_changed()
{
#ifdef LIBINFINITY_HAVE_AVAHI
	inf_discovery_avahi_set_security_policy(
		m_discovery, m_preferences.security.policy);
#endif
}

void Gobby::Browser::on_trust_file_changed()
{
	const std::string trust_file = m_preferences.security.trust_file;

	g_object_set(G_OBJECT(m_cert_manager), "trust-file",
		     trust_file.empty() ? NULL : trust_file.c_str(), NULL);
}
