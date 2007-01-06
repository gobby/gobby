/* gobby - A GTKmm driven libobby client
 * Copyright (C) 2005 0x539 dev group
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

#ifndef _GOBBY_MIMEMAP_HPP_
#define _GOBBY_MIMEMAP_HPP_

#include <map>
#include <glibmm/ustring.h>
#include <net6/non_copyable.hpp>

namespace Gobby
{

/** Table which translates a file extension into a mime type.
 */

class MimeMap : private net6::non_copyable
{
public:
	typedef std::map<Glib::ustring, Glib::ustring> map_type;

	MimeMap();
	~MimeMap();

	/** Returns the mime type for a given file name.
	 */
	Glib::ustring get_mime_type_by_file(
		const Glib::ustring& filename) const;

	/** Returns the mime type for a given file extension.
	 */
	Glib::ustring get_mime_type_by_extension(
		const Glib::ustring& extension) const;

protected:
	map_type m_map;
};

}

#endif // _GOBBY_MIMEMAP_HPP_
