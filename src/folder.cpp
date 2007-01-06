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

#include "folder.hpp"

Gobby::Folder::Folder()
 : Gtk::Notebook()
{
}

Gobby::Folder::~Folder()
{
}

void Gobby::Folder::obby_start()
{
	set_sensitive(true);
}

void Gobby::Folder::obby_end()
{
	// TODO: Just remove the editable-attribute to allow the user to still
	// save the documents.
	set_sensitive(false);
}

void Gobby::Folder::obby_user_join(obby::user& user)
{
}

void Gobby::Folder::obby_user_part(obby::user& user)
{
}

void Gobby::Folder::obby_document_insert(obby::document& document)
{
}

void Gobby::Folder::obby_document_remove(obby::document& document)
{
}
