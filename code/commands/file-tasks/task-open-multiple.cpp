/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008, 2009 Armin Burgmeier <armin@arbur.net>
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

// TODO: This should not be a task because the asynchronous IO operations
// should not be interrupted by tasks like "save as".
// Possibly do the should-not-abort part in an self-maintaining object?

#include "commands/file-tasks/task-open-multiple.hpp"
#include "operations/operation-open.hpp"
#include "util/i18n.hpp"

Gobby::TaskOpenMultiple::TaskOpenMultiple(FileCommands& file_commands):
	Task(file_commands), m_query_counter(0)
{
	Glib::signal_idle().connect(
		sigc::bind_return(
			sigc::mem_fun(*this, &TaskOpenMultiple::on_idle),
			false));
}

Gobby::TaskOpenMultiple::~TaskOpenMultiple()
{
	if(m_handle != get_status_bar().invalid_handle())
		get_status_bar().remove_message(m_handle);
	get_document_location_dialog().hide();
}

void Gobby::TaskOpenMultiple::add_file(const Glib::RefPtr<Gio::File>& file)
{
	try
	{
		file->query_info_async(
			sigc::bind(
				sigc::mem_fun(*this, &TaskOpenMultiple::on_query_info),
				file),
			G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME);
		++m_query_counter;
	}
	catch(const Gio::Error& ex)
	{
		error(ex.what());
	}
}

void Gobby::TaskOpenMultiple::on_idle()
{
	DocumentLocationDialog& dialog = get_document_location_dialog();
	dialog.signal_response().connect(sigc::mem_fun(
		*this, &TaskOpenMultiple::on_location_response));

	m_handle = get_status_bar().add_message(
		StatusBar::INFO,
		_("Opening multiple files..."),
		0);

	dialog.hide_document_name_entry();
	dialog.present();
}

void Gobby::TaskOpenMultiple::on_query_info(
	const Glib::RefPtr<Gio::AsyncResult>& result,
	Glib::RefPtr<Gio::File> file)
{
	--m_query_counter;

	try
	{
		Glib::RefPtr<Gio::FileInfo> info =
			file->query_info_finish(result);

		m_files.push(FileInfo(file, info->get_display_name()));

    // TODO: hide document name field in dialog
		// dialog.set_document_name(info->get_display_name());
	}
	catch(const Gio::Error& ex)
	{
		error(ex.what());
	}

	if (m_location.get() && m_query_counter == 0) {
		flush();
	}
}

void Gobby::TaskOpenMultiple::on_location_response(int response_id)
{
	if(response_id == Gtk::RESPONSE_ACCEPT)
	{
		DocumentLocationDialog& dialog =
			get_document_location_dialog();

		InfcBrowserIter iter;
		InfcBrowser* browser = dialog.get_selected_directory(&iter);
		g_assert(browser != NULL);

		m_location.reset(
			new NodeWatch(dialog.get_browser_model(), browser, &iter));
		m_location->signal_node_removed().connect(
			sigc::bind(
				sigc::mem_fun(*this, &TaskOpenMultiple::error),
				_("Destination directory was deleted while opening documents")));

		if (m_query_counter == 0)
			flush();
	}
	else
	{
		finish();
	}
}

void Gobby::TaskOpenMultiple::flush()
{
	if (m_files.empty()) {
		finish();
		return;
	}

	InfcBrowserIter iter = m_location->get_browser_iter();
	OperationOpen* op = get_operations().create_document(
		m_location->get_browser(), &iter, m_files.front().name,
		get_preferences(), m_files.front().file->get_uri(), NULL);
	m_files.pop();

	op->signal_finished().connect(
		sigc::hide(sigc::mem_fun(*this, &TaskOpenMultiple::flush)));
}

void Gobby::TaskOpenMultiple::error(const Glib::ustring& message)
{
	get_status_bar().add_message(StatusBar::ERROR,
		Glib::ustring::compose(
			_("Failed to open multiple documents: %1"),
			message), 5);

	finish();
}
