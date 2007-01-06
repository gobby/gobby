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

#include "toolwindow.hpp"

Gobby::ToolWindow::ToolWindow(Gtk::Window& parent,
                              const Glib::ustring& title,
                              const Glib::RefPtr<Gtk::ToggleAction>& action,
			      Config& config,
			      const Glib::ustring& config_key):
	Gtk::Window(Gtk::WINDOW_TOPLEVEL),
	m_action(action),
	m_config(config),
	m_config_key(config_key)
{
	set_transient_for(parent);
	set_title(title);

	set_skip_pager_hint(true);
	set_skip_taskbar_hint(true);

	action->signal_activate().connect(
		sigc::mem_fun(*this, &ToolWindow::on_activate) );

	set_border_width(0);

	if(config["appearance"]["windows"]["remember"].get<bool>(true) )
	{
		// Read the ToolWindow's last position from the configuration,
		// relative to the parent window.
		const int x = config[config_key]["x"].get<int>(0);
		const int y = config[config_key]["y"].get<int>(0);
		const int w = config[config_key]["width"].get<int>(0);
		const int h = config[config_key]["height"].get<int>(0);
		const bool visible = config[config_key]["visible"].get<bool>(false);
		if((x != 0) || (y != 0))
		{
			move(x, y);
			resize(w, h);
		}
	}
}

Gobby::ToolWindow::~ToolWindow()
{
	if(m_config["appearance"]["windows"]["remember"].get<bool>(true) )
	{
		int x, y, w, h;
		get_position(x, y);
		get_size(w, h);
		m_config[m_config_key]["x"].set(x);
		m_config[m_config_key]["y"].set(y);
		m_config[m_config_key]["width"].set(w);
		m_config[m_config_key]["height"].set(h);
		m_config[m_config_key]["visible"].set(is_visible() );
	}
}

void Gobby::ToolWindow::on_activate()
{
	if(m_action->get_active() )
		show();
	else
		hide();
}

void Gobby::ToolWindow::on_show()
{
	m_action->set_active(true);
	Gtk::Window::on_show();
}

void Gobby::ToolWindow::on_hide()
{
	m_action->set_active(false);
	Gtk::Window::on_hide();
}

