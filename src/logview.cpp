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

#include <ctime>
#include <obby/format_string.hpp>
#include "features.hpp"
#include "logview.hpp"

#ifdef WITH_GNOME
# include <libgnomevfs/gnome-vfs-utils.h>
#endif

#ifdef _WIN32
# include <windows.h>
#endif

#if defined(WITH_GNOME)
void show_url(const char* url)
{
	gnome_vfs_url_show(url);
}
#elif defined(WIN32)
void show_url(const char* url)
{
	ShellExecute(NULL, "open", url, NULL, NULL, SW_SHOWNA);
}
#elif defined(OSX)
void show_url(const char* url)
{
	Glib::spawn_command_line_async("open " + std::string(url) );
}
#endif

Gobby::LogView::LogView():
#ifdef HAVE_SHOW_URL
	Gtk::TextView(), m_default(Gdk::XTERM), m_hand(Gdk::HAND2),
	m_hovering(false)
#else
	Gtk::TextView()
#endif
{
	m_end_mark = get_buffer()->create_mark(
		"end_mark", get_buffer()->end(), false
	);

	set_editable(false);
	set_wrap_mode(Gtk::WRAP_WORD_CHAR);

	// needed for accessibility
	set_cursor_visible(true);

#ifdef HAVE_SHOW_URL
	signal_motion_notify_event().connect(
		sigc::mem_fun(*this, &LogView::on_motion_notify)
	);

	signal_event_after().connect(
		sigc::mem_fun(*this, &LogView::on_event_after)
	);

	Gdk::Color blue;
	blue.set_red(0x0000);
	blue.set_green(0x0000);
	blue.set_blue(0xffff);

	m_tag_link = Gtk::TextTag::create();
	m_tag_link->property_foreground_gdk() = blue;
	m_tag_link->property_underline() = Pango::UNDERLINE_SINGLE;
	get_buffer()->get_tag_table()->add(m_tag_link);
#endif
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
	Glib::RefPtr<Gtk::TextTag> tag;
	if(!color.empty()) buffer->get_tag_table()->lookup(color);

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
	str << buf << ins_text.raw();

	if(!tag && !color.empty())
	{
		tag = Gtk::TextTag::create();
		tag->property_foreground() = color;
		buffer->get_tag_table()->add(tag);
		tag->set_priority(0);
	}

	Gtk::TextIter end;
	if(tag)
		end = buffer->insert_with_tag(buffer->end(), str.str(), tag);
	else
		end = buffer->insert(buffer->end(), str.str());

	scroll_to(m_end_mark, 0.0f);

#ifdef HAVE_SHOW_URL
	Gtk::TextIter begin = end;
	begin.backward_chars(text.length() );

	set_url_tag(begin, end);
#endif
}

#ifdef HAVE_SHOW_URL
void Gobby::LogView::set_url_tag(const Gtk::TextIter& begin,
                                 const Gtk::TextIter& end)
{
	Gtk::TextIter pos = begin;
	Gtk::TextIter match_begin, match_end;
	Gtk::TextSearchFlags flags = Gtk::TextSearchFlags(0);

	while(pos.forward_search("http://", flags, match_begin,	match_end, end))
	{
		// Advance to next space
		pos = match_end;
		while(pos != end && !Glib::Unicode::isspace(*pos))
			++ pos;

		get_buffer()->apply_tag(m_tag_link, match_begin, pos);
	}
}

bool Gobby::LogView::on_motion_notify(GdkEventMotion* event)
{
	int buffer_x, buffer_y;

	window_to_buffer_coords(
		Gtk::TEXT_WINDOW_WIDGET,
		static_cast<int>(event->x),
		static_cast<int>(event->y),
		buffer_x,
		buffer_y
	);

	Gtk::TextIter iter;
	get_iter_at_location(iter, buffer_x, buffer_y);

	if(iter.has_tag(m_tag_link) && !m_hovering)
	{
		get_window(Gtk::TEXT_WINDOW_TEXT)->set_cursor(m_hand);
		m_hovering = true;
	}
	else if(!iter.has_tag(m_tag_link) && m_hovering)
	{
		get_window(Gtk::TEXT_WINDOW_TEXT)->set_cursor(m_default);
		m_hovering = false;
	}

	gdk_window_get_pointer(
		Gtk::Widget::get_window()->gobj(),
		NULL, NULL, NULL
	);

	return false;
}

void Gobby::LogView::on_event_after(GdkEvent* event)
{
	if(event->type != GDK_BUTTON_RELEASE) return;
	GdkEventButton* button_event = &event->button;

	if(button_event->button != 1) return;

	Glib::RefPtr<Gtk::TextBuffer> buffer = get_buffer();
	Gtk::TextIter begin, end;
	buffer->get_selection_bounds(begin, end);

	if(begin != end) return;

	int buffer_x, buffer_y;
	window_to_buffer_coords(
		Gtk::TEXT_WINDOW_WIDGET,
		static_cast<int>(button_event->x),
		static_cast<int>(button_event->y),
		buffer_x,
		buffer_y
	);

	Gtk::TextIter iter;
	get_iter_at_location(iter, buffer_x, buffer_y);
	if(!iter.has_tag(m_tag_link) ) return;

	begin = end = iter;
	begin.backward_to_tag_toggle(m_tag_link);
	end.forward_to_tag_toggle(m_tag_link);

	Glib::ustring link = begin.get_slice(end);
	show_url(link.c_str() );
}
#endif
