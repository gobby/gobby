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
				view, info->uri,
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
