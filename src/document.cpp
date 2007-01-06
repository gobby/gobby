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

#include <obby/format_string.hpp>
#include <obby/user_table.hpp>
#include "common.hpp"
#include "document.hpp"
#include "folder.hpp"

Gobby::Document::Document(obby::local_document_info& doc, const Folder& folder,
                          const Preferences& preferences)
 : Gtk::SourceView(),
   m_doc(doc),
   m_preferences(preferences), m_editing(true),
   m_title(doc.get_title() )
{
	// Documents can only show content if the local user is subscribed
	if(!doc.is_subscribed() )
		throw std::logic_error("Gobby::Document::Document");

	Glib::RefPtr<Gtk::SourceBuffer> buf = get_buffer();

	// Prevent from GTK sourceview's undo 
	buf->begin_not_undoable_action();

	// Set monospaced font
	Pango::FontDescription desc;
	desc.set_family("monospace");
	modify_font(desc);

	// Set SourceLanguage by file extension
	Glib::ustring mime_type =
		folder.get_mime_map().get_mime_type_by_file(doc.get_title() );

	if(!mime_type.empty() )
	{
		Glib::RefPtr<const Gtk::SourceLanguagesManager> manager =
			folder.get_lang_manager();
		Glib::RefPtr<Gtk::SourceLanguage> language = 
			manager->get_language_from_mime_type(mime_type);

		set_language(language);
	}

	// Switch syntax highlighting on
	buf->set_highlight(true);

	// Insert user tags into the tag table
	const obby::user_table& user_table = doc.get_buffer().get_user_table();
	for(obby::user_table::iterator iter =
		user_table.begin(
			obby::user::flags::NONE,
			obby::user::flags::NONE
		);
	    iter !=
		user_table.end(
			obby::user::flags::NONE,
			obby::user::flags::NONE
		);
	    ++ iter)
	{
		// Create new tag
		Glib::RefPtr<Gtk::TextBuffer::Tag> tag =
			buf->create_tag("gobby_user_" + iter->get_name() );

		// Build user color
		Gdk::Color color;
		color.set_red(iter->get_colour().get_red() * 65535 / 255);
		color.set_green(iter->get_colour().get_green() * 65535 / 255);
		color.set_blue(iter->get_colour().get_blue() * 65535 / 255);

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
		sigc::mem_fun(*this, &Document::on_mark_set), false);
	buf->signal_apply_tag().connect(
		sigc::mem_fun(*this, &Document::on_apply_tag_after), true);

	// Install signal handlers
	const obby::document& content = doc.get_content();
	content.insert_event().before().connect(
		sigc::mem_fun(*this, &Document::on_obby_insert_before) );
	content.insert_event().after().connect(
		sigc::mem_fun(*this, &Document::on_obby_insert_after) );
	content.delete_event().before().connect(
		sigc::mem_fun(*this, &Document::on_obby_delete_before) );
	content.delete_event().after().connect(
		sigc::mem_fun(*this, &Document::on_obby_delete_after) );

	// Apply preferences
	apply_preferences();

	// Set initial text
	m_editing = true;
	buf->set_text(content.get_text() );

	// Not modified when subscribed, if the text is empty
	buf->set_modified(
		content.get_line_count() != 1 ||
		content.get_line(0).length() != 0
	);

	// Set initial authors
	for(unsigned int i = 0; i < content.get_line_count(); ++ i)
	{
		// Get current line
		const obby::line& line = content.get_line(i);
		obby::line::author_iterator prev = line.author_begin();
		obby::line::author_iterator cur = prev;

		// Iterate through it's authors list
		for(++ cur; prev != line.author_end(); ++ cur)
		{
			// Get current user
			const obby::user* user = prev->author;

			// user can be NULL (server insert event, but then
			// we do not have to apply a tag or so).
			if(user == NULL) { prev = cur; continue; }

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
}

const obby::local_document_info& Gobby::Document::get_document() const
{
	return m_doc;
}

obby::local_document_info& Gobby::Document::get_document()
{
	return m_doc;
}

Gobby::Document::signal_cursor_moved_type
Gobby::Document::cursor_moved_event() const
{
	return m_signal_cursor_moved;
}

Gobby::Document::signal_content_changed_type
Gobby::Document::content_changed_event() const
{
	return m_signal_content_changed;
}

Gobby::Document::signal_language_changed_type
Gobby::Document::language_changed_event() const
{
	return m_signal_language_changed;
}

void Gobby::Document::get_cursor_position(unsigned int& row,
                                          unsigned int& col)
{
	// Get insert mark
	Glib::RefPtr<Gtk::TextBuffer::Mark> mark =
		get_buffer()->get_insert();

	// Get corresponding iterator
	// Gtk::TextBuffer::Mark::get_iter is not const. Why not? It prevents
	// this function from being const.
	const Gtk::TextBuffer::iterator iter = mark->get_iter();

	// Read line and column from iterator
	row = iter.get_line();
	col = iter.get_line_offset();

	// Add tab characters to col
	const std::string& line = m_doc.get_content().get_line(row);
	unsigned int tabs = m_preferences.editor.tab_width;

	// col chars
	std::string::size_type chars = col; col = 0;
	for(std::string::size_type i = 0; i < chars; ++ i)
	{
		unsigned int width = 1;
		if(line[i] == '\t')
		{
			width = (tabs - col % tabs) % tabs;
			if(width == 0) width = tabs;
		}
		col += width;
	}
}

void Gobby::Document::set_selection(const Gtk::TextIter& begin,
                                    const Gtk::TextIter& end)
{
        get_buffer()->select_range(begin, end);
        scroll_to(get_buffer()->get_insert(), 0.1);
}

Glib::ustring Gobby::Document::get_selection_text() const
{
	Gtk::TextIter start, end;
	get_buffer()->get_selection_bounds(start, end);
	return start.get_slice(end);
}

const Glib::ustring& Gobby::Document::get_title() const
{
	return m_title;
}

const Glib::ustring& Gobby::Document::get_path() const
{
	return m_path;
}

bool Gobby::Document::get_modified() const
{
	return get_buffer()->get_modified();
}

void Gobby::Document::set_path(const Glib::ustring& new_path)
{
	m_path = new_path;
}

Glib::RefPtr<Gtk::SourceLanguage> Gobby::Document::get_language() const
{
	return get_buffer()->get_language();
}

void Gobby::Document::set_language(
	const Glib::RefPtr<Gtk::SourceLanguage>& language
)
{
	get_buffer()->set_language(language);
	m_signal_language_changed.emit();
}

const Gobby::Preferences& Gobby::Document::get_preferences() const
{
	return m_preferences;
}

void Gobby::Document::set_preferences(const Preferences& preferences)
{
	m_preferences = preferences;
	apply_preferences();
}

Glib::ustring Gobby::Document::get_content()
{
	return get_buffer()->get_text();
}

void Gobby::Document::obby_user_join(const obby::user& user)
{
	update_tag_colour(user);
}

void Gobby::Document::obby_user_part(const obby::user& user)
{
}

void Gobby::Document::obby_user_colour(const obby::user& user)
{
	update_tag_colour(user);
}

void Gobby::Document::on_obby_insert_before(obby::position pos,
                                            const std::string& text,
                                            const obby::user* author)
{
	if(m_editing) return;
	m_editing = true;

	// Get textbuffer
	Glib::RefPtr<Gtk::TextBuffer> buffer = get_buffer();

	// Translate position to row/column
	unsigned int row, col;
	m_doc.get_content().position_to_coord(pos, row, col);

	// Insert text
	Gtk::TextBuffer::iterator end = buffer->insert(
		buffer->get_iter_at_line_index(row, col), text
	);

	// Get iterator to beginning of inserted text
	Gtk::TextBuffer::iterator begin = end;
	begin.backward_chars(Glib::ustring(text).length() );

	// Left gravity cursor ona remote inserts
	if(end == buffer->get_insert()->get_iter() )
		buffer->move_mark(buffer->get_insert(), begin);

	if(end == buffer->get_selection_bound()->get_iter() )
		buffer->move_mark(buffer->get_selection_bound(), begin);

	// Colourize new text with that user's color
	update_user_colour(begin, end, author);

	m_editing = false;
}

void Gobby::Document::on_obby_insert_after(obby::position pos,
                                           const std::string& text,
                                           const obby::user* author)
{
	if(m_editing) return;

	m_signal_cursor_moved.emit();
	m_signal_content_changed.emit();
}

void Gobby::Document::on_obby_delete_before(obby::position pos,
                                            obby::position len,
                                            const obby::user* author)
{
	if(m_editing) return;
	m_editing = true;

	Glib::RefPtr<Gtk::TextBuffer> buffer = get_buffer();
	unsigned int brow, bcol, erow, ecol;

	m_doc.get_content().position_to_coord(pos, brow, bcol);
	m_doc.get_content().position_to_coord(pos + len, erow, ecol);

	buffer->erase(
		buffer->get_iter_at_line_index(brow, bcol),
		buffer->get_iter_at_line_index(erow, ecol)
	);

	m_editing = false;
}

void Gobby::Document::on_obby_delete_after(obby::position pos,
                                           obby::position len,
                                           const obby::user* author)
{
	if(m_editing) return;

	m_signal_cursor_moved.emit();
	m_signal_content_changed.emit();
}

void Gobby::Document::on_insert_before(const Gtk::TextBuffer::iterator& begin,
                                       const Glib::ustring& text,
                                       int bytes)
{
	if(m_editing) return;
	m_editing = true;

	m_doc.insert(
		m_doc.get_content().coord_to_position(
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
		m_doc.get_content().coord_to_position(
			begin.get_line(),
			begin.get_line_index()
		),
		m_doc.get_content().coord_to_position(
			end.get_line(),
			end.get_line_index()
		) - m_doc.get_content().coord_to_position(
			begin.get_line(),
			begin.get_line_index()
		)
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
		// Find the user that has written this text
		const obby::user& user = m_doc.get_buffer().get_self();

		// Find start position of new text
		Gtk::TextBuffer::iterator pos = end;
		pos.backward_chars(text.length() );

		// Update user colour. Set m_editing to true because this
		// colour update came from an editing operation. See
		// on_tag_apply below for more information on why this is
		// necessary.
		m_editing = true;
		update_user_colour(pos, end, &user);
		m_editing = false;

		// Cursor position has changed
		m_signal_cursor_moved.emit();
		// Document content has changed
		m_signal_content_changed.emit();
	}
}

void Gobby::Document::on_erase_after(const Gtk::TextBuffer::iterator& begin,
                                     const Gtk::TextBuffer::iterator& end)
{
	if(!m_editing)
	{
		// Cursor position may have changed
		m_signal_cursor_moved.emit();
		// Document content has changed
		m_signal_content_changed.emit();
	}
}

void Gobby::Document::on_apply_tag_after(const Glib::RefPtr<Gtk::TextTag>& tag,
                                         const Gtk::TextBuffer::iterator& begin,
                                         const Gtk::TextBuffer::iterator& end)
{
	Glib::ustring tag_name = tag->property_name();
	if(!m_editing && tag_name.compare(0, 10, "gobby_user") == 0)
	{
		// Not editing, but user tag is inserted. Not good. May result
		// from a copy+paste operation where tags where copied. Refresh
		// given range.
		unsigned int num_line = begin.get_line();
		unsigned int num_col = begin.get_line_index();

		// Find author of the text
		const obby::line& line = m_doc.get_content().get_line(num_line);

		obby::line::author_iterator iter = line.author_begin();
		for(iter; iter != line.author_end(); ++ iter)
			if(iter->position > num_col)
				break;
		--iter;

		// Refresh.
		m_editing = true;
		update_user_colour(begin, end, iter->author);
		m_editing = false;
	}
}

void Gobby::Document::on_mark_set(
	const Gtk::TextBuffer::iterator& location,
	const Glib::RefPtr<Gtk::TextBuffer::Mark>& mark
)
{
	// Mark was deleted or something
	if(!mark) return;

	// Insert mark changed position: Cursor position change
	if(mark == get_buffer()->get_insert() )
		m_signal_cursor_moved.emit();
}

void Gobby::Document::update_user_colour(const Gtk::TextBuffer::iterator& begin,
                                         const Gtk::TextBuffer::iterator& end,
                                         const obby::user* user)
{
	// Remove other user tags in that range
	Glib::RefPtr<Gtk::TextBuffer> buffer = get_buffer();
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

	// If user is NULL (the server wrote this text), we have not to apply
	// any user tags.
	if(user == NULL) return;

	// Insert new user tag to the given range
	Glib::RefPtr<Gtk::TextTag> tag =
		tag_table->lookup("gobby_user_" + user->get_name() );

	// Make sure the tag exists
	if(!tag)
		throw std::logic_error("Gobby::Document::update_user_colour");

	buffer->apply_tag(tag, begin, end);
}

void Gobby::Document::apply_preferences()
{
	// Editor
	set_tabs_width(m_preferences.editor.tab_width);
	set_insert_spaces_instead_of_tabs(m_preferences.editor.tab_spaces);
	set_auto_indent(m_preferences.editor.indentation_auto);
	set_smart_home_end(m_preferences.editor.homeend_smart);

	// View
	// Check preference for wrapped text
	if(m_preferences.view.wrap_text)
	{
		if(m_preferences.view.wrap_words)
			set_wrap_mode(Gtk::WRAP_CHAR);
		else
			set_wrap_mode(Gtk::WRAP_WORD);
	}
	else
	{
		set_wrap_mode(Gtk::WRAP_NONE);
	}

	set_show_line_numbers(m_preferences.view.linenum_display);
	set_highlight_current_line(m_preferences.view.curline_highlight);
	set_show_margin(m_preferences.view.margin_display);
	set_margin(m_preferences.view.margin_pos);
	get_buffer()->set_check_brackets(m_preferences.view.bracket_highlight);

	// Cursor position may have changed because tab width may have changed
	m_signal_cursor_moved.emit();
}

void
Gobby::Document::on_remove_user_colour(Glib::RefPtr<Gtk::TextBuffer::Tag> tag,
                                       const Gtk::TextBuffer::iterator& begin,
				       const Gtk::TextBuffer::iterator& end)
{
	// Remove tag if it is a user color tag.
	Glib::ustring tag_name = tag->property_name();
	if(tag_name.compare(0, 10, "gobby_user") == 0)
		get_buffer()->remove_tag(tag, begin, end);
}

void Gobby::Document::update_tag_colour(const obby::user& user)
{
	// Build tag name for this user
	Glib::ustring tag_name = "gobby_user_" + user.get_name();

	// Find already existing tag
	Glib::RefPtr<Gtk::TextBuffer> buffer = get_buffer();
	Glib::RefPtr<Gtk::TextBuffer::TagTable> tag_table =
		buffer->get_tag_table();
	Glib::RefPtr<Gtk::TextBuffer::Tag> tag = tag_table->lookup(tag_name);

	// Create new tag, if it doesn't exist
	if(!tag)
		tag = buffer->create_tag(tag_name);

	// Build color
	Gdk::Color color;
	color.set_red(user.get_colour().get_red() * 65535 / 255);
	color.set_green(user.get_colour().get_green() * 65535 / 255);
	color.set_blue(user.get_colour().get_blue() * 65535 / 255);

	// Set/Update color
	tag->property_background_gdk() = color;
}
