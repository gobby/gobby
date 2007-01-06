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

#include <ctime>
#include <obby/format_string.hpp>
#include "logview.hpp"

Gobby::LogView::LogView()
 : Gtk::TextView()
{
	m_end_mark = get_buffer()->create_mark(
		"end_mark", get_buffer()->end(), false
	);

	set_editable(false);
	set_wrap_mode(Gtk::WRAP_WORD_CHAR);
}

Gobby::LogView::~LogView()
{
}

void Gobby::LogView::clear()
{
	get_buffer()->set_text("");
}

void Gobby::LogView::log(const Glib::ustring& text,
                         const Glib::ustring& color)
{
	log(text, color, std::time(NULL) );
}

void Gobby::LogView::log(const Glib::ustring& text,
                         const Glib::ustring& color,
                         std::time_t timestamp)
{
	Glib::RefPtr<Gtk::TextBuffer> buffer = get_buffer();
	Glib::RefPtr<Gtk::TextTag> tag = buffer->get_tag_table()->lookup(color);

	Glib::ustring ins_text = text;
	if(ins_text[ins_text.length() - 1] != '\n') ins_text += "\n";

	const char* formatter = "%X";
	std::time_t cur_time_t = std::time(NULL);
	std::tm cur_time_tm = *std::localtime(&cur_time_t);
	std::tm given_time_tm = *std::localtime(&timestamp);

	// Show date if the text was not logged today
	if(cur_time_tm.tm_yday != given_time_tm.tm_yday ||
	   cur_time_tm.tm_year != given_time_tm.tm_year)
	{
		formatter = "%x %X";
	}

	char buf[0x7f];
	std::strftime(buf, 0x7f, formatter, &given_time_tm);
	obby::format_string str("[%0%] %1%");
	str << buf << ins_text;

	if(!tag)
	{
		tag = Gtk::TextTag::create();
		tag->property_foreground() = color;
		buffer->get_tag_table()->add(tag);
	}

	buffer->insert_with_tag(buffer->end(), str.str(), tag);
	scroll_to_mark(m_end_mark, 0.0f);
}

