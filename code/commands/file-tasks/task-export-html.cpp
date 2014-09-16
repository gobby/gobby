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
			*m_view, m_file_dialog.get_file());
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
