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
