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

namespace
{
	Gdk::Color user_color(const obby::user& user)
	{
		const obby::colour& col = user.get_colour();
		Gdk::Color gdk_col;

		gdk_col.set_red(col.get_red() * 0xffff / 0xff);
		gdk_col.set_green(col.get_green() * 0xffff / 0xff);
		gdk_col.set_blue(col.get_blue() * 0xffff / 0xff);

		return gdk_col;
	}

	void forward_bytes(Gtk::TextIter& iter, std::size_t bytes)
	{
		while(bytes > 0 && !iter.is_end() )
		{
			std::size_t remaining_bytes =
				iter.get_bytes_in_line() -
				iter.get_line_index();

			if(remaining_bytes >= bytes)
			{
				iter.set_line_index(
					iter.get_line_index() +
					bytes
				);

				remaining_bytes = bytes;
			}
			else
			{
				iter.forward_line();
			}

			bytes -= remaining_bytes;
		}

		if(bytes > 0 && iter.is_end() )
		{
			throw std::logic_error(
				"document.cpp:forward_bytes:\n"
				"Bytes to forward exceed buffer size"
			);
		}
	}

	std::size_t diff_bytes(const Gtk::TextIter& begin,
	                       const Gtk::TextIter& end)
	{
		std::size_t bytes = 0;
		Gtk::TextIter iter = begin;

		while(iter != end)
		{
			std::size_t line_bytes;
			if(iter.get_line() == end.get_line() )
			{
				line_bytes =
					end.get_line_index() -
					iter.get_line_index();

				iter = end;
			}
			else
			{
				line_bytes =
					iter.get_bytes_in_line() -
					iter.get_line_index();

				iter.forward_line();
			}

			bytes += line_bytes;
		}

		return bytes;
	}

	class editor: private net6::non_copyable
	{
	public:
		editor(bool& edit_var):
			m_edit_var(edit_var)
		{
			if(m_edit_var == true)
			{
				throw std::logic_error(
					"document.cpp:editor::editor:\n"
					"Edit var is already true"
				);
			}

			m_edit_var = true;
		}

		~editor()
		{
			m_edit_var = false;
		}

	private:
		bool& m_edit_var;
	};
}

Gobby::Document::chunk_iterator::chunk_iterator(const Document& doc,
                                                const Gtk::TextIter& begin):
	m_doc(doc), m_author(doc.author_at_iter(begin) ), m_iter_begin(begin),
	m_iter_end(begin)
{
	proceed_end();
}

const obby::user* Gobby::Document::chunk_iterator::get_author() const
{
	return m_author;
}

std::string Gobby::Document::chunk_iterator::get_text() const
{
	return m_iter_begin.get_slice(m_iter_end).raw();
}

Gobby::Document::chunk_iterator& Gobby::Document::chunk_iterator::operator++()
{
	m_author = m_next_author;
	m_iter_begin = m_iter_end;

	proceed_end();
	return *this;
}

Gobby::Document::chunk_iterator Gobby::Document::chunk_iterator::operator++(int)
{
	chunk_iterator temp(*this);
	operator++();
	return temp;
}

bool
Gobby::Document::chunk_iterator::operator==(const chunk_iterator& other) const
{
	return m_iter_begin == other.m_iter_begin;
}

bool
Gobby::Document::chunk_iterator::operator!=(const chunk_iterator& other) const
{
	return m_iter_begin != other.m_iter_begin;
}

void Gobby::Document::chunk_iterator::proceed_end()
{
	//m_author = m_next_author;
	m_next_author = m_doc.forward_chunk(m_iter_end);
}

Gobby::Document::template_type::template_type():
	m_buffer(NULL)
{
}

Gobby::Document::template_type::
	template_type(const buffer_type& buffer):
	m_buffer(&buffer)
{
}

const Gobby::Document::template_type::buffer_type&
Gobby::Document::template_type::get_buffer() const
{
	if(m_buffer == NULL)
	{
		throw std::logic_error(
			"Gobby::Document::template_type::get_buffer:\n"
			"Invalid template, no buffer present"
		);
	}

	return *m_buffer;
}

Gobby::Document::Document(const template_type& tmpl):
	m_self(tmpl.get_buffer().get_self() ),
	m_editing(false), m_buffer(Gtk::SourceBuffer::create() )
{
	const template_type::buffer_type& buf = tmpl.get_buffer();
	const obby::user_table& table = buf.get_user_table();

	for(obby::user_table::iterator iter =
		table.begin(obby::user::flags::NONE, obby::user::flags::NONE);
	    iter != table.end(obby::user::flags::NONE, obby::user::flags::NONE);
	    ++ iter)
	{
		on_user_join(*iter);
	}

	m_buffer->signal_insert().connect(
		sigc::hide(sigc::mem_fun(*this, &Document::on_insert_before)),
		false
	);

	m_buffer->signal_insert().connect(
		sigc::hide(sigc::mem_fun(*this, &Document::on_insert_after)),
		true
	);

	m_buffer->signal_erase().connect(
		sigc::mem_fun(*this, &Document::on_erase_before),
		false
	);

	m_buffer->signal_apply_tag().connect(
		sigc::mem_fun(*this, &Document::on_apply_tag_before),
		false
	);

	m_buffer->begin_not_undoable_action();

	// TODO: Connect to user table's signal handler - as soon as it
	// has some...
	buf.user_join_event().connect(
		sigc::mem_fun(*this, &Document::on_user_join) );
	buf.user_colour_event().connect(
		sigc::mem_fun(*this, &Document::on_user_color) );
}

bool Gobby::Document::empty() const
{
	return m_buffer->begin() == m_buffer->end();
}

obby::position Gobby::Document::size() const
{
	obby::position cur_size = 0;

	for(Gtk::TextIter iter = m_buffer->begin();
	    iter != m_buffer->end();
	    iter.forward_line() )
	{
		cur_size += iter.get_bytes_in_line();
	}

	return cur_size;
}

obby::text
Gobby::Document::get_slice(obby::position from, obby::position len) const
{
	obby::text result;

	Gtk::TextIter pos = get_iter(from);
	Gtk::TextIter prev = pos;

	// Initial author
	const obby::user* author = author_at_iter(pos);
	Glib::RefPtr<const Gtk::TextTag> any_tag(NULL);

	while( (len > 0 || len == obby::text::npos) && pos != m_buffer->end())
	{
		const obby::user* new_author = forward_chunk(pos);
		obby::position diff = diff_bytes(prev, pos);

		if(len != obby::text::npos && diff > len)
		{
			pos = prev;
			diff = len;
			forward_bytes(pos, diff);
		}

		result.append(prev.get_slice(pos), author);

		if(len != obby::text::npos)
			len -= diff;

		prev = pos;
		author = new_author;
	}

	if(len != obby::text::npos && pos == m_buffer->end() && len > 0)
	{
		throw std::logic_error(
			"Gobby::Document::get_slice:\n"
			"len exceeds size of buffer"
		);
	}

	return result;
}

Gobby::Document::chunk_iterator Gobby::Document::chunk_begin() const
{
	return chunk_iterator(*this, m_buffer->begin() );
}

Gobby::Document::chunk_iterator Gobby::Document::chunk_end() const
{
	return chunk_iterator(*this, m_buffer->end() );
}

void Gobby::Document::clear()
{
	m_buffer->set_text("");
}

void Gobby::Document::insert(obby::position pos,
                             const obby::text& str)
{
	if(m_editing) return;
	editor edit(m_editing);

	Gtk::TextIter iter = get_iter(pos);
	insert_impl(iter, str);
}

void Gobby::Document::insert(obby::position pos,
                             const std::string& str,
                             const obby::user* author)
{
	if(m_editing) return;
	editor edit(m_editing);

	Gtk::TextIter iter = get_iter(pos);
	insert_impl(iter, str, author);
}

void Gobby::Document::erase(obby::position pos, obby::position len)
{
	if(m_editing) return;
	editor edit(m_editing);

	Gtk::TextIter begin = get_iter(pos);
	Gtk::TextIter end = begin;

	if(len == obby::text::npos)
		end = m_buffer->end();
	else
		forward_bytes(end, len);

	m_buffer->erase(begin, end);
}

void Gobby::Document::append(const obby::text& str)
{
	if(m_editing) return;
	editor edit(m_editing);

	insert_impl(m_buffer->end(), str);
}

void Gobby::Document::append(const std::string& str,
                             const obby::user* author)
{
	if(m_editing) return;
	editor edit(m_editing);

	insert_impl(m_buffer->end(), str, author);
}

Glib::RefPtr<Gtk::SourceBuffer> Gobby::Document::get_buffer() const
{
	return m_buffer;
}

Gobby::Document::signal_insert_type Gobby::Document::insert_event() const
{
	return m_signal_insert;
}

Gobby::Document::signal_erase_type Gobby::Document::erase_event() const
{
	return m_signal_erase;
}

void Gobby::Document::on_user_join(const obby::user& user)
{
	map_user_type::iterator user_it = m_map_user.find(&user);

	if(user_it == m_map_user.end() )
	{
		Glib::RefPtr<Gtk::TextTag> tag = Gtk::TextTag::create();
		tag->property_background_gdk() = user_color(user);
		m_buffer->get_tag_table()->add(tag);
		tag->set_priority(0);

		m_map_user[&user] = tag;
		m_map_tag[tag] = &user;
	}
	else
	{
		// User may already be in map if he rejoins
		user_it->second->property_background_gdk() = user_color(user);
	}
}

void Gobby::Document::on_user_color(const obby::user& user)
{
	map_user_type::iterator user_it = m_map_user.find(&user);

	if(user_it == m_map_user.end() )
	{
		throw std::logic_error(
			"Gobby::Document::on_user_color:\n"
			"User is not in user tag map"
		);
	}

	user_it->second->property_background_gdk() = user_color(user);
}

void Gobby::Document::on_insert_before(const Gtk::TextIter& iter,
                                       const Glib::ustring& text)
{
	// Only local edits that are not done via insert
	if(m_editing) return;
	editor edit(m_editing);

	m_signal_insert.emit(diff_bytes(m_buffer->begin(), iter), text);
}

void Gobby::Document::on_insert_after(const Gtk::TextIter& iter,
                                      const Glib::ustring& text)
{
	if(m_editing) return;
	editor edit(m_editing);

	Gtk::TextIter begin = iter;
	begin.backward_chars(text.length() );

	tag_text(begin, iter, &m_self);
}

void Gobby::Document::on_erase_before(const Gtk::TextIter& begin,
                                      const Gtk::TextIter& end)
{
	// Only local edits that are not done via erase
	if(m_editing) return;

	editor edit(m_editing);
	m_signal_erase.emit(
		diff_bytes(m_buffer->begin(), begin),
		diff_bytes(begin, end)
	);
}

void Gobby::Document::on_apply_tag_before(const Glib::RefPtr<Gtk::TextTag>& tag,
                                          const Gtk::TextIter& begin,
                                          const Gtk::TextIter& end)
{
	if(m_map_tag.find(tag) != m_map_tag.end() && !m_editing)
		m_buffer->signal_apply_tag().emission_stop();
}

Gtk::TextIter Gobby::Document::get_iter(obby::position at) const
{
	Gtk::TextIter pos;
	for(pos = m_buffer->begin();
	    at > 0 && pos != m_buffer->end();)
	{
		obby::position new_bytes = pos.get_bytes_in_line();

		if(new_bytes > at)
		{
			pos.set_line_index(at);
			new_bytes = at;
		}
		else
		{
			pos.forward_line();
		}

		at -= new_bytes;
	}

	if(pos == m_buffer->end() && at > 0)
	{
		throw std::logic_error(
			"Gobby::Document::get_iter:\n"
			"at exceeds size of document"
		);
	}

	return pos; //m_buffer->end();
}

const obby::user*
Gobby::Document::author_in_list(const tag_list_type& list) const
{
	for(tag_list_type::const_iterator iter = list.begin();
	    iter != list.end();
	    ++ iter)
	{
		map_tag_type::const_iterator tag_it = m_map_tag.find(*iter);
		if(tag_it != m_map_tag.end() ) return tag_it->second;
	}

	return NULL;
}

const obby::user*
Gobby::Document::author_at_iter(const Gtk::TextIter& pos) const
{
	return author_in_list(pos.get_tags() );
}

bool Gobby::Document::author_toggle(const Gtk::TextIter& at,
                                    const obby::user*& to) const
{
	const obby::user* new_author = author_in_list(
		at.get_toggled_tags(true)
	);

	if(new_author == NULL)
	{
		const obby::user* old_author = author_in_list(
			at.get_toggled_tags(false)
		);

		if(old_author == NULL)
			return false;
	}

	to = new_author;
	return true;
}

const obby::user* Gobby::Document::forward_chunk(Gtk::TextIter& iter) const
{
	Glib::RefPtr<Gtk::TextTag> any_tag(NULL);

	for(iter.forward_to_tag_toggle(any_tag);
	    iter != m_buffer->end();
	    iter.forward_to_tag_toggle(any_tag))
	{
		const obby::user* new_author;
		if(!author_toggle(iter, new_author) )
			continue;

		return new_author;
	}

	return NULL;
}

Gtk::TextIter Gobby::Document::insert_impl(const Gtk::TextIter& iter,
                                           const std::string& str,
                                           const obby::user* author)
{
	Gtk::TextIter result = m_buffer->insert(iter, str);

	Gtk::TextIter begin = result;
	begin.backward_chars(g_utf8_strlen(str.c_str(), -1));

	tag_text(begin, result, author);

	// Left gravity cursor on remote insert
	if(result == m_buffer->get_insert()->get_iter() )
		m_buffer->move_mark(m_buffer->get_insert(), begin);

	if(result == m_buffer->get_selection_bound()->get_iter() )
		m_buffer->move_mark(m_buffer->get_selection_bound(), begin);

	return result;
}

Gtk::TextIter Gobby::Document::insert_impl(const Gtk::TextIter& iter,
                                           const obby::text& str)
{
	Gtk::TextIter pos = iter;

	for(obby::text::chunk_iterator chunk_it = str.chunk_begin();
	    chunk_it != str.chunk_end();
	    ++ chunk_it)
	{
		pos = insert_impl(
			pos,
			chunk_it->get_text(),
			chunk_it->get_author()
		);
	}

	return pos;
}

void Gobby::Document::tag_text(const Gtk::TextIter& begin,
                               const Gtk::TextIter& end,
                               const obby::user* with)
{
	for(map_user_type::const_iterator user_iter = m_map_user.begin();
	    user_iter != m_map_user.end();
	    ++ user_iter)
	{
		m_buffer->remove_tag(user_iter->second, begin, end);
	}

	if(with != NULL)
	{
		map_user_type::const_iterator user_it = m_map_user.find(with);

		if(user_it == m_map_user.end() )
		{
			throw std::logic_error(
				"Gobby::Document::insert_impl:\n"
				"User is not in user tag map"
			);
		}

		m_buffer->apply_tag(user_it->second, begin, end);
	}
}
