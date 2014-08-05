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

#ifndef _GOBBY_FILE_TASK_SAVE_HPP_
#define _GOBBY_FILE_TASK_SAVE_HPP_

#include "commands/file-commands.hpp"

namespace Gobby
{

class TaskSave: public FileCommands::Task
{
private:
	FileChooser::Dialog m_file_dialog;
	TextSessionView* m_view;
	bool m_running;

public:
	TaskSave(FileCommands& file_commands, TextSessionView& view);
	virtual void run();

	void on_response(int response_id);
	void on_document_removed(SessionView& view);
};

} // namespace Gobby

#endif // _GOBBY_FILE_TASK_SAVE_HPP_
