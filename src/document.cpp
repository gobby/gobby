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
#include <obby/host_document.hpp>
#include <obby/client_buffer.hpp>
#include <obby/host_buffer.hpp>
#include "document.hpp"
#include "folder.hpp"

#include <iostream>
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

	// Insert users from user table to insert users that have already
	// left the obby session.
	const obby::buffer& obbybuf = doc.get_buffer();
	const obby::user_table& user_table = doc.get_buffer().get_user_table();
	for(obby::user_table::user_iterator iter = user_table.user_begin();
	    iter != user_table.user_end();
	    ++ iter)
	{
		// Create new tag
		Glib::RefPtr<Gtk::TextBuffer::Tag> tag =
			buf->create_tag("gobby_user_" + iter->get_name() );

		// Build user color
		Gdk::Color color;
		color.set_red(iter->get_red() * 65535 / 255);
		color.set_green(iter->get_green() * 65535 / 255);
		color.set_blue(iter->get_blue() * 65535 / 255);

		// Assign color to tag
		tag->property_background_gdk() = color;
	}

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

	// Set initial authors
	for(unsigned int i = 0; i < doc.get_line_count(); ++ i)
	{
		// Get current line
		const obby::line& line = doc.get_line(i);
		obby::line::author_iterator prev = line.author_begin();
		obby::line::author_iterator cur = prev;

		// Iterate through it's authors list
		for(++ cur; prev != line.author_end(); ++ cur)
		{
			// Get current user
			const obby::user_table::user* user = prev->author;
			// user should never be NULL...
			if(user == NULL) continue;

			// Get the range to highlight
			obby::line::size_type endpos;
			if(cur != line.author_end() )
				endpos = cur->position;
			else
				endpos = line.length();

			Gtk::TextBuffer::iterator begin =
				buf->get_iter_at_line_index(i, prev->position);
			Gtk::TextBuffer::iterator end =
				buf->get_iter_at_line_index(i, endpos);

			// Apply corresponding tag
			buf->apply_tag_by_name(
				"gobby_user_" + user->get_name(),
				begin,
				end
			);

			prev = cur;
		}
	}

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

	// Read line and column from iterator
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

void Gobby::Document::obby_user_join(obby::user& user)
{
	// Build tag name for this user
	Glib::ustring tag_name = "gobby_user_" + user.get_name();

	// Find already existing tag
	Glib::RefPtr<Gtk::TextBuffer> buffer = m_view.get_buffer();
	Glib::RefPtr<Gtk::TextBuffer::TagTable> tag_table =
		buffer->get_tag_table();
	Glib::RefPtr<Gtk::TextBuffer::Tag> tag = tag_table->lookup(tag_name);

	// Create new tag, if it doesn't exist
	if(!tag)
		tag = buffer->create_tag(tag_name);

	// Build color
	Gdk::Color color;
	color.set_red(user.get_red() * 65535 / 255);
	color.set_green(user.get_green() * 65535 / 255);
	color.set_blue(user.get_blue() * 65535 / 255);

	// Set/Update color
	tag->property_background_gdk() = color;
}

void Gobby::Document::obby_user_part(obby::user& user)
{
}

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

	// Get textbuffer
	Glib::RefPtr<Gtk::TextBuffer> buffer = m_view.get_buffer();

	// Translate position to row/column
	unsigned int row, col;
	m_doc.position_to_coord(record.get_position(), row, col);

	// Find obby::user that inserted the text
	obby::user* user = m_doc.get_buffer().find_user(record.get_from() );
	assert(user != NULL);

	// Insert text
	Gtk::TextBuffer::iterator end = buffer->insert(
		buffer->get_iter_at_line_index(row, col),
		record.get_text()
	);

	// Colourize new text
	Gtk::TextBuffer::iterator begin = end;
	begin.backward_chars(record.get_text().length() );
	update_user_colour(begin, end, *user);

	m_view.queue_draw();
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

void Gobby::Document::on_insert_after(const Gtk::TextBuffer::iterator& end,
                                      const Glib::ustring& text,
                                      int bytes)
{
	// Other editing function is at work.
	if(!m_editing)
	{
		// TODO: Find a better solution to access the local user object.
		obby::client_document* client_doc =
			dynamic_cast<obby::client_document*>(&m_doc);
		obby::host_document* host_doc =
			dynamic_cast<obby::host_document*>(&m_doc);

		const obby::user* user = NULL;
		if(client_doc != NULL)
			user = &client_doc->get_buffer().get_self();
		if(host_doc != NULL)
			user = &host_doc->get_buffer().get_self();

		assert(user != NULL);

		// Find start position of new text
		Gtk::TextBuffer::iterator pos = end;
		pos.backward_chars(text.length() );

		// Update colour
		update_user_colour(pos, end, *user);
	}

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

void Gobby::Document::update_user_colour(const Gtk::TextBuffer::iterator& begin,
                                         const Gtk::TextBuffer::iterator& end,
                                         const obby::user& user)
{
	// Remove other user tags in that range
	Glib::RefPtr<Gtk::TextBuffer> buffer = m_view.get_buffer();
	Glib::RefPtr<Gtk::TextBuffer::TagTable> tag_table =
		buffer->get_tag_table();

	tag_table->foreach(
		sigc::bind(
			sigc::mem_fun(
				*this,
				&Document::on_remove_user_colour
			),
			sigc::ref(begin),
			sigc::ref(end)
		)
	);

	// Insert new user tag to the given range
	buffer->apply_tag_by_name("gobby_user_" + user.get_name(), begin, end);
}

void
Gobby::Document::on_remove_user_colour(Glib::RefPtr<Gtk::TextBuffer::Tag> tag,
                                       const Gtk::TextBuffer::iterator& begin,
				       const Gtk::TextBuffer::iterator& end)
{
	// Remove tag if it is a user color tag.
	Glib::ustring tag_name = tag->property_name();
	if(tag_name.compare(0, 10, "gobby_user") == 0)
		m_view.get_buffer()->remove_tag(tag, begin, end);
}

