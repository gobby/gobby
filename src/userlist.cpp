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

#include "userlist.hpp"

Gobby::UserList::Columns::Columns()
{
	add(name);
	add(color);
}

Gobby::UserList::Columns::~Columns()
{
}

Gobby::UserList::UserList()
{
	m_list_data = Gtk::ListStore::create(m_list_cols);
	m_list_view.set_model(m_list_data);

	m_list_view.append_column("Color", m_list_cols.color);
	m_list_view.append_column("Name", m_list_cols.name);

	set_shadow_type(Gtk::SHADOW_IN);
	add(m_list_view);

	set_sensitive(false);
}

Gobby::UserList::~UserList()
{
}

void Gobby::UserList::obby_start()
{
	set_sensitive(true);
}

void Gobby::UserList::obby_end()
{
	m_list_data->clear();
	set_sensitive(false);
}

void Gobby::UserList::obby_user_join(obby::user& user)
{
	Gtk::TreeModel::Row row = *(m_list_data->append() );

	unsigned int red = user.get_red();
	unsigned int green = user.get_green();
	unsigned int blue = user.get_blue();

	Glib::RefPtr<Gdk::Pixbuf> pixbuf =
		Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, false, 8, 16, 16);
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

	row[m_list_cols.name] = user.get_name();
	row[m_list_cols.color] = pixbuf;
}

void Gobby::UserList::obby_user_part(obby::user& user)
{
	Gtk::TreeModel::iterator iter = find_user(user.get_name() );
	if(iter == m_list_data->children().end() ) return;

	m_list_data->erase(iter);
}

void Gobby::UserList::obby_document_insert(obby::document& document)
{
	
}

void Gobby::UserList::obby_document_remove(obby::document& document)
{
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

