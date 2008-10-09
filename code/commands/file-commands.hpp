/* gobby - A GTKmm driven libobby client
 * Copyright (C) 2005 - 2008 0x539 dev group
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

#ifndef _GOBBY_FILE_COMMANDS_HPP_
#define _GOBBY_FILE_COMMANDS_HPP_

#include "operations/operations.hpp"
#include "dialogs/documentlocationdialog.hpp"
#include "core/header.hpp"
#include "core/browser.hpp"
#include "core/filechooser.hpp"

#include <gtkmm/filechooserdialog.h>
#include <gtkmm/window.h>
#include <sigc++/trackable.h>

namespace Gobby
{

class FileCommands: public sigc::trackable
{
public:
	FileCommands(Gtk::Window& parent, Header& header,
	             Browser& browser, Folder& folder,
	             FileChooser& file_chooser, Operations& operations,
	             const DocumentInfoStorage& info_storage,
	             Preferences& preferences);
	~FileCommands();

	class Task: public sigc::trackable
	{
	public:
		typedef sigc::signal<void> SignalFinished;

		Task(FileCommands& file_commands);
		virtual ~Task() = 0;

		void finish();

		Gtk::Window& get_parent();
		Folder& get_folder();
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
protected:
	void set_task(Task* task);

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

	void on_document_changed(DocWindow* document);
	void on_row_inserted();
	void on_row_deleted();
	void on_task_finished();

	void on_new();
	void on_open();
	void on_save();
	void on_save_as();
	void on_save_all();

	void on_close();
	void on_quit();

	void update_sensitivity();

	Gtk::Window& m_parent;
	Header& m_header;
	Browser& m_browser;
	Folder& m_folder;
	FileChooser& m_file_chooser;
	Operations& m_operations;
	const DocumentInfoStorage& m_document_info_storage;
	Preferences& m_preferences;

	std::auto_ptr<Task> m_task;
	std::auto_ptr<DocumentLocationDialog> m_location_dialog;

	gulong m_row_inserted_handler;
	gulong m_row_deleted_handler;
};

}
	
#endif // _GOBBY_FILE_COMMANDS_HPP_
