/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2011 Armin Burgmeier <armin@arbur.net>
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

#include "commands/file-tasks/task-open-multiple.hpp"
#include "operations/operation-open-multiple.hpp"

Gobby::TaskOpenMultiple::TaskOpenMultiple(FileCommands& file_commands):
	Task(file_commands)
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

void Gobby::TaskOpenMultiple::add_file(const Glib::ustring& uri)
{
	m_uris.push_back(uri);
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
				get_preferences(), m_uris.size());

		for(uri_list::const_iterator iter = m_uris.begin();
		    iter != m_uris.end(); ++iter)
		{
			operation->add_uri(*iter, NULL, NULL);
		}
	}

	finish();
}
