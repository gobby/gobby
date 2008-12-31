/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008 Armin Burgmeier <armin@arbur.net>
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

#include "commands/file-commands.hpp"
#include "util/i18n.hpp"

#include <gtkmm/stock.h>
#include <giomm/file.h>

namespace
{
	using namespace Gobby;

	// TODO: Split tasks into separate files in commands/file-tasks?
	class TaskNew: public Gobby::FileCommands::Task
	{
	public:
		TaskNew(FileCommands& file_commands):
			Task(file_commands)
		{
			DocumentLocationDialog& dialog =
				get_document_location_dialog();

			dialog.signal_response().connect(
				sigc::mem_fun(*this, &TaskNew::on_response));
			dialog.set_document_name(_("New Document"));
			dialog.present();
		}

		virtual ~TaskNew()
		{
			get_document_location_dialog().hide();
		}

		void on_response(int response_id)
		{
			if(response_id == Gtk::RESPONSE_ACCEPT)
			{
				DocumentLocationDialog& dialog =
					get_document_location_dialog();
			
				InfcBrowserIter iter;
				InfcBrowser* browser =
					dialog.get_selected_directory(&iter);
				g_assert(browser != NULL);

				get_operations().create_document(
					browser, &iter,
					dialog.get_document_name());
			}

			finish();
		}
	};

	class TaskOpen: public Gobby::FileCommands::Task
	{
	private:
		FileChooser::Dialog m_file_dialog;
		std::string m_open_uri;

	public:
		TaskOpen(FileCommands& file_commands):
			Task(file_commands),
			m_file_dialog(get_file_chooser(), get_parent(),
			              _("Choose a text file to open"),
			              Gtk::FILE_CHOOSER_ACTION_OPEN)
		{
			m_file_dialog.signal_response().connect(sigc::mem_fun(
				*this, &TaskOpen::on_file_response));

			m_file_dialog.present();
		}

		virtual ~TaskOpen()
		{
			get_document_location_dialog().hide();
		}

		void on_file_response(int response_id)
		{
			const gchar* const ATTR_DISPLAY_NAME =
				G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME;

			if(response_id == Gtk::RESPONSE_ACCEPT)
			{
				DocumentLocationDialog& dialog =
					get_document_location_dialog();

				dialog.signal_response().connect(
					sigc::mem_fun(
						*this,
						&TaskOpen::
							on_location_response));

				// TODO: Handle multiple selection
				m_open_uri = m_file_dialog.get_uri();
				// TODO: Query the display name
				// asynchronously, and use a default as long
				// as the query is running.
				Glib::RefPtr<Gio::File> file =
					Gio::File::create_for_uri(m_open_uri);
				Glib::RefPtr<Gio::FileInfo> info =
					file->query_info(ATTR_DISPLAY_NAME);

				dialog.set_document_name(
					info->get_display_name());

				m_file_dialog.hide();
				dialog.present();
			}
			else
			{
				finish();
			}
		}

		void on_location_response(int response_id)
		{
			if(response_id == Gtk::RESPONSE_ACCEPT)
			{
				DocumentLocationDialog& dialog =
					get_document_location_dialog();

				InfcBrowserIter iter;
				InfcBrowser* browser =
					dialog.get_selected_directory(&iter);
				g_assert(browser != NULL);

				get_operations().create_document(
					browser, &iter,
					dialog.get_document_name(),
					get_preferences(), m_open_uri, NULL);
			}

			finish();
		}
	};

	class TaskSave: public Gobby::FileCommands::Task
	{
	private:
		FileChooser::Dialog m_file_dialog;
		DocWindow* m_document;

	public:
		TaskSave(FileCommands& file_commands, DocWindow& document):
			Task(file_commands),
			m_file_dialog(
				get_file_chooser(), get_parent(),
				Glib::ustring::compose(_(
					"Choose a location to save document "
					"\"%1\" to"),
					document.get_title()),
				Gtk::FILE_CHOOSER_ACTION_SAVE),
			m_document(&document)
		{
			
			m_file_dialog.signal_response().connect(sigc::mem_fun(
				*this, &TaskSave::on_response));
			get_folder().signal_document_removed().connect(
				sigc::mem_fun(
					*this,
					&TaskSave::on_document_removed));

			const DocumentInfoStorage::Info* info =
				get_document_info_storage().get_info(
					document.get_info_storage_key());

			if(info != NULL && !info->uri.empty())
			{
				m_file_dialog.set_uri(info->uri);
			}
			else
			{
				m_file_dialog.set_current_name(
					document.get_title());
			}

			m_file_dialog.present();
		}

		void on_response(int response_id)
		{
			if(response_id == Gtk::RESPONSE_ACCEPT)
			{
				const std::string& info_storage_key =
					m_document->get_info_storage_key();

				const DocumentInfoStorage::Info* info =
					get_document_info_storage().get_info(
						info_storage_key);

				// TODO: Get encoding from file dialog
				// TODO: Default to CRLF on Windows
				get_operations().save_document(
					*m_document, get_folder(),
					m_file_dialog.get_uri(),
					info ? info->encoding : "UTF-8",
					info ? info->eol_style :
						DocumentInfoStorage::EOL_LF);
			}

			finish();
		}

		void on_document_removed(DocWindow& document)
		{
			// The document we are about to save was removed.
			if(m_document == &document)
				finish();
		}
	};

	class TaskSaveAll: public Gobby::FileCommands::Task
	{
	private:
		std::list<DocWindow*> m_documents;
		std::list<DocWindow*>::iterator m_current;
		std::auto_ptr<TaskSave> m_task;

	public:
		TaskSaveAll(FileCommands& file_commands):
			Task(file_commands)
		{
			typedef Gtk::Notebook_Helpers::PageList PageList;
			PageList& pages = get_folder().pages();

			for(PageList::iterator iter = pages.begin();
			    iter != pages.end(); ++ iter)
			{
				m_documents.push_back(
					static_cast<DocWindow*>(
						iter->get_child()));
			}

			get_folder().signal_document_removed().connect(
				sigc::mem_fun(
					*this,
					&TaskSaveAll::on_document_removed));

			m_current = m_documents.begin();
			process_current();
		}

		void on_document_removed(DocWindow& document)
		{
			std::list<DocWindow*>::iterator iter = std::find(
				m_documents.begin(), m_documents.end(),
				&document);

			if(iter == m_current)
			{
				m_current = m_documents.erase(m_current);
				// Go on with next
				process_current();
			}

			if(iter != m_documents.end())
				m_documents.erase(iter);
		}

		void process_current()
		{
			m_task.reset(NULL);

			if(m_current == m_documents.end())
			{
				finish();
			}
			else
			{
				DocWindow& doc = **m_current;

				const DocumentInfoStorage::Info* info =
					get_document_info_storage().get_info(
						doc.get_info_storage_key());

				if(info != NULL && !info->uri.empty())
				{
					get_operations().save_document(
						doc, get_folder(), info->uri,
						info->encoding,
						info->eol_style);

					m_current =
						m_documents.erase(m_current);
					process_current();
				}
				else
				{
					m_task.reset(
						new TaskSave(m_file_commands,
						             doc));

					m_task->signal_finished().connect(
						sigc::mem_fun(*this,
							&TaskSaveAll::
								on_finished));
				}
			}
		}

		void on_finished()
		{
			m_current = m_documents.erase(m_current);
			process_current();
		}
	};
} // anonymous namespace

Gobby::FileCommands::Task::Task(FileCommands& file_commands):
	m_file_commands(file_commands)
{
}

Gobby::FileCommands::Task::~Task()
{
}

void Gobby::FileCommands::Task::finish()
{
	// Note this could delete this:
	m_signal_finished.emit();
}

Gtk::Window& FileCommands::Task::get_parent()
{
	return m_file_commands.m_parent;
}

Gobby::Folder& Gobby::FileCommands::Task::get_folder()
{
	return m_file_commands.m_folder;
}

Gobby::FileChooser& Gobby::FileCommands::Task::get_file_chooser()
{
	return m_file_commands.m_file_chooser;
}

Gobby::Operations& Gobby::FileCommands::Task::get_operations()
{
	return m_file_commands.m_operations;
}

const Gobby::DocumentInfoStorage&
Gobby::FileCommands::Task::get_document_info_storage()
{
	return m_file_commands.m_document_info_storage;
}

Gobby::Preferences& Gobby::FileCommands::Task::get_preferences()
{
	return m_file_commands.m_preferences;
}

Gobby::DocumentLocationDialog&
Gobby::FileCommands::Task::get_document_location_dialog()
{
	if(m_file_commands.m_location_dialog.get() == NULL)
	{
		m_file_commands.m_location_dialog.reset(
			new DocumentLocationDialog(
				m_file_commands.m_parent,
				INF_GTK_BROWSER_MODEL(
					m_file_commands.m_browser.
						get_store())));
	}

	return *m_file_commands.m_location_dialog;
}

Gobby::FileCommands::FileCommands(Gtk::Window& parent, Header& header,
                                  Browser& browser, Folder& folder,
                                  FileChooser& file_chooser,
                                  Operations& operations,
				  const DocumentInfoStorage& info_storage,
                                  Preferences& preferences):
	m_parent(parent), m_header(header), m_browser(browser),
	m_folder(folder), m_file_chooser(file_chooser),
	m_operations(operations), m_document_info_storage(info_storage),
	m_preferences(preferences)
{
	header.action_file_new->signal_activate().connect(
		sigc::mem_fun(*this, &FileCommands::on_new));
	header.action_file_open->signal_activate().connect(
		sigc::mem_fun(*this, &FileCommands::on_open));
	header.action_file_save->signal_activate().connect(
		sigc::mem_fun(*this, &FileCommands::on_save));
	header.action_file_save_as->signal_activate().connect(
		sigc::mem_fun(*this, &FileCommands::on_save_as));
	header.action_file_save_all->signal_activate().connect(
		sigc::mem_fun(*this, &FileCommands::on_save_all));
	header.action_file_close->signal_activate().connect(
		sigc::mem_fun(*this, &FileCommands::on_close));
	header.action_file_quit->signal_activate().connect(
		sigc::mem_fun(*this, &FileCommands::on_quit));

	folder.signal_document_changed().connect(
		sigc::mem_fun(*this, &FileCommands::on_document_changed));

	InfGtkBrowserStore* store = browser.get_store();
	m_row_inserted_handler =
		g_signal_connect(G_OBJECT(store), "row-inserted",
		                 G_CALLBACK(on_row_inserted_static), this);
	m_row_deleted_handler =
		g_signal_connect(G_OBJECT(store), "row-deleted",
		                 G_CALLBACK(on_row_deleted_static), this);

	update_sensitivity();	
}

Gobby::FileCommands::~FileCommands()
{
	InfGtkBrowserStore* store = m_browser.get_store();
	g_signal_handler_disconnect(G_OBJECT(store), m_row_inserted_handler);
	g_signal_handler_disconnect(G_OBJECT(store), m_row_deleted_handler);
}

void Gobby::FileCommands::set_task(Task* task)
{
	task->signal_finished().connect(sigc::mem_fun(
		*this, &FileCommands::on_task_finished));
	m_task.reset(task);
}

void Gobby::FileCommands::on_document_changed(DocWindow* document)
{
	update_sensitivity();
}

void Gobby::FileCommands::on_row_inserted()
{
	update_sensitivity();
}

void Gobby::FileCommands::on_row_deleted()
{
	update_sensitivity();
}

void Gobby::FileCommands::on_task_finished()
{
	m_task.reset(NULL);
}

void Gobby::FileCommands::on_new()
{
	set_task(new TaskNew(*this));
}

void Gobby::FileCommands::on_open()
{
	set_task(new TaskOpen(*this));
}

void Gobby::FileCommands::on_save()
{
	// TODO: Encoding selection in file chooser
	DocWindow* document = m_folder.get_current_document();
	g_assert(document != NULL);

	const DocumentInfoStorage::Info* info =
		m_document_info_storage.get_info(
			document->get_info_storage_key());

	if(info != NULL && !info->uri.empty())
	{
		m_operations.save_document(
			*document, m_folder, info->uri, info->encoding,
			info->eol_style);
	}
	else
	{
		on_save_as();
	}
}

void Gobby::FileCommands::on_save_as()
{
	DocWindow* document = m_folder.get_current_document();
	g_assert(document != NULL);

	set_task(new TaskSave(*this, *document));
}

void Gobby::FileCommands::on_save_all()
{
	set_task(new TaskSaveAll(*this));
}

void Gobby::FileCommands::on_close()
{
	DocWindow* document = m_folder.get_current_document();
	g_assert(document != NULL);

	m_folder.remove_document(*document);
}

void Gobby::FileCommands::on_quit()
{
	m_parent.hide();
}

void Gobby::FileCommands::update_sensitivity()
{
	GtkTreeIter dummy_iter;
	bool create_sensitivity = gtk_tree_model_get_iter_first(
		GTK_TREE_MODEL(m_browser.get_store()), &dummy_iter);
	gboolean active_sensitivity = m_folder.get_current_document() != NULL;

	m_header.action_file_new->set_sensitive(create_sensitivity);
	m_header.action_file_open->set_sensitive(create_sensitivity);

	m_header.action_file_save->set_sensitive(active_sensitivity);
	m_header.action_file_save_as->set_sensitive(active_sensitivity);
	m_header.action_file_save_all->set_sensitive(active_sensitivity);
	m_header.action_file_close->set_sensitive(active_sensitivity);
}
