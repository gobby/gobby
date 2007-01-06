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

#include "togglewindow.hpp"

Gobby::ToggleWindow::ToggleWindow(Gtk::Window& parent,
                                  const Glib::RefPtr<Gtk::ToggleAction>& action,
			          const Preferences& preferences,
				  Config::ParentEntry& config_entry):
	ToolWindow(parent),
	m_action(action),
	m_preferences(preferences),
	m_config_entry(config_entry)
{
	action->signal_activate().connect(
		sigc::mem_fun(*this, &ToggleWindow::on_activate) );

	if(preferences.appearance.remember)
	{
		// Read the ToggleWindow's last position from config
		const int x = config_entry.get_value<int>("x", 0);
		const int y = config_entry.get_value<int>("y", 0);
		const int w = config_entry.get_value<int>("width", 0);
		const int h = config_entry.get_value<int>("height", 0);
		bool first_run = (x == 0 && y == 0 && w == 0 && h == 0);

		if(!first_run)
		{
			move(x, y);
			resize(w, h);
		}

		if(config_entry.get_value<bool>("visible", false) )
		{
			// Show widget after parent has been shown
			parent.signal_show().connect(
				sigc::mem_fun(*this, &Gtk::Widget::show) );
		}
	}
}

Gobby::ToggleWindow::~ToggleWindow()
{
	if(m_preferences.appearance.remember)
	{
		int x, y, w, h;
		get_position(x, y);
		get_size(w, h);

		m_config_entry.set_value("x", x);
		m_config_entry.set_value("y", y);
		m_config_entry.set_value("width", w);
		m_config_entry.set_value("height", h);
		m_config_entry.set_value("visible", is_visible() );
	}
}

void Gobby::ToggleWindow::on_activate()
{
	if(m_action->get_active() )
		show();
	else
		hide();
}

void Gobby::ToggleWindow::on_show()
{
	m_action->set_active(true);
	ToolWindow::on_show();
}

void Gobby::ToggleWindow::on_hide()
{
	m_action->set_active(false);
	ToolWindow::on_hide();
}
