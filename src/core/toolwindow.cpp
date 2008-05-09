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

#include "features.hpp"
#include "core/toolwindow.hpp"

#include <gdk/gdkkeysyms.h>

Gobby::ToolWindow::ToolWindow(Gtk::Window& parent):
	Gtk::Window(Gtk::WINDOW_TOPLEVEL),
	m_x(0), m_y(0), m_w(0), m_h(0)
{
	set_type_hint(Gdk::WINDOW_TYPE_HINT_DIALOG);
	set_transient_for(parent);
	set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

// GTK+ does not remember the position of toolwindows when
// the parent window has been moved or resized - workaround
void Gobby::ToolWindow::on_show()
{
	Gtk::Window::on_show();

	if(m_x == 0 && m_y == 0 && m_w == 0 && m_h == 0)
		return;

	move(m_x, m_y);
	resize(m_w, m_h);
}

void Gobby::ToolWindow::on_hide()
{
	get_position(m_x, m_y);
	get_size(m_w, m_h);

	Gtk::Window::on_hide();
}

bool Gobby::ToolWindow::on_key_press_event(GdkEventKey* event)
{
	if(event->keyval == GDK_Escape)
	{
		hide();
		return true;
	}

	return Gtk::Window::on_key_press_event(event);
}
