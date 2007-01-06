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

#include "common.hpp"
#include "userlist.hpp"

namespace
{
	/** Creates a pixbuf representing a user's colour.
	 */
	Glib::RefPtr<Gdk::Pixbuf> create_coloured_pixbuf(int red, int green,
	                                                 int blue)
	{
		Glib::RefPtr<Gdk::Pixbuf> pixbuf =
			Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, false, 8, 16,
				16);
		pixbuf->fill( (red << 24) | (green << 16) | (blue << 8) );

		// Border around the color
		guint8* pixels = pixbuf->get_pixels();
		for(unsigned int y = 0; y < 16; ++ y)
		{
			for(unsigned int x = 0; x < 16; ++ x)
			{
				if(x == 0 || y == 0 || x == 15 || y == 15)
				{
					pixels[0] = 0;
					pixels[1] = 0;
					pixels[2] = 0;
				}

				pixels += 3;
			}
		}

		return pixbuf;
	}
}


Gobby::UserList::Columns::Columns()
{
	add(colour);
	add(name);
	add(connected);
	add(subscribed);
}

Gobby::UserList::Columns::~Columns()
{
}

Gobby::UserList::UserList(Header& header, const Folder& folder):
	m_header(header), m_info(NULL)
{
	m_list_data = Gtk::ListStore::create(m_list_cols);
	m_list_view.set_model(m_list_data);

	m_list_view.append_column(_("Colour"), m_list_cols.colour);
	m_list_view.append_column(_("Name"), m_list_cols.name);
	m_list_view.append_column(_("Connected"), m_list_cols.connected);
	m_list_view.append_column(_("Subscribed"), m_list_cols.subscribed);

	// Store TreeViewColumns
	m_view_col_colour = m_list_view.get_column(0);
	m_view_col_name = m_list_view.get_column(1);
	m_view_col_connected = m_list_view.get_column(2);
	m_view_col_subscribed = m_list_view.get_column(3);

	// Let the user sort by these columns
	m_view_col_name->set_sort_column(m_list_cols.name);
	m_view_col_connected->set_sort_column(m_list_cols.connected);
	m_view_col_subscribed->set_sort_column(m_list_cols.subscribed);

	// Let the user reorder the columns
	for(int i = 0; i < 4; ++ i)
		m_list_view.get_column(i)->set_reorderable(true);

	// No users can be selected
	m_list_view.get_selection()->set_mode(Gtk::SELECTION_NONE);

	// Hide subscription column until a document has been inserted
	m_view_col_subscribed->set_visible(false);

	set_shadow_type(Gtk::SHADOW_IN);
	set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	set_sensitive(false);

	add(m_list_view);

	// Connect tab_switched_event from folder to change subscribed flag
	// according to current document.
	folder.tab_switched_event().connect(
		sigc::mem_fun(*this, &UserList::on_folder_tab_switched) );

	// UserList crashes with GTK+ 2.8.x if it gets too small and no entry
	set_size_request(200, 0);
}

Gobby::UserList::~UserList()
{
}

void Gobby::UserList::obby_start(obby::local_buffer& buf)
{
	// Enable list
	set_sensitive(true);
}

void Gobby::UserList::obby_end()
{
	// Clear list
	m_info = NULL;
	m_list_data->clear();
	// Hide subscription column
	m_view_col_subscribed->set_visible(false);
	// Disable list item
	set_sensitive(false);
}

void Gobby::UserList::obby_user_join(const obby::user& user)
{
	// Is there already such a user?
	Gtk::TreeModel::iterator iter = find_user(user.get_name() );
	if(iter == m_list_data->children().end() )
	{
		// No, so add the user to the list
		add_user(user);
	}
	else
	{
		// Update connected flag
		(*iter)[m_list_cols.connected] =
			user.get_flags() & obby::user::flags::CONNECTED;
	}
}

void Gobby::UserList::obby_user_part(const obby::user& user)
{
	// User is not anymore connceted
	Gtk::TreeModel::iterator iter = find_user(user.get_name() );
	(*iter)[m_list_cols.connected] = false;
}

void Gobby::UserList::obby_user_colour(const obby::user& user)
{
	// Get user with this name
	Gtk::TreeModel::iterator iter = find_user(user.get_name() );
	// Get new colour
	unsigned int red = user.get_colour().get_red();
	unsigned int green = user.get_colour().get_green();
	unsigned int blue = user.get_colour().get_blue();
	// Update it
	(*iter)[m_list_cols.colour] = create_coloured_pixbuf(red, green, blue);
}

void Gobby::UserList::obby_document_insert(obby::local_document_info& info)
{
	// Get notification when a user subscribed to this document
	info.subscribe_event().connect(sigc::bind(
		sigc::mem_fun(*this, &UserList::on_user_subscribe),
			sigc::ref(info)) );

	// Get notification when a user unsubscribed
	info.unsubscribe_event().connect(sigc::bind(
		sigc::mem_fun(*this, &UserList::on_user_unsubscribe),
			sigc::ref(info)) );
	
	// There is at least one document: Show subscription column
	m_view_col_subscribed->set_visible(true);
}

void Gobby::UserList::obby_document_remove(obby::local_document_info& document)
{
	// Is this the last document to be removed?
	if(document.get_buffer().document_count() == 1)
		// Hide subscription column then
		m_view_col_subscribed->set_visible(false);
}

void Gobby::UserList::on_folder_tab_switched(Document& document)
{
	// No document open
	if(!m_view_col_subscribed->get_visible() ) return;
	// Update current info
	obby::local_document_info* info = &document.get_document();
	// Same document
	if(info == m_info) return;
	m_info = info;
	// Clear current data
	m_list_data->clear();
	// Get user table
	const obby::user_table& table =
		info->get_buffer().get_user_table();
	// Add all users in user table
	for(obby::user_table::iterator iter =
		table.begin(obby::user::flags::NONE, obby::user::flags::NONE);
	    iter !=
		table.end(obby::user::flags::NONE, obby::user::flags::NONE);
	    ++ iter)
	{
		add_user(*iter);
	}
}

void Gobby::UserList::on_user_subscribe(const obby::user& user,
                                        obby::local_document_info& info)
{
	// Not current doc
	if(&info != m_info) return;
	// Find user
	Gtk::TreeModel::iterator iter = find_user(user.get_name() );
	// Update subscribed flag
	(*iter)[m_list_cols.subscribed] = true;
}

void Gobby::UserList::on_user_unsubscribe(const obby::user& user,
                                          obby::local_document_info& info)
{
	// Not current doc
	if(&info != m_info) return;
	// Find user
	Gtk::TreeModel::iterator iter = find_user(user.get_name() );
	// Update subscribed flag
	(*iter)[m_list_cols.subscribed] = false;
}

Gtk::TreeModel::iterator
Gobby::UserList::find_user(const Glib::ustring& name) const
{
	Gtk::TreeModel::iterator iter = m_list_data->children().begin();
	for(iter; iter != m_list_data->children().end(); ++ iter)
		if( (*iter)[m_list_cols.name] == name)
			return iter;
	return m_list_data->children().end();
}

void Gobby::UserList::add_user(const obby::user& user)
{
	Gtk::TreeModel::Row row = *(m_list_data->append() );

	unsigned int red = user.get_colour().get_red();
	unsigned int green = user.get_colour().get_green();
	unsigned int blue = user.get_colour().get_blue();

	row[m_list_cols.name] = user.get_name();
	row[m_list_cols.colour] = create_coloured_pixbuf(red, green, blue);
	row[m_list_cols.connected] =
		user.get_flags() & obby::user::flags::CONNECTED;

	if(m_info != NULL)
		row[m_list_cols.subscribed] = m_info->is_subscribed(user);
}

#if 0
Gobby::UserListSession::UserListSession(const Folder& folder)
 : UserList(folder)
{
	set_sensitive(false);
}

Gobby::UserListSession::~UserListSession()
{
}

void Gobby::UserListSession::obby_start(obby::local_buffer& buf)
{
}

void Gobby::UserListSession::obby_end()
{
}

void Gobby::UserListSession::obby_user_join(obby::user& user)
{
	Gtk::TreeModel::Row row = *(m_list_data->append() );

	unsigned int red = user.get_red();
	unsigned int green = user.get_green();
	unsigned int blue = user.get_blue();

	row[m_list_cols.name] = user.get_name();
	row[m_list_cols.connected] = true;
	row[m_list_cols.color] = create_coloured_pixbuf(red, green, blue);
}

void Gobby::UserListSession::obby_user_part(obby::user& user)
{
	Gtk::TreeModel::iterator iter = m_list_data->children().begin();
	(*iter)[m_list_cols.connected] = false;
}

Gobby::UserListDocument::UserListDocument(const Folder& folder)
 : UserList(folder), m_info(NULL)
{

	// Show subscribed users, not connected ones
	m_list_view.get_column(1)->set_title("Subscribed");
	set_sensitive(false);
}

Gobby::UserListDocument::~UserListDocument()
{
}

void Gobby::UserListDocument::obby_start(obby::local_buffer& buf)
{
}

void Gobby::UserListDocument::obby_end()
{
	m_info = NULL;
	m_list_data->clear();
	m_list_view.get_column(2)->set_visible(false);
	set_sensitive(false);
}

void Gobby::UserListDocument::obby_user_join(obby::user& user)
{
	// Are there open documents? So add the user.
	if(m_info)
		add_user(user);
	{
	}
}

void Gobby::UserListDocument::obby_user_part(obby::user& user)
{
}

void Gobby::UserListDocument::obby_document_insert(
	obby::local_document_info& info
)
{

	// There is at least one document: Enable widget
	set_sensitive(true);
}

void Gobby::UserListDocument::obby_document_remove(
	obby::local_document_info& info
)
{
	// Is this the last document that gets removed?
	if(info.get_buffer().document_count() == 1)
	{
		m_info = NULL;
		// Hide subcription column
		m_list_view.get_column(2)->set_visible(false);
	}
}
#endif
