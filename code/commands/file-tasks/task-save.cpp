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

#include "commands/file-tasks/task-save.hpp"
#include "util/i18n.hpp"

Gobby::TaskSave::TaskSave(FileCommands& file_commands, TextSessionView& view):
	Task(file_commands),
	m_file_dialog(get_file_chooser(), get_parent(),
		Glib::ustring::compose(
			_("Choose a location to save document \"%1\" to"),
			view.get_title()),
		Gtk::FILE_CHOOSER_ACTION_SAVE),
	m_view(&view),
	m_running(false)
{
	get_folder().signal_document_removed().connect(
		sigc::mem_fun( *this, &TaskSave::on_document_removed));
}

void Gobby::TaskSave::run()
{
	// m_view will be set to NULL if it has been removed before run
	// was called.
	if(!m_view)
	{
		finish();
		return;
	}

	m_running = true;

	m_file_dialog.signal_response().connect(sigc::mem_fun(
		*this, &TaskSave::on_response));

	const DocumentInfoStorage::Info* info =
		get_document_info_storage().get_info(
			m_view->get_info_storage_key());

	if(info != NULL && !info->uri.empty())
		m_file_dialog.set_uri(info->uri);
	else
		m_file_dialog.set_current_name(m_view->get_title());

	m_file_dialog.present();
}

void Gobby::TaskSave::on_response(int response_id)
{
	if(response_id == Gtk::RESPONSE_ACCEPT)
	{
		const std::string& info_storage_key =
			m_view->get_info_storage_key();

		const DocumentInfoStorage::Info* info =
			get_document_info_storage().get_info(
				info_storage_key);

		// TODO: Get encoding from file dialog
		// TODO: Default to CRLF on Windows
		get_operations().save_document(
			*m_view, get_folder(), m_file_dialog.get_uri(),
			info ? info->encoding : "UTF-8",
			info ? info->eol_style : DocumentInfoStorage::EOL_LF);
	}

	finish();
}

void Gobby::TaskSave::on_document_removed(SessionView& view)
{
	// The document we are about to save was removed.
	if(m_view == &view)
	{
		if(m_running)
			finish();
		else
			m_view = NULL;
	}
}
