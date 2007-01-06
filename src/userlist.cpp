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
	Glib::RefPtr<Gdk::Pixbuf>
	create_coloured_pixbuf(const obby::colour& colour)
	{
		Glib::RefPtr<Gdk::Pixbuf> pixbuf =
			Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, false, 8, 16,
				16);
		pixbuf->fill( (colour.get_red() << 24) | (colour.get_green() << 16) | (colour.get_blue() << 8) );

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
	add(icon);
	add(text);
}

Gobby::UserList::Columns::~Columns()
{
}

Gobby::UserList::UserList(Header& header):
	m_header(header)
{
	m_tree_data = Gtk::TreeStore::create(m_tree_cols);

	m_view_col.pack_start(m_tree_cols.icon, false);
	m_view_col.pack_start(m_tree_cols.text, false);
	m_view_col.set_spacing(5);

	m_tree_view.set_model(m_tree_data);
	m_tree_view.append_column(m_view_col);

	m_iter_online = m_tree_data->append();
	m_iter_offline = m_tree_data->append();

	(*m_iter_online)[m_tree_cols.text] = _("Online");
	(*m_iter_offline)[m_tree_cols.text] = _("Offline");

	m_view_col.set_sort_column(m_tree_cols.text);

	m_tree_view.get_selection()->set_mode(Gtk::SELECTION_NONE);
	m_tree_view.set_headers_visible(false);

	set_shadow_type(Gtk::SHADOW_IN);
	set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	set_sensitive(false);

	add(m_tree_view);
}

Gobby::UserList::~UserList()
{
}

void Gobby::UserList::obby_start(obby::local_buffer& buf)
{
	set_sensitive(true);
}

void Gobby::UserList::obby_end()
{
	set_sensitive(false);
	// TODO: Remove all users in the list
}

void Gobby::UserList::obby_user_join(const obby::user& user)
{
	// TODO: Check connected flag, add user to correct column, remove from
	// other, if necessary
}

void Gobby::UserList::obby_user_part(const obby::user& user)
{
	// TODO: Remove user from online column, add to offline
}

void Gobby::UserList::obby_user_colour(const obby::user& user)
{
	// Change user colour of given user
}

void Gobby::UserList::obby_document_insert(obby::local_document_info& info)
{
	// TODO: Add to (un)subscribe event, add corresponding documents as
	// child of user into list.
}

void Gobby::UserList::obby_document_remove(obby::local_document_info& document)
{
}
