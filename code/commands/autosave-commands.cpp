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

#include "commands/autosave-commands.hpp"

#include "operations/operation-save.hpp"

#include "core/sessionuserview.hpp"

#include <ctime>

class Gobby::AutosaveCommands::Info
{
public:
	Info(AutosaveCommands& commands, TextSessionView& view):
		m_commands(commands), m_view(view), m_save_op(NULL)
	{
		GtkSourceBuffer* buffer = m_view.get_text_buffer();

		m_modified_changed_handler = g_signal_connect_after(
			G_OBJECT(buffer), "modified-changed",
			G_CALLBACK(on_modified_changed_static), this);

		// We can't get this correct, so we assume the document was
		// synchronized to disk at the current time. If it has been
		// modified, we schedule a first autosave.
		m_sync_time = std::time(NULL);

		Operations& operations = m_commands.m_operations;
		OperationSave* save_op =
			operations.get_save_operation_for_document(view);
		if(save_op != NULL)
			begin_save_operation(save_op);
		else if(gtk_text_buffer_get_modified(GTK_TEXT_BUFFER(buffer)))
			schedule();
	}

	~Info()
	{
		GtkSourceBuffer* buffer = m_view.get_text_buffer();

		g_signal_handler_disconnect(G_OBJECT(buffer),
		                            m_modified_changed_handler);

		m_timeout_handler.disconnect();
	}

	// Called by AutosaveCommands when the timeout interval has changed.
	// Just reschedule the timeout.
	void reschedule()
	{
		if(m_timeout_handler.connected())
		{
			m_timeout_handler.disconnect();
			schedule();
		}
	}

	// Called by AutosaveCommands
	void begin_save_operation(OperationSave* save_op)
	{
		g_assert(m_save_op == NULL);

		// The document is already being saved, so we don't
		// need to autosave anymore. Reschedule autosave when save
		// operation finished.
		if(m_timeout_handler.connected())
			m_timeout_handler.disconnect();

		m_save_op = save_op;

		m_save_op->signal_finished().connect(
			sigc::mem_fun(
				*this,
				&Info::on_save_operation_finished));
	}

protected:
	void on_modified_changed()
	{
		if(m_save_op == NULL)
		{
			GtkTextBuffer* buffer =
				GTK_TEXT_BUFFER(m_view.get_text_buffer());

			if(!gtk_text_buffer_get_modified(buffer))
				m_timeout_handler.disconnect();
			else
			{
				// Until now the document on disk is in sync
				// with the in-memory buffer. Now the document
				// has changed, so schedule an autosave.
				m_sync_time = std::time(NULL);
				schedule();
			}
		}
	}

	void schedule()
	{
		g_assert(!m_timeout_handler.connected());
		g_assert(m_save_op == NULL);

		// Don't schedule a timeout in case the document has no entry
		// in the document info storage. This means we don't have an
		// uri yet where to save the document. However, we
		// automatically retry when the document is assigned an URI,
		// since the modification flag will change with this anyway.
		const std::string& key = m_view.get_info_storage_key();
		const DocumentInfoStorage::Info* info =
			m_commands.m_info_storage.get_info(key);
		if(!info || info->uri.empty())
			return;

		guint elapsed_seconds = std::time(NULL) - m_sync_time;
		guint autosave_interval = 60 *
			m_commands.m_preferences.editor.autosave_interval;

		if(elapsed_seconds > autosave_interval)
		{
			on_timeout();
		}
		else
		{
			m_timeout_handler =
				Glib::signal_timeout().connect_seconds(
					sigc::mem_fun(
						*this, &Info::on_timeout),
					autosave_interval - elapsed_seconds);
		}
	}

	void on_save_operation_finished(bool success)
	{
		GtkTextBuffer* buffer =
			GTK_TEXT_BUFFER(m_view.get_text_buffer());

		if(success)
			m_sync_time = m_save_op->get_start_time();

		m_save_op = NULL;

		// Schedule the next save operation in case the buffer has
		// been modified since the save operation was started.
		if(gtk_text_buffer_get_modified(buffer))
			schedule();
	}

	bool on_timeout()
	{
		const std::string& key = m_view.get_info_storage_key();
		const DocumentInfoStorage::Info* info =
			m_commands.m_info_storage.get_info(key);

		// Might have been removed from info in the meanwhile, so
		// don't assert here.
		if(info != NULL)
		{
			m_commands.m_operations.save_document(
				m_view, m_commands.m_folder,
				info->uri, info->encoding, info->eol_style);

			g_assert(m_save_op != NULL);

			// Set sync time to operation's start time even though
			// we don't know yet whether the operation will fail,
			// because otherwise we would try to save the
			// document the whole time. This way, we simply retry
			// when the timeout triggers again.
			m_sync_time = m_save_op->get_start_time();
		}

		return false;
	}

private:
	static void on_modified_changed_static(GtkTextBuffer* buffer,
	                                       gpointer user_data)
	{
		static_cast<Info*>(user_data)->on_modified_changed();
	}

	AutosaveCommands& m_commands;
	TextSessionView& m_view;

	gulong m_modified_changed_handler;

	sigc::connection m_timeout_handler;
	OperationSave* m_save_op;

	std::time_t m_sync_time;
};

Gobby::AutosaveCommands::AutosaveCommands(Folder& folder,
                                          Operations& operations,
                                          const DocumentInfoStorage& storage,
					  const Preferences& preferences):
	m_folder(folder), m_operations(operations),
	m_info_storage(storage), m_preferences(preferences)
{
	m_folder.signal_document_added().connect(
		sigc::mem_fun(*this, &AutosaveCommands::on_document_added));
	m_folder.signal_document_removed().connect(
		sigc::mem_fun(*this, &AutosaveCommands::on_document_removed));

	m_operations.signal_begin_save_operation().connect(
		sigc::mem_fun(
			*this,
			&AutosaveCommands::on_begin_save_operation));

	m_preferences.editor.autosave_enabled.signal_changed().connect(
		sigc::mem_fun(
			*this,
			&AutosaveCommands::on_autosave_enabled_changed));

	m_preferences.editor.autosave_interval.signal_changed().connect(
		sigc::mem_fun(
			*this,
			&AutosaveCommands::on_autosave_interval_changed));

	// Create autosave infos for initial documents
	on_autosave_enabled_changed();
}

Gobby::AutosaveCommands::~AutosaveCommands()
{
	for(InfoMap::iterator iter = m_info_map.begin();
	    iter != m_info_map.end(); ++ iter)
	{
		delete iter->second;
	}
}

void Gobby::AutosaveCommands::on_document_added(SessionView& view)
{
	if(m_preferences.editor.autosave_enabled)
	{
		// We can only save text views:
		TextSessionView* text_view =
			dynamic_cast<TextSessionView*>(&view);
		if(text_view)
		{
			g_assert(m_info_map.find(text_view) ==
			         m_info_map.end());
			m_info_map[text_view] = new Info(*this, *text_view);
		}
	}
}

void Gobby::AutosaveCommands::on_document_removed(SessionView& view)
{
	if(m_preferences.editor.autosave_enabled)
	{
		TextSessionView* text_view =
			dynamic_cast<TextSessionView*>(&view);

		if(text_view)
		{
			InfoMap::iterator iter = m_info_map.find(text_view);
			g_assert(iter != m_info_map.end());
			delete iter->second;
			m_info_map.erase(iter);
		}
	}
}

void Gobby::AutosaveCommands::on_begin_save_operation(
	OperationSave* operation)
{
	TextSessionView* view = operation->get_view();
	// Save operation just started, document must be present
	g_assert(view != NULL);

	if(m_preferences.editor.autosave_enabled)
	{
		InfoMap::iterator iter = m_info_map.find(view);
		g_assert(iter != m_info_map.end());

		iter->second->begin_save_operation(operation);
	}
}

void Gobby::AutosaveCommands::on_autosave_enabled_changed()
{
	if(m_preferences.editor.autosave_enabled)
	{
		for(unsigned int i = 0;
		    i < static_cast<unsigned int>(m_folder.get_n_pages());
		    ++i)
		{
			// TODO: Add convenience API to folder, so that we
			// don't need to know here that it actually contains
			// SessionUserViews.
			SessionUserView* userview =
				static_cast<SessionUserView*>(
					m_folder.get_nth_page(i));

			SessionView& view = userview->get_session_view();
			TextSessionView* text_view =
				dynamic_cast<TextSessionView*>(&view);

			if(text_view)
			{
				m_info_map[text_view] =
					new Info(*this, *text_view);
			}
		}
	}
	else
	{
		for(InfoMap::iterator iter = m_info_map.begin();
		    iter != m_info_map.end(); ++ iter)
		{
			delete iter->second;
		}

		m_info_map.clear();
	}
}

void Gobby::AutosaveCommands::on_autosave_interval_changed()
{
	// Propagate to infos
	for(InfoMap::iterator iter = m_info_map.begin();
	    iter != m_info_map.end(); ++ iter)
	{
		return iter->second->reschedule();
	}
}
