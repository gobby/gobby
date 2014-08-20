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

#ifndef _GOBBY_BROWSER_HPP_
#define _GOBBY_BROWSER_HPP_

#include "core/connectionmanager.hpp"
#include "core/statusbar.hpp"
#include "util/historyentry.hpp"

#include <libinfgtk/inf-gtk-browser-store.h>
#include <libinfgtk/inf-gtk-browser-view.h>
#include <libinfgtk/inf-gtk-certificate-manager.h>
#include <libinfgtk/inf-gtk-browser-model.h>
#include <libinfgtk/inf-gtk-browser-model-sort.h>
#include <libinfinity/common/inf-browser.h>

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
	        StatusBar& status_bar,
	        ConnectionManager& connection_manager);
	~Browser();

	ConnectionManager& get_connection_manager()
		{ return m_connection_manager; }

	InfGtkBrowserModelSort* get_store() { return m_sort_model; }
	const InfGtkBrowserModelSort* get_store() const {
		return m_sort_model;
	}

	InfGtkBrowserView* get_view() { return m_browser_view; }

	bool get_selected_browser(InfBrowser** browser);
	bool get_selected_iter(InfBrowser* browser, InfBrowserIter* iter);
	void set_selected(InfBrowser* browser, const InfBrowserIter* iter);

	InfBrowser* connect_to_host(const std::string& hostname,
	                            const std::string& service,
	                            unsigned int device_index);
	InfBrowser* connect_to_host(const InfIpAddress* address, guint port,
	                            unsigned int device_index,
	                            const std::string& hostname);
	void add_browser(InfBrowser* browser, const char* name);
	void remove_browser(InfBrowser* browser);

	SignalActivate signal_activate() const { return m_signal_activate; }
	SignalConnect signal_connect() const { return m_signal_connect; }

protected:
	void init_accessibility();

	static void on_set_browser_static(InfGtkBrowserModel* model,
	                                  GtkTreePath* path,
	                                  GtkTreeIter* iter,
	                                  InfBrowser* old_browser,
	                                  InfBrowser* new_browser,
	                                  gpointer user_data)
	{
		static_cast<Browser*>(user_data)->on_set_browser(
			iter, old_browser, new_browser);
	}

	static void on_activate_static(InfGtkBrowserView* view,
	                               GtkTreeIter* iter,
	                               gpointer user_data)
	{
		static_cast<Browser*>(user_data)->on_activate(iter);
	}

	void on_connection_replaced(InfXmppConnection* connection,
	                            InfXmppConnection* by);
	void on_expanded_changed();
	void on_set_browser(GtkTreeIter* iter, InfBrowser* old_browser,
	                    InfBrowser* new_browser);
	void on_activate(GtkTreeIter* iter);
	void on_hostname_activate();

	Gtk::Window& m_parent;
	StatusBar& m_status_bar;
	ConnectionManager& m_connection_manager;

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
