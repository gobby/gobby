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

#include "commands/file-tasks/task-save-all.hpp"

Gobby::TaskSaveAll::TaskSaveAll(FileCommands& file_commands):
	Task(file_commands)
{
}

void Gobby::TaskSaveAll::run() {
	typedef Gtk::Notebook_Helpers::PageList PageList;
	PageList& pages = get_folder().pages();

	for(PageList::iterator iter = pages.begin();
	    iter != pages.end(); ++ iter)
	{
		m_documents.push_back(
			static_cast<DocWindow*>(iter->get_child()));
	}

	get_folder().signal_document_removed().connect(
		sigc::mem_fun(*this, &TaskSaveAll::on_document_removed));

	m_current = m_documents.begin();
	process_current();
}

void Gobby::TaskSaveAll::on_document_removed(DocWindow& document)
{
	std::list<DocWindow*>::iterator iter = std::find(
		m_documents.begin(), m_documents.end(), &document);

	if(iter == m_current)
	{
		m_current = m_documents.erase(m_current);
		// Go on with next
		process_current();
	}

	if(iter != m_documents.end())
		m_documents.erase(iter);
}

void Gobby::TaskSaveAll::on_finished()
{
	m_current = m_documents.erase(m_current);
	process_current();
}

void Gobby::TaskSaveAll::process_current()
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
				info->encoding, info->eol_style);

			m_current = m_documents.erase(m_current);
			process_current();
		}
		else
		{
			m_task.reset(new TaskSave(m_file_commands, doc));

			m_task->signal_finished().connect(sigc::mem_fun(
				*this, &TaskSaveAll::on_finished));
			m_task->run();
		}
	}
}
