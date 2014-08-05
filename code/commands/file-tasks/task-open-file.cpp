/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2014 Armin Burgmeier <armin@arbur.net>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
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
		
		if (uris.size() == 1)
		{
			Glib::RefPtr<Gio::File> file = Gio::File::create_for_uri(*uris.begin());
			m_open_task.reset(new TaskOpen(m_file_commands, file));
			m_open_task->signal_finished().connect(
				sigc::mem_fun(*this, &TaskOpenFile::finish));
			m_open_task->run();
		}
		else
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
