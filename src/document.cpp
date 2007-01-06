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

#include "document.hpp"

Gobby::Document::Document(obby::document& doc)
 : Gtk::ScrolledWindow(), m_doc(doc), m_editing(true)
{
	Glib::RefPtr<Gtk::TextBuffer> buf = m_view.get_buffer();

	// Textbuffer signal handlers
	buf->signal_insert().connect(
		sigc::mem_fun(*this, &Document::on_insert), false);
	buf->signal_erase().connect(
		sigc::mem_fun(*this, &Document::on_erase), false);

	// Obby signal handlers
	doc.insert_event().connect(
		sigc::mem_fun(*this, &Document::on_obby_insert) );
	doc.delete_event().connect(
		sigc::mem_fun(*this, &Document::on_obby_delete) );

	// Set initial text
	buf->set_text(doc.get_whole_buffer() );
	m_editing = false;
	
	set_shadow_type(Gtk::SHADOW_IN);
	set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	add(m_view);
}

Gobby::Document::~Document()
{
}

const obby::document& Gobby::Document::get_document() const
{
	return m_doc;
}

obby::document& Gobby::Document::get_document()
{
	return m_doc;
}

#include <iostream>
void Gobby::Document::on_insert(const Gtk::TextBuffer::iterator& begin,
                                const Glib::ustring& text,
                                int foo)
{
	if(m_editing) return;

	std::cout << "Insert " << text << " at " << begin.get_offset() << std::endl;
	m_doc.insert(begin.get_offset(), text);
}

void Gobby::Document::on_erase(const Gtk::TextBuffer::iterator& begin,
                               const Gtk::TextBuffer::iterator& end)
{
	if(m_editing) return;
	
	std::cout << "Erasing from " << begin.get_offset()
	          << " to " << end.get_offset() << std::endl;
	m_doc.erase(begin.get_offset(), end.get_offset() );
}

void Gobby::Document::on_obby_insert(const obby::insert_record& record)
{
	m_editing = true;
	Glib::RefPtr<Gtk::TextBuffer> buffer = m_view.get_buffer();
	buffer->insert(buffer->get_iter_at_offset(record.get_position()),
	               record.get_text() );
	m_editing = false;
}

void Gobby::Document::on_obby_delete(const obby::delete_record& record)
{
	m_editing = true;
	Glib::RefPtr<Gtk::TextBuffer> buffer = m_view.get_buffer();
	buffer->erase(buffer->get_iter_at_offset(record.get_begin()),
	              buffer->get_iter_at_offset(record.get_end()) );
	m_editing = false;
}

