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
#include <obby/local_buffer.hpp>
#include "folder.hpp"

namespace Gobby
{

/** List showing users that are participating in the obby session.
 */
class UserList : public Gtk::ScrolledWindow
{
public:
	class Columns : public Gtk::TreeModel::ColumnRecord
	{
	public:
		Columns();
		~Columns();

		Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > colour;
		Gtk::TreeModelColumn<Glib::ustring> name;
		Gtk::TreeModelColumn<bool> connected;
		Gtk::TreeModelColumn<bool> subscribed;
	};

	UserList(const Folder& folder);
	~UserList();

	// Calls from the user folder
	// TODO: Replace them by signal handlers from buf
	// TODO: Context menu to show/hide non-connected users
	// TODO: Nix auswaehlbar machen?
	virtual void obby_start(obby::local_buffer& buf);
	virtual void obby_end();
	virtual void obby_user_join(obby::user& user);
	virtual void obby_user_part(obby::user& user);
	virtual void obby_user_colour(obby::user& user);
	virtual void obby_document_insert(obby::local_document_info& document);
	virtual void obby_document_remove(obby::local_document_info& document);
protected:
	/** Signal handlers
	 */
	void on_folder_tab_switched(Document& document);
	void on_user_subscribe(const obby::user& user,
	                       obby::local_document_info& info);
	void on_user_unsubscribe(const obby::user& user,
	                         obby::local_document_info& info);

	/** Helper functions
	 */
	Gtk::TreeModel::iterator find_user(const Glib::ustring& name) const;
	void add_user(const obby::user& user);

	/** Currently selected document.
	 */
	obby::local_document_info* m_info;

	/** GUI components.
	 */
	Gtk::TreeView m_list_view;
	Glib::RefPtr<Gtk::ListStore> m_list_data;
	Columns m_list_cols;

	Gtk::TreeViewColumn* m_view_col_colour;
	Gtk::TreeViewColumn* m_view_col_name;
	Gtk::TreeViewColumn* m_view_col_connected;
	Gtk::TreeViewColumn* m_view_col_subscribed;
};

#if 0
/** UserList-derivated class that lists all users in the session.
 */
class UserListSession : public UserList
{
public:
	UserListSession(const Folder& folder);
	~UserListSession();

protected:
	virtual void obby_start(obby::local_buffer& buf);
	virtual void obby_end();
	virtual void obby_user_join(obby::user& user);
	virtual void obby_user_part(obby::user& user);
};

/** UserList derivated class that lists all users who are subscribed to the
 * currently active document.
 */
class UserListDocument : public UserList
{
public:
	UserListDocument(const Folder& folder);
	~UserListDocument();

protected:
	/** Window delegates
	 */
	virtual void obby_start(obby::local_buffer& buf);
	virtual void obby_end();
	virtual void obby_user_join(obby::user& user);
	virtual void obby_user_part(obby::user& user);
	virtual void obby_document_insert(obby::local_document_info& info);
	virtual void obby_document_remove(obby::local_document_info& info);

};
#endif

}

#endif // _GOBBY_USERLIST_HPP_
