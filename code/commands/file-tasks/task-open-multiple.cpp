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

#include "commands/file-tasks/task-open-multiple.hpp"
#include "operations/operation-open-multiple.hpp"

Gobby::TaskOpenMultiple::TaskOpenMultiple(FileCommands& file_commands,
                                          const file_list& files):
	Task(file_commands), m_files(files)
{
}

Gobby::TaskOpenMultiple::~TaskOpenMultiple()
{
	get_document_location_dialog().hide();
}

void Gobby::TaskOpenMultiple::run()
{
	DocumentLocationDialog& dialog = get_document_location_dialog();
	dialog.signal_response().connect(sigc::mem_fun(
		*this, &TaskOpenMultiple::on_location_response));

	dialog.set_multiple_document_mode();
	dialog.present();
}

void Gobby::TaskOpenMultiple::on_location_response(int response_id)
{
	if(response_id == Gtk::RESPONSE_ACCEPT)
	{
		DocumentLocationDialog& dialog =
			get_document_location_dialog();

		InfBrowserIter iter;
		InfBrowser* browser = dialog.get_selected_directory(&iter);
		g_assert(browser != NULL);

		OperationOpenMultiple* operation =
			get_operations().create_documents(
				browser, &iter,
				get_preferences(), m_files);
	}

	finish();
}
