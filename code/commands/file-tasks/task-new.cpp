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

#include "commands/file-tasks/task-new.hpp"
#include "util/i18n.hpp"

Gobby::TaskNew::TaskNew(FileCommands& file_commands):
	Task(file_commands)
{
}

Gobby::TaskNew::~TaskNew()
{
	get_document_location_dialog().hide();
}

void Gobby::TaskNew::run()
{
	DocumentLocationDialog& dialog = get_document_location_dialog();

	dialog.signal_response().connect(
		sigc::mem_fun(*this, &TaskNew::on_response));
	dialog.set_document_name(_("New Document"));
	dialog.set_single_document_mode();
	dialog.present();
}

void Gobby::TaskNew::on_response(int response_id)
{
	if(response_id == Gtk::RESPONSE_ACCEPT)
	{
		DocumentLocationDialog& dialog =
			get_document_location_dialog();
			
		InfBrowserIter iter;
		InfBrowser* browser = dialog.get_selected_directory(&iter);
		g_assert(browser != NULL);

		get_operations().create_document(browser, &iter,
		                                 dialog.get_document_name());
	}

	finish();
}
