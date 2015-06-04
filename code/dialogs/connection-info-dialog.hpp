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

#ifndef _GOBBY_CONNECTIONINFODIALOG_HPP_
#define _GOBBY_CONNECTIONINFODIALOG_HPP_

#include <gtkmm/dialog.h>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/builder.h>

#include <libinfgtk/inf-gtk-connection-view.h>

#include <libinfinity/common/inf-browser.h>
#include <libinfinity/server/infd-directory.h>

namespace Gobby
{

class ConnectionInfoDialog: public Gtk::Dialog
{
	friend class Gtk::Builder;
	ConnectionInfoDialog(GtkDialog* cobject,
	                     const Glib::RefPtr<Gtk::Builder>& builder);
public:
	~ConnectionInfoDialog();

	static std::auto_ptr<ConnectionInfoDialog> create(
		Gtk::Window& parent, InfBrowser* browser);

	void set_browser(InfBrowser* browser);
private:
	static void foreach_connection_func_static(InfXmlConnection* conn,
	                                           gpointer user_data)
	{
		static_cast<ConnectionInfoDialog*>(user_data)->
			foreach_connection_func(conn);
	}

	static void on_connection_added_static(InfdDirectory* directory,
	                                       InfXmlConnection* conn,
	                                       gpointer user_data)
	{
		static_cast<ConnectionInfoDialog*>(user_data)->
			on_connection_added(conn);
	}

	static void on_connection_removed_static(InfdDirectory* directory,
	                                         InfXmlConnection* conn,
	                                         gpointer user_data)
	{
		static_cast<ConnectionInfoDialog*>(user_data)->
			on_connection_removed(conn);
	}

	void foreach_connection_func(InfXmlConnection* conn);
	void on_connection_added(InfXmlConnection* conn);
	void on_connection_removed(InfXmlConnection* conn);

	void on_selection_changed();

	void icon_cell_data_func(Gtk::CellRenderer* renderer,
	                         const Gtk::TreeIter& iter);
	void name_cell_data_func(Gtk::CellRenderer* renderer,
	                         const Gtk::TreeIter& iter);

protected:
	Gtk::TreeIter find_connection(InfXmppConnection* conn);

	class Columns: public Gtk::TreeModelColumnRecord
	{
	public:
		Gtk::TreeModelColumn<InfXmppConnection*> connection;

		Columns()
		{
			add(connection);
		}
	};

	InfBrowser* m_browser;

	Columns m_columns;
	Glib::RefPtr<Gtk::ListStore> m_connection_store;

	Gtk::Image* m_image;
	Gtk::TreeView* m_connection_tree_view;
	Gtk::ScrolledWindow* m_connection_scroll;

	InfGtkConnectionView* m_connection_view;

	gulong m_connection_added_handler;
	gulong m_connection_removed_handler;

	bool m_empty;
};

}

#endif // _GOBBY_CONNECTIONINFODIALOG_HPP_

