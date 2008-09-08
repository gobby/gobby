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

#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include <gtkmm/box.h>

#include <libinftext/inf-text-session.h>
#include <libinftext/inf-text-user.h>

namespace Gobby
{
	class UserList: public Gtk::VBox
	{
	public:
		UserList(InfTextSession* session);
		~UserList();

	protected:
		InfTextSession* m_session;

		class Columns: public Gtk::TreeModelColumnRecord
		{
		public:
			Gtk::TreeModelColumn<InfTextUser*> user;
			Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > color;
			Gtk::TreeModelColumn<gulong> notify_color_handle;
			Gtk::TreeModelColumn<gulong> notify_status_handle;

			Columns()
			{
				add(user);
				add(color);
				add(notify_color_handle);
				add(notify_status_handle);
			}
		};

		static void on_add_user_static(InfUserTable* user_table,
		                               InfUser* user,
		                               gpointer user_data)
		{
			static_cast<UserList*>(user_data)->
				on_add_user(INF_TEXT_USER(user));
		}

		static void on_notify_color_static(InfUser* user,
		                                   GParamSpec* pspec,
		                                   gpointer user_data)
		{
			static_cast<UserList*>(user_data)->
				on_notify_color(INF_TEXT_USER(user));
		}

		static void on_notify_status_static(InfUser* user,
		                                    GParamSpec* pspec,
		                                    gpointer user_data)
		{
			static_cast<UserList*>(user_data)->
				on_notify_status(INF_TEXT_USER(user));
		}

		void icon_cell_data_func(Gtk::CellRenderer* renderer,
		                         const Gtk::TreeIter& iter);
		void color_cell_data_func(Gtk::CellRenderer* renderer,
		                          const Gtk::TreeIter& iter);
		void name_cell_data_func(Gtk::CellRenderer* renderer,
		                         const Gtk::TreeIter& iter);
		int sort_func(const Gtk::TreeIter& iter1,
		              const Gtk::TreeIter& iter2);

		void on_add_user(InfTextUser* user);
		void on_notify_color(InfTextUser* user);
		void on_notify_status(InfTextUser* user);

		Gtk::TreeIter find_user_iter(InfTextUser* user);

		Columns m_columns;
		Glib::RefPtr<Gtk::ListStore> m_store;
		Gtk::TreeView m_view;

		gulong m_add_user_handle;
	};
}

#endif // _GOBBY_ICON_HPP_
