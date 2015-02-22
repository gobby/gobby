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

// TODO: Someone else should do task management

#ifndef _GOBBY_FILE_COMMANDS_HPP_
#define _GOBBY_FILE_COMMANDS_HPP_

#include "operations/operations.hpp"
#include "dialogs/document-location-dialog.hpp"
#include "dialogs/connection-dialog.hpp"
#include "core/browser.hpp"
#include "core/statusbar.hpp"
#include "core/filechooser.hpp"
#include "core/windowactions.hpp"

#include <gtkmm/applicationwindow.h>
#include <sigc++/trackable.h>

namespace Gobby
{

class FileCommands: public sigc::trackable
{
public:
	FileCommands(Gtk::Window& parent, WindowActions& actions,
	             Browser& browser, FolderManager& folder_manager,
	             StatusBar& status_bar, FileChooser& file_chooser,
	             Operations& operations,
	             const DocumentInfoStorage& info_storage,
	             Preferences& preferences);
	~FileCommands();

	class Task: public sigc::trackable
	{
	public:
		typedef sigc::signal<void> SignalFinished;

		Task(FileCommands& file_commands);
		virtual ~Task() = 0;

		virtual void run() = 0;
		void finish();

		Gtk::Window& get_parent();
		const Folder& get_folder();
		StatusBar& get_status_bar();
		FileChooser& get_file_chooser();
		Operations& get_operations();
		const DocumentInfoStorage& get_document_info_storage();
		Preferences& get_preferences();
		DocumentLocationDialog& get_document_location_dialog();

		SignalFinished signal_finished() const
		{
			return m_signal_finished;
		}
	protected:
		FileCommands& m_file_commands;
		SignalFinished m_signal_finished;
	};

	void set_task(Task* task);

protected:
	static void on_row_inserted_static(GtkTreeModel* model, 
	                                   GtkTreePath* path,
					   GtkTreeIter* iter,
					   gpointer user_data)
	{
		static_cast<FileCommands*>(user_data)->on_row_inserted();
	}

	static void on_row_deleted_static(GtkTreeModel* model,
	                                  GtkTreePath* path,
					  gpointer user_data)
	{
		static_cast<FileCommands*>(user_data)->on_row_deleted();
	}

	void on_document_changed(SessionView* view);
	void on_row_inserted();
	void on_row_deleted();
	void on_task_finished();

	void on_new();
	void on_open();
	void on_open_location();
	void on_save();
	void on_save_as();
	void on_save_all();

	void on_export_html();
	void on_connect();

	void on_close();

	void on_connect_response(int response_id);

	void update_sensitivity();

	Gtk::Window& m_parent;
	WindowActions& m_actions;
	Browser& m_browser;
	FolderManager& m_folder_manager;
	StatusBar& m_status_bar;
	FileChooser& m_file_chooser;
	Operations& m_operations;
	const DocumentInfoStorage& m_document_info_storage;
	Preferences& m_preferences;

	// Note: Order is important to get deinitialization right: Task may
	// access the location dialog in its destructor, so make sure the task
	// is freed before the location dialog.
	std::auto_ptr<DocumentLocationDialog> m_location_dialog;
	std::auto_ptr<Task> m_task;

	std::auto_ptr<ConnectionDialog> m_connection_dialog;

	gulong m_row_inserted_handler;
	gulong m_row_deleted_handler;
};

}

#endif // _GOBBY_FILE_COMMANDS_HPP_
