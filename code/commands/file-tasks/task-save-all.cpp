/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2013 Armin Burgmeier <armin@arbur.net>
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

// TODO: This should not be a task because the asynchronous IO operations
// should not be interrupted by tasks like "save as".
// The should-not-abort part should be an operation.

#include "commands/file-tasks/task-save-all.hpp"

Gobby::TaskSaveAll::TaskSaveAll(FileCommands& file_commands):
	Task(file_commands)
{
}

void Gobby::TaskSaveAll::run()
{
	const unsigned int n_pages = get_folder().get_n_pages();
	for(unsigned int i = 0; i < n_pages; ++i)
	{
		SessionView& view = get_folder().get_document(i);
		TextSessionView* text_view =
			dynamic_cast<TextSessionView*>(&view);

		if(text_view)
			m_views.push_back(text_view);
	}

	get_folder().signal_document_removed().connect(
		sigc::mem_fun(*this, &TaskSaveAll::on_document_removed));

	m_current = m_views.begin();
	process_current();
}

void Gobby::TaskSaveAll::on_document_removed(SessionView& view)
{
	std::list<TextSessionView*>::iterator iter = std::find(
		m_views.begin(), m_views.end(), &view);

	if(iter == m_current)
	{
		m_current = m_views.erase(m_current);
		// Go on with next
		process_current();
	}

	if(iter != m_views.end())
		m_views.erase(iter);
}

void Gobby::TaskSaveAll::on_finished()
{
	m_current = m_views.erase(m_current);
	process_current();
}

void Gobby::TaskSaveAll::process_current()
{
	m_task.reset(NULL);

	if(m_current == m_views.end())
	{
		finish();
	}
	else
	{
		TextSessionView& view = **m_current;

		const DocumentInfoStorage::Info* info =
			get_document_info_storage().get_info(
				view.get_info_storage_key());

		if(info != NULL && !info->uri.empty())
		{
			get_operations().save_document(
				view, get_folder(), info->uri,
				info->encoding, info->eol_style);

			m_current = m_views.erase(m_current);
			process_current();
		}
		else
		{
			m_task.reset(new TaskSave(m_file_commands, view));

			m_task->signal_finished().connect(sigc::mem_fun(
				*this, &TaskSaveAll::on_finished));
			m_task->run();
		}
	}
}
