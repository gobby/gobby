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

// TODO: Merge with TaskSave, or share code in a common base class

#include "commands/file-tasks/task-export-html.hpp"
#include "util/i18n.hpp"

Gobby::TaskExportHtml::TaskExportHtml(FileCommands& file_commands,
                                      TextSessionView& view):
	Task(file_commands),
	m_file_dialog(get_file_chooser(), get_parent(),
		Glib::ustring::compose(
			_("Choose a location to export document \"%1\" to"),
			view.get_title()),
		Gtk::FILE_CHOOSER_ACTION_SAVE),
	m_view(&view), m_running(false)
{
	get_folder().signal_document_removed().connect(
		sigc::mem_fun(*this, &TaskExportHtml::on_document_removed));
}

void Gobby::TaskExportHtml::run()
{
	// m_document will be set to NULL if it has been removed before run
	// was called.
	if(!m_view)
	{
		finish();
		return;
	}

	m_running = true;

	m_file_dialog.signal_response().connect(sigc::mem_fun(
		*this, &TaskExportHtml::on_response));

	m_file_dialog.set_current_name(m_view->get_title() + ".xhtml");
	m_file_dialog.present();
}

void Gobby::TaskExportHtml::on_response(int response_id)
{
	if(response_id == Gtk::RESPONSE_ACCEPT)
	{
		get_operations().export_html(
			*m_view, m_file_dialog.get_uri());
	}

	finish();
}

void Gobby::TaskExportHtml::on_document_removed(SessionView& view)
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
