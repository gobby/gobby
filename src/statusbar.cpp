/* gobby - A GTKmm driven libobby client
 * Copyright (C) 2005, 2006 0x539 dev group
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
#include <gtkmm/separator.h>
#include <obby/format_string.hpp>
#include <obby/local_buffer.hpp>
#include "common.hpp"
#include "statusbar.hpp"

Gobby::StatusBar::StatusBar(Header& header, const Folder& folder):
	m_header(header)
{
	pack_end(m_bar_position, Gtk::PACK_SHRINK);
	pack_end(m_bar_language, Gtk::PACK_SHRINK);

	m_bar_position.set_size_request(200, -1);
	m_bar_language.set_size_request(200, -1);

	set_has_resize_grip(false);
	m_bar_language.set_has_resize_grip(false);

	push(_("Not connected"));

	folder.document_cursor_moved_event().connect(
		sigc::mem_fun(*this, &StatusBar::update_cursor) );
	folder.document_language_changed_event().connect(
		sigc::mem_fun(*this, &StatusBar::update_language) );
	folder.tab_switched_event().connect(
		sigc::mem_fun(*this, &StatusBar::update_from_document) );
}

void Gobby::StatusBar::update_language(DocWindow& wnd)
{
	// Selected language
	m_bar_language.pop();
	if(wnd.get_language() )
	{
    Glib::ustring name = gtk_source_language_get_name(wnd.get_language());
		obby::format_string str(_("Selected language: %0%") );
		str << name.raw();
		m_bar_language.push(str.str() );
	}
	else
	{
		m_bar_language.push(_("No language selected") );
	}
}

void Gobby::StatusBar::update_cursor(DocWindow& wnd)
{
	unsigned int row, col;
	wnd.get_cursor_position(row, col);

	m_bar_position.pop();
	obby::format_string str("Line: %0%, Column: %1%");
	str << (row + 1) << (col + 1);
	m_bar_position.push(str.str() );
}

void Gobby::StatusBar::update_from_document(DocWindow& wnd)
{
	update_language(wnd);
	update_cursor(wnd);
}

void Gobby::StatusBar::update_connection(const Glib::ustring& str)
{
	// TODO: Do this in obby_start!
	pop();
	push(str);
}

void Gobby::StatusBar::obby_start(LocalBuffer& buf)
{
}

void Gobby::StatusBar::obby_end()
{
	pop();
	m_bar_language.pop();
	m_bar_position.pop();

	push(_("Not connected"));
}

void Gobby::StatusBar::obby_user_join(const obby::user& user)
{
}

void Gobby::StatusBar::obby_user_part(const obby::user& user)
{
}

void Gobby::StatusBar::obby_document_insert(LocalDocumentInfo& document)
{
}

void Gobby::StatusBar::obby_document_remove(LocalDocumentInfo& document)
{
	// Last document that is closed?
	if(document.get_buffer().document_count() == 1)
	{
		// Clear document-related statusbar items
		m_bar_language.pop();
		m_bar_position.pop();
	}
}

void Gobby::StatusBar::on_show()
{
	Gtk::Statusbar::on_show();
	//m_sep.hide();
}

