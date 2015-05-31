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

#ifndef _GOBBY_USERLIST_HPP_
#define _GOBBY_USERLIST_HPP_

#include <gtkmm/treeview.h>
#include <gtkmm/treemodelfilter.h>
#include <gtkmm/liststore.h>
#include <gtkmm/grid.h>

#include <libinfinity/common/inf-user-table.h>
#include <libinftext/inf-text-user.h>

namespace Gobby
{
	class UserList: public Gtk::Grid
	{
	public:
		typedef sigc::signal<void, InfUser*> SignalUserActivated;

		UserList(InfUserTable* table);
		~UserList();

		void set_show_disconnected(bool show_disconnected);

		SignalUserActivated signal_user_activated() const
		{
			return m_signal_user_activated;
		}
		
	protected:
		InfUserTable* m_table;

		class Columns: public Gtk::TreeModelColumnRecord
		{
		public:
			Gtk::TreeModelColumn<InfUser*> user;
			Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > color;
			Gtk::TreeModelColumn<gulong> notify_hue_handle;
			Gtk::TreeModelColumn<gulong> notify_status_handle;

			Columns()
			{
				add(user);
				add(color);
				add(notify_hue_handle);
				add(notify_status_handle);
			}
		};

		static void on_add_user_static(InfUserTable* user_table,
		                               InfUser* user,
		                               gpointer user_data)
		{
			static_cast<UserList*>(user_data)->
				on_add_user(user);
		}

		static void on_notify_status_static(InfUser* user,
		                                    GParamSpec* pspec,
		                                    gpointer user_data)
		{
			static_cast<UserList*>(user_data)->
				on_notify_status(user);
		}

		static void on_notify_hue_static(InfUser* user,
		                                 GParamSpec* pspec,
		                                 gpointer user_data)
		{
			static_cast<UserList*>(user_data)->
				on_notify_hue(INF_TEXT_USER(user));
		}

		bool visible_func(const Gtk::TreeIter& iter);
		void icon_cell_data_func(Gtk::CellRenderer* renderer,
		                         const Gtk::TreeIter& iter);
		void color_cell_data_func(Gtk::CellRenderer* renderer,
		                          const Gtk::TreeIter& iter);
		void name_cell_data_func(Gtk::CellRenderer* renderer,
		                         const Gtk::TreeIter& iter);
		int sort_func(const Gtk::TreeIter& iter1,
		              const Gtk::TreeIter& iter2);

		void on_add_user(InfUser* user);
		void on_notify_status(InfUser* user);
		void on_notify_hue(InfTextUser* user);

		//void on_select_func(const Gtk::TreeIter& iter);
		void on_row_activated(const Gtk::TreePath& path,
		                      Gtk::TreeViewColumn* column);

		virtual void on_style_updated();

		Gtk::TreeIter find_user_iter(InfUser* user);

		Columns m_columns;
		Glib::RefPtr<Gtk::ListStore> m_store;
		Glib::RefPtr<Gtk::TreeModelFilter> m_filter_model;
		Gtk::TreeView m_view;

		gulong m_add_user_handle;

		SignalUserActivated m_signal_user_activated;
	};
}

#endif // _GOBBY_ICON_HPP_
