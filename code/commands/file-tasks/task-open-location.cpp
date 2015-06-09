/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2015 Armin Burgmeier <armin@arbur.net>
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

#include "commands/file-tasks/task-open-location.hpp"
#include "util/i18n.hpp"

Gobby::TaskOpenLocation::TaskOpenLocation(FileCommands& file_commands):
	Task(file_commands)
{
}

void Gobby::TaskOpenLocation::run()
{
	m_location_dialog = OpenLocationDialog::create(get_parent());
	m_location_dialog->signal_response().connect(
		sigc::mem_fun( *this, &TaskOpenLocation::on_response));

	m_location_dialog->add_button(_("_Close"), Gtk::RESPONSE_CLOSE);
	m_location_dialog->add_button(_("_Open"), Gtk::RESPONSE_ACCEPT);

	m_location_dialog->present();
}

void Gobby::TaskOpenLocation::on_response(int response_id)
{
	if(response_id == Gtk::RESPONSE_ACCEPT)
	{
		std::string uri = m_location_dialog->get_uri();
		Glib::RefPtr<Gio::File> file = Gio::File::create_for_uri(uri);

		m_location_dialog.reset(NULL);

		m_open_task.reset(new TaskOpen(m_file_commands, file));
		m_open_task->signal_finished().connect(
			sigc::mem_fun(*this, &TaskOpenLocation::finish));
		m_open_task->run();
	}
	else
	{
		finish(); // deletes this
	}
}
