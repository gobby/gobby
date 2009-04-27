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

#ifndef _GOBBY_FILE_TASK_OPEN_MULTIPLE_HPP_
#define _GOBBY_FILE_TASK_OPEN_MULTIPLE_HPP_

#include "commands/file-commands.hpp"
#include "core/nodewatch.hpp"

#include <queue>

namespace Gobby
{

class TaskOpenMultiple: public FileCommands::Task
{
public:
	TaskOpenMultiple(FileCommands& file_commands);
	virtual ~TaskOpenMultiple();

	virtual void run();

	void add_file(const Glib::RefPtr<Gio::File>& file);

private:
	void on_query_info(const Glib::RefPtr<Gio::AsyncResult>& result,
	                   Glib::RefPtr<Gio::File> file);
	void on_location_response(int response_id);
	void flush();
	void error(const Glib::ustring& message);

	StatusBar::MessageHandle m_handle;
	struct FileInfo {
		FileInfo(const Glib::RefPtr<Gio::File>& f,
		         const Glib::ustring& n) : file(f), name(n) {}

		Glib::RefPtr<Gio::File> file;
		Glib::ustring name;
	};

	std::queue<FileInfo> m_files;
	unsigned int m_query_counter;
	std::auto_ptr<NodeWatch> m_location;
};

} // namespace Gobby

#endif // _GOBBY_FILE_TASK_OPEN_MULTIPLE_HPP_
