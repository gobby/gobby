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
#include <obby/format_string.hpp>
#include <obby/local_buffer.hpp>
#include "common.hpp"
#include "statusbar.hpp"

Gobby::StatusBar::StatusBar(const Folder& folder)
 : Frame(), 
   m_language("", Gtk::ALIGN_LEFT),
   m_sync("", Gtk::ALIGN_LEFT),
   m_revision("", Gtk::ALIGN_LEFT),
   m_position("", Gtk::ALIGN_LEFT)
{
	m_box.pack_start(m_language);
	m_box.pack_start(m_sync);
	m_box.pack_start(m_revision);
	m_box.pack_start(m_position);
	m_box.set_homogeneous(true);

	add(m_box);
	set_shadow_type(Gtk::SHADOW_OUT);

	folder.document_cursor_moved_event().connect(
		sigc::mem_fun(*this, &StatusBar::update_cursor) );
	folder.document_content_changed_event().connect(
		sigc::mem_fun(*this, &StatusBar::update_sync) );
	folder.document_content_changed_event().connect(
		sigc::mem_fun(*this, &StatusBar::update_revision) );
#ifdef WITH_GTKSOURCEVIEW
	folder.document_language_changed_event().connect(
		sigc::mem_fun(*this, &StatusBar::update_language) );
#endif
	folder.tab_switched_event().connect(
		sigc::mem_fun(*this, &StatusBar::update_all) );
}

Gobby::StatusBar::~StatusBar()
{
}

#ifdef WITH_GTKSOURCEVIEW
#include <iostream>
void Gobby::StatusBar::update_language(Document& document)
{
	// Selected language
	if(document.get_language() )
	{
		obby::format_string str(_("Selected language: %0") );
		str << document.get_language()->get_name().raw();
		m_language.set_text(str.str() );
	}
	else
	{
		m_language.set_text(_("No language selected") );
	}
}
#endif

void Gobby::StatusBar::update_sync(Document& document)
{
	unsigned int n = document.get_unsynced_changes_count();
	obby::format_string str(
		ngettext("%0 pending change", "%0 pending changes", n) );
	str << n;
	m_sync.set_text(str.str() );
}

void Gobby::StatusBar::update_revision(Document& document)
{
	obby::format_string str(_("Revision: %0") );
	str << document.get_revision();
	m_revision.set_text(str.str() );
}

void Gobby::StatusBar::update_cursor(Document& document)
{
	unsigned int row, col;
	document.get_cursor_position(row, col);

	obby::format_string str("Line: %0 Column: %1");
	str << (row + 1) << (col + 1);
	m_position.set_text(str.str() );
}

void Gobby::StatusBar::update_all(Document& document)
{
#ifdef WITH_GTKSOURCEVIEW
	update_language(document);
#endif
	update_sync(document);
	update_revision(document);
	update_cursor(document);
}

void Gobby::StatusBar::obby_start(obby::local_buffer& buf)
{
}

void Gobby::StatusBar::obby_end()
{
	m_language.set_text("");
	m_sync.set_text("");
	m_revision.set_text("");
	m_position.set_text("");
}

void Gobby::StatusBar::obby_user_join(obby::user& user)
{
}

void Gobby::StatusBar::obby_user_part(obby::user& user)
{
}

void Gobby::StatusBar::obby_document_insert(obby::local_document_info& document)
{
}

void Gobby::StatusBar::obby_document_remove(obby::local_document_info& document)
{
	// Last document that is closed?
	if(document.get_buffer().document_count() == 1)
	{
		// Clear statusbar
		obby_end();
	}
}

void Gobby::StatusBar::obby_preferences_changed(const Preferences& preferences)
{
}

