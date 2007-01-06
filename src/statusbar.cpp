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
#include <gtkmm/separator.h>
#include <obby/format_string.hpp>
#include <obby/local_buffer.hpp>
#include "common.hpp"
#include "statusbar.hpp"

Gobby::StatusBar::StatusBar(Header& header, const Folder& folder):
	m_header(header), m_context_noconn(0), m_context_state(0),
	m_language("", Gtk::ALIGN_LEFT), m_position("", Gtk::ALIGN_LEFT)
	//m_connection("Not connected", Gtk::ALIGN_LEFT),
{
	pack_start(m_language, Gtk::PACK_EXPAND_WIDGET);
	pack_end(m_position, Gtk::PACK_SHRINK);
	set_spacing(10);

	m_language.set_ellipsize(Pango::ELLIPSIZE_END);

	//set_shadow_type(Gtk::SHADOW_OUT);

	m_context_noconn = push(_("Not connected"));

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
	if(wnd.get_language() )
	{
		obby::format_string str(_("Selected language: %0%") );
		str << wnd.get_language()->get_name().raw();
		m_language.set_text(str.str() );
	}
	else
	{
		m_language.set_text(_("No language selected") );
	}

	//m_sep.show();
}

void Gobby::StatusBar::update_cursor(DocWindow& wnd)
{
	unsigned int row, col;
	wnd.get_cursor_position(row, col);

	obby::format_string str("Line: %0%, Column: %1%");
	str << (row + 1) << (col + 1);
	m_position.set_text(str.str() );
}

void Gobby::StatusBar::update_from_document(DocWindow& wnd)
{
	update_language(wnd);
	update_cursor(wnd);
}

void Gobby::StatusBar::update_connection(const Glib::ustring& str)
{
	// TODO: Do this in obby_start!
	m_context_state = push(str);
}

void Gobby::StatusBar::obby_start(LocalBuffer& buf)
{
}

void Gobby::StatusBar::obby_end()
{
	m_language.set_text("");
	pop(m_context_state); m_context_state = 0;
	m_position.set_text("");
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
		m_language.set_text("");
		//m_sep.hide();
		m_position.set_text("");
	}
}

void Gobby::StatusBar::on_show()
{
	Gtk::Statusbar::on_show();
	//m_sep.hide();
}

