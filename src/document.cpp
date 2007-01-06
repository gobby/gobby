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

#include <obby/client_document.hpp>
#include "document.hpp"
#include "folder.hpp"

Gobby::Document::Document(obby::document& doc, const Folder& folder)
 : Gtk::ScrolledWindow(), m_doc(doc), m_folder(folder), m_editing(true)
{
#ifdef WITH_GTKSOURCEVIEW
	m_view.set_show_line_numbers(true);
	Glib::RefPtr<Gtk::SourceBuffer> buf = m_view.get_buffer();
#else
	Glib::RefPtr<Gtk::TextBuffer> buf = m_view.get_buffer();
#endif

	// Set monospaced font
	Pango::FontDescription desc;
	desc.set_family("monospace");
	m_view.modify_font(desc);


#ifdef WITH_GTKSOURCEVIEW
	// Set SourceLanguage by file extension
	Glib::ustring mime_type =
		folder.get_mime_map().get_mime_type_by_file(doc.get_title() );
	if(!mime_type.empty() )
	{
		Glib::RefPtr<Gtk::SourceLanguagesManager> manager =
			folder.get_lang_manager();
		Glib::RefPtr<Gtk::SourceLanguage> language = 
			manager->get_language_from_mime_type(mime_type);

		if(language)
			buf->set_language(language);
	}

	buf->set_highlight(true);
#endif

	// Textbuffer signal handlers
	buf->signal_insert().connect(
		sigc::mem_fun(*this, &Document::on_insert_before), false);
	buf->signal_erase().connect(
		sigc::mem_fun(*this, &Document::on_erase_before), false);
	buf->signal_insert().connect(
		sigc::mem_fun(*this, &Document::on_insert_after), true);
	buf->signal_erase().connect(
		sigc::mem_fun(*this, &Document::on_erase_after), true);
	buf->signal_mark_set().connect(
		sigc::mem_fun(*this, &Document::on_cursor_changed) );

	// Obby signal handlers
	doc.insert_event().before().connect(
		sigc::mem_fun(*this, &Document::on_obby_insert) );
	doc.delete_event().before().connect(
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

Gobby::Document::signal_update_type Gobby::Document::update_event() const
{
	return m_signal_update;
}
	
void Gobby::Document::get_cursor_position(unsigned int& row,
                                          unsigned int& col)
{
	// Get insert mark
	Glib::RefPtr<Gtk::TextBuffer::Mark> mark =
		m_view.get_buffer()->get_mark("insert");

	// Get corresponding iterator
	// Gtk::TextBuffer::Mark::get_iter is not const. Why not? It prevents
	// this function from being const.
	const Gtk::TextBuffer::iterator iter = mark->get_iter();

	// Read line and column
	row = iter.get_line();
	col = iter.get_line_offset();
}

unsigned int Gobby::Document::get_unsynced_changes_count() const
{
	obby::client_document* doc = 
		dynamic_cast<obby::client_document*>(&m_doc);

	// Changes in Server/Host documents are always synced
	if(doc == NULL)
		return 0;

	return doc->get_unsynced_changes_count();
}

#ifdef WITH_GTKSOURCEVIEW
Glib::RefPtr<Gtk::SourceLanguage> Gobby::Document::get_language() const
{
	return m_view.get_buffer()->get_language();
}
#endif

void Gobby::Document::on_insert_before(const Gtk::TextBuffer::iterator& begin,
                                       const Glib::ustring& text,
                                       int bytes)
{
	if(m_editing) return;
	m_editing = true;
	
	m_doc.insert(
		m_doc.coord_to_position(
			begin.get_line(),
			begin.get_line_index()
		),
		text
	);

	m_editing = false;
}

void Gobby::Document::on_erase_before(const Gtk::TextBuffer::iterator& begin,
                                      const Gtk::TextBuffer::iterator& end)
{
	if(m_editing) return;
	m_editing = true;

	m_doc.erase(
		m_doc.coord_to_position(
			begin.get_line(),
			begin.get_line_index()
		),
		m_doc.coord_to_position(
			end.get_line(),
			end.get_line_index()
		)
	);

	m_editing = false;
}

void Gobby::Document::on_obby_insert(const obby::insert_record& record)
{
	if(m_editing) return;
	m_editing = true;

	Glib::RefPtr<Gtk::TextBuffer> buffer = m_view.get_buffer();

	unsigned int row, col;
	m_doc.position_to_coord(record.get_position(), row, col);
	buffer->insert(
		buffer->get_iter_at_line_index(row, col),
		record.get_text()
	);

	m_editing = false;
}

void Gobby::Document::on_obby_delete(const obby::delete_record& record)
{
	if(m_editing) return;
	m_editing = true;

	Glib::RefPtr<Gtk::TextBuffer> buffer = m_view.get_buffer();

	unsigned int brow, bcol, erow, ecol;
	m_doc.position_to_coord(record.get_begin(), brow, bcol);
	m_doc.position_to_coord(record.get_end(), erow, ecol);

	buffer->erase(
		buffer->get_iter_at_line_index(brow, bcol),
		buffer->get_iter_at_line_index(erow, ecol)
	);
	m_editing = false;
}

void Gobby::Document::on_insert_after(const Gtk::TextBuffer::iterator& begin,
                                      const Glib::ustring& text,
                                      int bytes)
{
	// Document changed: Update statusbar
	m_signal_update.emit();
}

void Gobby::Document::on_erase_after(const Gtk::TextBuffer::iterator& begin,
                                     const Gtk::TextBuffer::iterator& end)
{
	// Document changed: Update statusbar
	m_signal_update.emit();
}

void Gobby::Document::on_cursor_changed(
	const Gtk::TextBuffer::iterator& location,
	const Glib::RefPtr<Gtk::TextBuffer::Mark>& mark
)
{
	// Insert mark changed position: Update status bar
	if(mark->get_name() == "insert")
		m_signal_update.emit();
}

