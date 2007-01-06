/* gobby - A GTKmm driven libobby client
 * Copyright (C) 2005 0x539 dev group
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef _GOBBY_USERLIST_HPP_
#define _GOBBY_USERLIST_HPP_

#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>

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
		Gtk::TreeModelColumn<Gdk::Color> color;
	};

	UserList();
	~UserList();

protected:
	Gtk::TreeView m_list_view;
	Glib::RefPtr<Gtk::ListStore> m_list_data;
	Columns m_list_cols;
};

}

#endif // _GOBBY_USERLIST_HPP_
