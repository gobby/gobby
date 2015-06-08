/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2015 Armin Burgmeier <armin@arbur.net>
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

#include "commands/file-commands.hpp"

#include "commands/file-tasks/task-new.hpp"
#include "commands/file-tasks/task-open-file.hpp"
#include "commands/file-tasks/task-open-location.hpp"
#include "commands/file-tasks/task-save.hpp"
#include "commands/file-tasks/task-save-all.hpp"
#include "commands/file-tasks/task-export-html.hpp"

#include "util/i18n.hpp"

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

Gtk::Window& Gobby::FileCommands::Task::get_parent()
{
	return m_file_commands.m_parent;
}

const Gobby::Folder& Gobby::FileCommands::Task::get_folder()
{
	return m_file_commands.m_folder_manager.get_text_folder();
}

Gobby::StatusBar& Gobby::FileCommands::Task::get_status_bar()
{
	return m_file_commands.m_status_bar;
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
		m_file_commands.m_location_dialog =
			DocumentLocationDialog::create(
				m_file_commands.m_parent,
				INF_GTK_BROWSER_MODEL(
					m_file_commands.m_browser.
						get_store()));
	}

	return *m_file_commands.m_location_dialog;
}

Gobby::FileCommands::FileCommands(Gtk::Window& parent,
                                  WindowActions& actions,
                                  Browser& browser,
                                  FolderManager& folder_manager,
                                  StatusBar& status_bar,
                                  FileChooser& file_chooser,
                                  Operations& operations,
                                  const DocumentInfoStorage& info_storage,
                                  Preferences& preferences):
	m_parent(parent), m_actions(actions), m_browser(browser),
	m_folder_manager(folder_manager), m_status_bar(status_bar),
	m_file_chooser(file_chooser), m_operations(operations),
	m_document_info_storage(info_storage), m_preferences(preferences)
{
	actions.new_document->signal_activate().connect(sigc::hide(
		sigc::mem_fun(*this, &FileCommands::on_new)));
	actions.open->signal_activate().connect(sigc::hide(
		sigc::mem_fun(*this, &FileCommands::on_open)));
	actions.open_location->signal_activate().connect(sigc::hide(
		sigc::mem_fun(*this, &FileCommands::on_open_location)));
	actions.save->signal_activate().connect(sigc::hide(
		sigc::mem_fun(*this, &FileCommands::on_save)));
	actions.save_as->signal_activate().connect(sigc::hide(
		sigc::mem_fun(*this, &FileCommands::on_save_as)));
	actions.save_all->signal_activate().connect(sigc::hide(
		sigc::mem_fun(*this, &FileCommands::on_save_all)));
	actions.export_html->signal_activate().connect(sigc::hide(
		sigc::mem_fun(*this, &FileCommands::on_export_html)));
	actions.connect->signal_activate().connect(sigc::hide(
		sigc::mem_fun(*this, &FileCommands::on_connect)));
	actions.close->signal_activate().connect(sigc::hide(
		sigc::mem_fun(*this, &FileCommands::on_close)));

	folder_manager.get_text_folder().signal_document_changed().connect(
		sigc::mem_fun(*this, &FileCommands::on_document_changed));

	InfGtkBrowserModelSort* store = browser.get_store();
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
	InfGtkBrowserModelSort* store = m_browser.get_store();
	g_signal_handler_disconnect(G_OBJECT(store), m_row_inserted_handler);
	g_signal_handler_disconnect(G_OBJECT(store), m_row_deleted_handler);
}

void Gobby::FileCommands::set_task(Task* task)
{
	task->signal_finished().connect(sigc::mem_fun(
		*this, &FileCommands::on_task_finished));
	m_task.reset(task);
	task->run();
}

void Gobby::FileCommands::on_document_changed(SessionView* view)
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
	set_task(new TaskOpenFile(*this));
}

void Gobby::FileCommands::on_open_location()
{
	set_task(new TaskOpenLocation(*this));
}

void Gobby::FileCommands::on_save()
{
	SessionView* view =
		m_folder_manager.get_text_folder().get_current_document();
	TextSessionView* text_view = dynamic_cast<TextSessionView*>(view);
	g_assert(text_view != NULL);

	const DocumentInfoStorage::Info* info =
		m_document_info_storage.get_info(
			text_view->get_info_storage_key());

	if(info != NULL && !info->uri.empty())
	{
		Glib::RefPtr<Gio::File> file =
			Gio::File::create_for_uri(info->uri);
		m_operations.save_document(
			*text_view, file,
			info->encoding, info->eol_style);
	}
	else
	{
		on_save_as();
	}
}

void Gobby::FileCommands::on_save_as()
{
	SessionView* view =
		m_folder_manager.get_text_folder().get_current_document();
	TextSessionView* text_view = dynamic_cast<TextSessionView*>(view);
	g_assert(text_view != NULL);

	set_task(new TaskSave(*this, *text_view));
}

void Gobby::FileCommands::on_save_all()
{
	set_task(new TaskSaveAll(*this));
}

void Gobby::FileCommands::on_export_html()
{
	SessionView* view =
		m_folder_manager.get_text_folder().get_current_document();
	TextSessionView* text_view = dynamic_cast<TextSessionView*>(view);
	g_assert(text_view != NULL);

	set_task(new TaskExportHtml(*this, *text_view));
}

void Gobby::FileCommands::on_connect()
{
	if(m_connection_dialog.get() == NULL)
	{
		m_connection_dialog = ConnectionDialog::create(m_parent);

		m_connection_dialog->signal_response().connect(
			sigc::mem_fun(*this,
				&FileCommands::on_connect_response));

		m_connection_dialog->add_button(_("Cancel"),
		                                Gtk::RESPONSE_CANCEL);
		m_connection_dialog->add_button(_("Connect"),
		                                Gtk::RESPONSE_ACCEPT);
	}

	m_connection_dialog->present();
}

void Gobby::FileCommands::on_close()
{
	SessionView* view =
		m_folder_manager.get_text_folder().get_current_document();
	g_assert(view != NULL);

	m_folder_manager.remove_document(*view);
}

void Gobby::FileCommands::on_connect_response(int response_id)
{
	if(response_id == Gtk::RESPONSE_ACCEPT)
	{
		const Glib::ustring& host =
			m_connection_dialog->get_host_name();
		if(!host.empty())
		{
			m_operations.subscribe_path(host);
		}
	}

	m_connection_dialog->hide();
}

void Gobby::FileCommands::update_sensitivity()
{
	GtkTreeIter dummy_iter;
	const bool create_sensitivity = gtk_tree_model_get_iter_first(
		GTK_TREE_MODEL(m_browser.get_store()), &dummy_iter);

	SessionView* view =
		m_folder_manager.get_text_folder().get_current_document();
	const bool view_sensitivity = view != NULL;

	// We can only save text documents currently
	const bool text_sensitivity =
		dynamic_cast<TextSessionView*>(view) != NULL;

	m_actions.new_document->set_enabled(create_sensitivity);
	m_actions.open->set_enabled(create_sensitivity);
	m_actions.open_location->set_enabled(create_sensitivity);

	m_actions.save->set_enabled(text_sensitivity);
	m_actions.save_as->set_enabled(text_sensitivity);
	m_actions.save_all->set_enabled(text_sensitivity);
	m_actions.export_html->set_enabled(text_sensitivity);
	m_actions.close->set_enabled(view_sensitivity);
}
