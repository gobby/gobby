/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2010 Armin Burgmeier <armin@arbur.net>
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

#include "commands/file-tasks/task-open-location.hpp"

#include <gtkmm/stock.h>

Gobby::TaskOpenLocation::TaskOpenLocation(FileCommands& file_commands):
	Task(file_commands), m_location_dialog(get_parent())
{
}

void Gobby::TaskOpenLocation::run()
{
	m_location_dialog.signal_response().connect(
		sigc::mem_fun( *this, &TaskOpenLocation::on_response));

	m_location_dialog.add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CLOSE);
	m_location_dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_ACCEPT);

	m_location_dialog.present();
}

void Gobby::TaskOpenLocation::on_response(int response_id)
{
	if(response_id == Gtk::RESPONSE_ACCEPT)
	{
		std::string uri = m_location_dialog.get_uri();
		Glib::RefPtr<Gio::File> file = Gio::File::create_for_uri(uri);

		m_location_dialog.hide();

		m_open_task.reset(new TaskOpen(m_file_commands, file));
		m_open_task->signal_finished().connect(
			sigc::mem_fun(*this, &TaskOpenLocation::finish));
		m_open_task->run();
	}
	else
	{
		finish();
	}
}
