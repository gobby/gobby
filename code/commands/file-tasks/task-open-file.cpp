/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2014 Armin Burgmeier <armin@arbur.net>
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
 * Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include "commands/file-tasks/task-open-file.hpp"
#include "util/i18n.hpp"

Gobby::TaskOpenFile::TaskOpenFile(FileCommands& file_commands):
	Task(file_commands), m_file_dialog(get_file_chooser(), get_parent(),
		_("Choose a text file to open"),
		Gtk::FILE_CHOOSER_ACTION_OPEN)
{
	m_file_dialog.set_select_multiple(true);
}

void Gobby::TaskOpenFile::run()
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
		Glib::SListHandle<Glib::ustring> uris = m_file_dialog.get_uris();
		
		g_assert(uris.size() >= 1);
		
		// Single file selected
		if (uris.size() == 1)
		{
			Glib::RefPtr<Gio::File> file = Gio::File::create_for_uri(*uris.begin());
			m_open_task.reset(new TaskOpen(m_file_commands, file));
			m_open_task->signal_finished().connect(
				sigc::mem_fun(*this, &TaskOpenFile::finish));
			m_open_task->run();
		}
		else // Multiple files selected
		{
			TaskOpenMultiple *task = new TaskOpenMultiple(m_file_commands);
			
			for(Glib::SListHandle<Glib::ustring>::iterator i = uris.begin(); i != uris.end(); ++i)
				task->add_file(*i);
			
			m_open_taskm.reset(task);
			m_open_taskm->signal_finished().connect(
				sigc::mem_fun(*this, &TaskOpenFile::finish));
			m_open_taskm->run();
		}
	}
	else
	{
		finish();
	}
}
