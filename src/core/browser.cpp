/* gobby - A GTKmm driven libobby client
 * Copyright (C) 2005 - 2008 0x539 dev group
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
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "core/browser.hpp"
#include "util/i18n.hpp"

#include <libinfinity/inf-config.h>
#include <libinfinity/common/inf-discovery-avahi.h>

#include <gtkmm/stock.h>
#include <gtkmm/image.h>

Gobby::Browser::Browser(Gtk::Window& parent,
                        const InfcNotePlugin* text_plugin,
                        StatusBar& status_bar,
                        Preferences& preferences):
	m_parent(parent),
	m_text_plugin(text_plugin),
	m_status_bar(status_bar),
	m_preferences(preferences),
	m_expander(_("_Direct connection"), true),
	m_hbox(false, 6),
	m_label_hostname(_("Host name:"))
{
	m_label_hostname.show();
	m_entry_hostname.signal_activate().connect(
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
	InfConnectionManager* connection_manager =
		inf_connection_manager_new();

	m_browser_store = inf_gtk_browser_store_new(INF_IO(m_io),
	                                            connection_manager,
	                                            NULL);
	g_object_unref(connection_manager);

	m_xmpp_manager = inf_xmpp_manager_new();
#ifdef INFINOTE_HAVE_AVAHI
	InfDiscoveryAvahi* discovery =
		inf_discovery_avahi_new(INF_IO(m_io), m_xmpp_manager,
		                        NULL, NULL);
	inf_gtk_browser_store_add_discovery(m_browser_store,
	                                    INF_DISCOVERY(discovery));
	g_object_unref(discovery);
#endif

	Glib::ustring known_hosts_file = Glib::build_filename(
		Glib::get_home_dir(), GOBBY_CONFIGDIR"/known_hosts");
	m_cert_manager = inf_gtk_certificate_manager_new(
		parent.gobj(), m_xmpp_manager,
		NULL, known_hosts_file.c_str());

	m_browser_view =
		INF_GTK_BROWSER_VIEW(
			inf_gtk_browser_view_new_with_model(
				INF_GTK_BROWSER_MODEL(m_browser_store)));

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

	set_spacing(6);
	pack_start(m_scroll, Gtk::PACK_EXPAND_WIDGET);
	pack_start(m_expander, Gtk::PACK_SHRINK);
}

Gobby::Browser::~Browser()
{
	for(ResolvMap::iterator iter = m_resolv_map.begin();
	    iter != m_resolv_map.end();
	    ++ iter)
	{
		cancel(iter->first);
	}

	g_object_unref(m_browser_store);
	g_object_unref(m_cert_manager);
	g_object_unref(m_xmpp_manager);
	g_object_unref(m_io);
}

void Gobby::Browser::on_expanded_changed()
{
	if(m_expander.get_expanded())
	{
		if(m_entry_hostname.get_flags() & Gtk::REALIZED)
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

	gtk_tree_model_get(GTK_TREE_MODEL(m_browser_store), iter,
	                   INF_GTK_BROWSER_MODEL_COL_BROWSER, &browser,
	                   INF_GTK_BROWSER_MODEL_COL_NODE, &browser_iter,
	                   -1);

	m_signal_activate.emit(browser, browser_iter);

	infc_browser_iter_free(browser_iter);
	g_object_unref(browser);
}

void Gobby::Browser::on_hostname_activate()
{
	Glib::ustring str = m_entry_hostname.get_text();
	if(str.empty()) return;

	Glib::ustring host;
	Glib::ustring service = "6523"; // Default

	Glib::ustring::size_type pos;
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
			sigc::mem_fun(*this, &Browser::on_resolv_done), str),
	        sigc::mem_fun(*this, &Browser::on_resolv_error));

	m_entry_hostname.set_text("");

	StatusBar::MessageHandle message_handle =
		m_status_bar.add_message(
			StatusBar::INFO, Glib::ustring::compose(
				_("Resolving %1…"), host), 0);

	m_resolv_map[resolv_handle].message_handle = message_handle;
}

void Gobby::Browser::on_resolv_done(ResolvHandle* handle,
                                    InfIpAddress* address, guint port,
                                    const Glib::ustring& hostname)
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
		InfTcpConnection* connection = INF_TCP_CONNECTION(
			g_object_new(INF_TYPE_TCP_CONNECTION,
			             "io", m_io,
			             "remote-address", address,
			             "remote-port", port,
			             NULL));

		GError* error = NULL;
		if(!inf_tcp_connection_open(connection, &error))
		{
			m_status_bar.add_message(StatusBar::ERROR,
			                         error->message, 5);
			g_error_free(error);
		}
		else
		{
			xmpp = inf_xmpp_connection_new(
				connection, INF_XMPP_CONNECTION_CLIENT,
				NULL, hostname.c_str(),
				m_preferences.security.policy, NULL, NULL);

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

	inf_gtk_browser_store_add_connection(
		m_browser_store, INF_XML_CONNECTION(xmpp),
		hostname.c_str());
}

void Gobby::Browser::on_resolv_error(ResolvHandle* handle,
                                     const std::runtime_error& error)
{
	ResolvMap::iterator iter = m_resolv_map.find(handle);
	g_assert(iter != m_resolv_map.end());

	m_status_bar.remove_message(iter->second.message_handle);
	m_resolv_map.erase(iter);

	m_status_bar.add_message(StatusBar::ERROR, error.what(), 5);
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
		GTK_TREE_MODEL(m_browser_store), &tree_iter,
		INF_GTK_BROWSER_MODEL_COL_BROWSER, &tmp_browser,
		-1);

	if(tmp_browser == NULL)
		return false;

	gtk_tree_model_get(
		GTK_TREE_MODEL(m_browser_store), &tree_iter,
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
		INF_GTK_BROWSER_MODEL(m_browser_store),
		browser, iter, &tree_iter);
	g_assert(has_iter == TRUE);

	inf_gtk_browser_view_set_selected(m_browser_view, &tree_iter);
}
