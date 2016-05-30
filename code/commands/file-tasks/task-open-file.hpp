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

#ifndef _GOBBY_FILE_TASK_OPEN_FILE_HPP_
#define _GOBBY_FILE_TASK_OPEN_FILE_HPP_

#include "commands/file-commands.hpp"
#include "commands/file-tasks/task-open.hpp"
#include "commands/file-tasks/task-open-multiple.hpp"

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
	std::unique_ptr<TaskOpen> m_open_task;
	std::unique_ptr<TaskOpenMultiple> m_open_taskm;
};

} // namespace Gobby

#endif // _GOBBY_FILE_TASK_OPEN_FILE_HPP_
