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

#include "historyentry.hpp"

Gobby::HistoryEntry::HistoryEntry()
 : m_pos(m_history.end() )
{
}

Gobby::HistoryEntry::~HistoryEntry()
{
}

void Gobby::HistoryEntry::clear_history()
{
	m_history.clear();
	m_pos = m_history.end();
}

void Gobby::HistoryEntry::on_activate()
{
	m_history.push_back(get_text() );
	m_pos = m_history.end();
	Gtk::Entry::on_activate();
}

bool Gobby::HistoryEntry::on_key_press_event(GdkEventKey* event)
{
	switch(event->keyval)
	{
	case GDK_Down:
		scroll_down();
		return true;
	case GDK_Up:
		if(m_pos != m_history.begin() )
			scroll_up();
		return true;
	default:
		return Gtk::Entry::on_key_press_event(event);
	}
}

void Gobby::HistoryEntry::scroll_down()
{
	if(m_pos != m_history.end() )
	{
		++ m_pos;
		if(m_pos != m_history.end() )
		{
			set_text(*m_pos);
			set_position(m_pos->length() );
		}
		else
		{
			set_text("");
		}
	}
	else
	{
		if(!get_text().empty() )
		{
			m_history.push_back(get_text() );
			m_pos = m_history.end();
			set_text("");
		}
	}
}

void Gobby::HistoryEntry::scroll_up()
{
	if(m_pos == m_history.end() )
	{
		if(!get_text().empty() )
		{
			m_history.push_back(get_text() );
			m_pos = m_history.end();
			-- m_pos;
		}
	}

	-- m_pos;
	set_text(*m_pos);
	set_position(m_pos->length() );
}

