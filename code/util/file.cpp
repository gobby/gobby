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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "util/file.hpp"
#include "util/i18n.hpp"

#include <glibmm/ustring.h>
#include <glibmm/fileutils.h>
#include <glibmm/miscutils.h>

#include <stdexcept>

// For mkdir / CreateDirectory
#ifdef WIN32
#include <windows.h>
#else
#include <glib/gstdio.h>
#include <cstring>
#include <cerrno>
#endif

namespace
{
	void create_directory(const char* path)
	{
#ifdef WIN32
		// TODO: Use widechar API?
		if(CreateDirectoryA(path, NULL) == FALSE)
		{
			LPVOID msgbuf;
			DWORD err = GetLastError();

			// TODO: Use widechar API?
			FormatMessageA(
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM,
				NULL,
				err,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				reinterpret_cast<LPSTR>(&msgbuf),
				0,
				NULL
			);

			std::string error_message = static_cast<LPSTR>(msgbuf);
			LocalFree(msgbuf);

			// TODO: Convert to UTF-8?

			throw std::runtime_error(
				Glib::ustring::compose(
					Gobby::_("Could not create directory "
					  "\"%1\": %2"), std::string(path),
					error_message));
		}
#else
		if(g_mkdir(path, 0755) == -1)
		{
			throw std::runtime_error(
				Glib::ustring::compose(
					Gobby::_("Could not create directory "
					  "\"%1\": %2"), std::string(path),
					std::strerror(errno)));
		}
#endif
	}
}

namespace Gobby
{
	void create_directory_with_parents(const std::string& path)
	{
		// Directory exists, nothing to do
		if(Glib::file_test(path, Glib::FILE_TEST_IS_DIR) )
			return;

		// Find path to the directory to create
		std::string path_to = Glib::path_get_dirname(path);

		// Create this path, if it doesn't exists
		create_directory_with_parents(path_to);

		// Create new directory
		create_directory(path.c_str() );
	}

	std::string config_filename(const std::string& config_directory,
	                            const std::string& filename)
	{
#ifdef G_OS_WIN32
		return Glib::build_filename(
			Glib::build_filename(
				Glib::get_user_config_dir(),
				config_directory),
			filename);
#else
		return Glib::build_filename(
			Glib::build_filename(
				Glib::get_home_dir(), config_directory),
			filename);
#endif
	}
}
