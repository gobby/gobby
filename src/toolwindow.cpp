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
                              const Glib::RefPtr<Gtk::ToggleAction>& action):
	Gtk::Window(Gtk::WINDOW_TOPLEVEL), m_action(action)
{
	set_transient_for(parent);
	set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
	set_title(title);

	set_skip_pager_hint(true);
	set_skip_taskbar_hint(true);

	action->signal_activate().connect(
		sigc::mem_fun(*this, &ToolWindow::on_activate) );

	set_border_width(0);
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
