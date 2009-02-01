/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008, 2009 Armin Burgmeier <armin@arbur.net>
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

#include "commands/file-tasks/task-open-file.hpp"
#include "util/i18n.hpp"

Gobby::TaskOpenFile::TaskOpenFile(FileCommands& file_commands):
	Task(file_commands), m_file_dialog(get_file_chooser(), get_parent(),
		_("Choose a text file to open"),
		Gtk::FILE_CHOOSER_ACTION_OPEN)
{
	m_file_dialog.signal_response().connect(sigc::mem_fun(
		*this, &TaskOpenFile::on_file_response));

	m_file_dialog.present();
}

void Gobby::TaskOpenFile::on_file_response(int response_id)
{
	if(response_id == Gtk::RESPONSE_ACCEPT)
	{
		m_file_dialog.hide();
		// TODO: Handle multiple selection
		Glib::ustring uri = m_file_dialog.get_uri();
		Glib::RefPtr<Gio::File> file =
			Gio::File::create_for_uri(uri);

		m_open_task.reset(new TaskOpen(m_file_commands, file));
		m_open_task->signal_finished().connect(
			sigc::mem_fun(*this, &TaskOpenFile::finish));
	}
	else
	{
		finish();
	}
}
