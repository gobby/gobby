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

#include <sstream>
#include "common.hpp"
#include "statusbar.hpp"

Gobby::StatusBar::StatusBar()
 : Frame(), 
   m_language("", Gtk::ALIGN_LEFT),
   m_sync("", Gtk::ALIGN_LEFT),
   m_position("", Gtk::ALIGN_LEFT)
{
	m_box.pack_start(m_language);
	m_box.pack_start(m_sync);
	m_box.pack_start(m_position);
	m_box.set_homogeneous(true);

	add(m_box);
	set_shadow_type(Gtk::SHADOW_OUT);
}

Gobby::StatusBar::~StatusBar()
{
}

void Gobby::StatusBar::update(Document& document)
{
	// Unsynced changes
	unsigned int unsynced_count = document.get_unsynced_changes_count();
	
/*	std::stringstream sync_str;
	if(!unsynced_count)
		sync_str << _("In sync");
	else
		sync_str << unsynced_count << _(" unsynced change(s)");*/
	
	// Position
	unsigned int row, col;
	document.get_cursor_position(row, col);
	++ row; ++ col;

	std::stringstream pos_str;
	pos_str << _("Line: ") << row << _(" Column: ") << col;

#ifdef WITH_GTKSOURCEVIEW
	// Selected language
	if(document.get_language() )
		m_language.set_text(
			_("Selected language: ") +
			document.get_language()->get_name()
		);
	else
		m_language.set_text(_("No language selected") );
#endif
//	m_sync.set_text(sync_str.str() );
	m_position.set_text(pos_str.str() );
}

void Gobby::StatusBar::obby_start()
{
}

void Gobby::StatusBar::obby_end()
{
	m_language.set_text("");
	m_position.set_text("");
	m_sync.set_text("");
}

void Gobby::StatusBar::obby_user_join(obby::user& user)
{
}

void Gobby::StatusBar::obby_user_part(obby::user& user)
{
}

void Gobby::StatusBar::obby_document_insert(obby::document& document)
{
}

void Gobby::StatusBar::obby_document_remove(obby::document& document)
{
}

