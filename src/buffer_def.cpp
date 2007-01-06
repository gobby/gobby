/* gobby - A GTKmm driven libobby client
 * Copyright (C) 2005, 2006 0x539 dev group
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

#include "buffer_def.hpp"

namespace
{
	const std::string utf8_reprs[] = {
		"UTF-8",
		"UTF8",
		"utf-8",
		"utf8"
	};

	const std::size_t utf8_repr_count =
		sizeof(utf8_reprs) / sizeof(utf8_reprs[0]);
}

bool Gobby::is_subscribable(const LocalDocumentInfo& info)
{
	// Gobby only allows subscriptions to UTF-8 encoded documents
	for(std::size_t n = 0; n < utf8_repr_count; ++ n)
		if(info.get_encoding() == utf8_reprs[n])
			return true;

	return false;
}
