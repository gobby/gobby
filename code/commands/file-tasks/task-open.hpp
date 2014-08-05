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

#ifndef _GOBBY_FILE_TASK_OPEN_HPP_
#define _GOBBY_FILE_TASK_OPEN_HPP_

#include "commands/file-commands.hpp"

#include <giomm/file.h>

namespace Gobby
{

class TaskOpen: public FileCommands::Task
{
public:
	TaskOpen(FileCommands& file_commands,
	         const Glib::RefPtr<Gio::File>& file);
	virtual ~TaskOpen();

	virtual void run();

private:
	void on_query_info(const Glib::RefPtr<Gio::AsyncResult>& result);
	void on_location_response(int response_id);
	void error(const Glib::ustring& message);

	StatusBar::MessageHandle m_handle;
	Glib::RefPtr<Gio::File> m_file;
};

} // namespace Gobby

#endif // _GOBBY_FILE_TASK_OPEN_HPP_
