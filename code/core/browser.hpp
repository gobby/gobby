/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2013 Armin Burgmeier <armin@arbur.net>
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

#ifndef _GOBBY_BROWSER_HPP_
#define _GOBBY_BROWSER_HPP_

#include "core/statusbar.hpp"
#include "core/preferences.hpp"
#include "core/certificatemanager.hpp"
#include "util/resolv.hpp"
#include "util/historyentry.hpp"

#include <libinfgtk/inf-gtk-io.h>
#include <libinfgtk/inf-gtk-browser-store.h>
#include <libinfgtk/inf-gtk-browser-view.h>
#include <libinfgtk/inf-gtk-certificate-manager.h>
#include <libinfinity/client/infc-browser.h>
#include <libinfinity/common/inf-discovery-avahi.h>
#include <libinfinity/common/inf-xmpp-manager.h>
#include <libinfinity/inf-config.h>
#include <libinfgtk/inf-gtk-browser-model.h>
#include <libinfgtk/inf-gtk-browser-model-sort.h>

#include <gtkmm/window.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/expander.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>
#include <gtkmm/treeiter.h>

namespace Gobby
{

class Browser: public Gtk::VBox
{
public:
	typedef sigc::signal<void, Glib::ustring>
		SignalConnect;
	typedef sigc::signal<void, InfBrowser*, InfBrowserIter*>
		SignalActivate;

	Browser(Gtk::Window& parent,
	        const InfcNotePlugin* text_plugin,
	        StatusBar& status_bar,
	        const CertificateManager& cert_manager,
	        const Preferences& preferences);
	~Browser();

	InfGtkBrowserModelSort* get_store() { return m_sort_model; }
	const InfGtkBrowserModelSort* get_store() const {
		return m_sort_model;
	}

	InfGtkBrowserView* get_view() { return m_browser_view; }

	bool get_selected(InfBrowser** browser, InfBrowserIter* iter);
	void set_selected(InfBrowser* browser, const InfBrowserIter* iter);

	InfBrowser* connect_to_host(const InfIpAddress* address, guint port,
	                            unsigned int device_index,
	                            const std::string& hostname);

	void set_sasl_context(InfSaslContext* sasl_context,
	                      const char* mechanisms);

	SignalActivate signal_activate() const { return m_signal_activate; }
	SignalConnect signal_connect() const { return m_signal_connect; }

protected:
	void init_accessibility();

	static void on_set_browser_static(InfGtkBrowserModel* model,
	                                  GtkTreePath* path,
	                                  GtkTreeIter* iter,
	                                  InfcBrowser* browser,
	                                  gpointer user_data)
	{
		static_cast<Browser*>(user_data)->on_set_browser(
			iter, browser);
	}

	static void on_activate_static(InfGtkBrowserView* view,
	                               GtkTreeIter* iter,
	                               gpointer user_data)
	{
		static_cast<Browser*>(user_data)->on_activate(iter);
	}

	void on_expanded_changed();
	void on_set_browser(GtkTreeIter* iter, InfcBrowser* browser);
	void on_activate(GtkTreeIter* iter);
	void on_hostname_activate();

	void on_security_policy_changed();
	void on_credentials_changed();

	Gtk::Window& m_parent;
	const InfcNotePlugin* m_text_plugin;
	StatusBar& m_status_bar;
	const CertificateManager& m_cert_manager;
	const Preferences& m_preferences;

	InfSaslContext* m_sasl_context;
	std::string m_sasl_mechanisms;
	InfXmppManager* m_xmpp_manager;
#ifdef LIBINFINITY_HAVE_AVAHI
	InfDiscoveryAvahi* m_discovery;
#endif
	InfGtkIo* m_io;
	InfGtkCertificateManager* m_cert_checker;
	InfGtkBrowserStore* m_browser_store;
	InfGtkBrowserView* m_browser_view;
	Gtk::ScrolledWindow m_scroll;

	Gtk::Expander m_expander;
	Gtk::HBox m_hbox;
	Gtk::Label m_label_hostname;
	HistoryComboBoxEntry m_entry_hostname;

	InfGtkBrowserModelSort* m_sort_model;

	SignalConnect m_signal_connect;
	SignalActivate m_signal_activate;
};

}
	
#endif // _GOBBY_BROWSER_HPP_
