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

#ifndef _GOBBY_BROWSER_HPP_
#define _GOBBY_BROWSER_HPP_

#include <libinfgtk/inf-gtk-io.h>
#include <libinfgtk/inf-gtk-browser-model.h>
#include <libinfgtk/inf-gtk-browser-view.h>
#include <libinfinity/client/infc-browser.h>
#include <libinfinity/common/inf-xmpp-manager.h>

#include <gtkmm/window.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/expander.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>
#include <gtkmm/treeiter.h>

#include "resolv.hpp"
#include "preferences.hpp"
#include "statusbar.hpp"

namespace Gobby
{

class Browser: public Gtk::VBox
{
public:
	struct Resolv
	{
		StatusBar::MessageHandle message_handle;
	};

	typedef std::map<ResolvHandle*, Resolv> ResolvMap;

	typedef sigc::signal<void, InfcBrowser*, InfcBrowserIter*>
		SignalActivate;

	Browser(Gtk::Window& parent,
	        const InfcNotePlugin* text_plugin,
	        StatusBar& status_bar,
	        Preferences& preferences);
	~Browser();

	SignalActivate signal_activate() const { return m_signal_activate; }

protected:
	static void on_set_browser_static(InfGtkBrowserModel* model,
	                                  GtkTreePath* path,
	                                  GtkTreeIter* iter,
	                                  InfcBrowser* browser,
	                                  gpointer user_data)
	{
		Browser* browserpp = static_cast<Browser*>(user_data);

		browserpp->on_set_browser(iter, browser);
			//Gtk::TreeIter(
			//	GTK_TREE_MODEL(browserpp->m_browser_model),
			//	iter), browser);
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

	void on_resolv_done(ResolvHandle* handle, InfIpAddress* address,
	                    guint port, const Glib::ustring& hostname);
	void on_resolv_error(ResolvHandle* handle,
	                     const std::runtime_error& error);

	Gtk::Window& m_parent;
	const InfcNotePlugin* m_text_plugin;
	StatusBar& m_status_bar;
	Preferences& m_preferences;

	InfXmppManager* m_xmpp_manager;
	InfGtkIo* m_io;
	InfGtkBrowserModel* m_browser_model;
	InfGtkBrowserView* m_browser_view;
	Gtk::ScrolledWindow m_scroll;

	Gtk::Expander m_expander;
	Gtk::HBox m_hbox;
	Gtk::Label m_label_hostname;
	Gtk::Entry m_entry_hostname;

	ResolvMap m_resolv_map;
	SignalActivate m_signal_activate;
};

}
	
#endif // _GOBBY_BROWSER_HPP_
