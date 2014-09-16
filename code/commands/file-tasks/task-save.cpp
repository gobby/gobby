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
			*m_view, m_file_dialog.get_file(),
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
