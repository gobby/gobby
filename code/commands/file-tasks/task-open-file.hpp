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

#ifndef _GOBBY_FILE_TASK_OPEN_FILE_HPP_
#define _GOBBY_FILE_TASK_OPEN_FILE_HPP_

#include "commands/file-commands.hpp"
#include "commands/file-tasks/task-open.hpp"

namespace Gobby
{

class TaskOpenFile: public FileCommands::Task
{
public:
	TaskOpenFile(FileCommands& file_commands);

	virtual void run();

private:
	void on_file_response(int response_id);

	FileChooser::Dialog m_file_dialog;
	std::auto_ptr<TaskOpen> m_open_task;
};

} // namespace Gobby

#endif // _GOBBY_FILE_TASK_OPEN_FILE_HPP_
