/* gobby - A GTKmm driven libobby client
 * Copyright (C) 2005 0x539 dev group
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

#ifndef _GOBBY_USERLIST_HPP_
#define _GOBBY_USERLIST_HPP_

#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include <obby/user.hpp>
#include <obby/local_document_info.hpp>

namespace Gobby
{

class UserList : public Gtk::ScrolledWindow
{
public:
	class Columns : public Gtk::TreeModel::ColumnRecord
	{
	public:
		Columns();
		~Columns();

		Gtk::TreeModelColumn<Glib::ustring> name;
//		Gtk::TreeModelColumn<Gdk::Color> color;
		Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > color;
	};

	UserList();
	~UserList();

	// Calls from the window
	void obby_start();
	void obby_end();
	void obby_user_join(obby::user& user);
	void obby_user_part(obby::user& user);
	void obby_document_insert(obby::local_document_info& document);
	void obby_document_remove(obby::local_document_info& document);
protected:
	// Helper functions
	Gtk::TreeModel::iterator find_user(const Glib::ustring& name) const;

	Gtk::TreeView m_list_view;
	Glib::RefPtr<Gtk::ListStore> m_list_data;
	Columns m_list_cols;
};

}

#endif // _GOBBY_USERLIST_HPP_
