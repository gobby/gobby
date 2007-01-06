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

#include <cstdlib>
#include "common.hpp"
#include "encoding.hpp"

namespace
{

Glib::ustring convert_to_utf8(const std::string& str, const std::string& from)
{
	Glib::ustring utf8 = Glib::convert(str, "UTF-8", from);
	if(!utf8.validate() )
	{
		throw Glib::ConvertError(
			Glib::ConvertError::NO_CONVERSION,
			"Couldn't convert to UTF_8"
		);
	}

	return utf8;
}

}

const std::vector<std::string>& Gobby::Encoding::get_encodings()
{
	static const std::string encodings[] = {
		"UTF-8",
		"ISO-8859-1",
		"ISO-8859-15",
		"UTF-7",
		"UTF-16",
		"UCS-2",
		"UCS-4"
	};

	static const std::size_t encoding_count =
		sizeof(encodings) / sizeof(encodings[0]);

	static std::vector<std::string> encoding_vec(
		encodings,
		encodings + encoding_count
	);

	return encoding_vec;
}

Glib::ustring Gobby::Encoding::convert_to_utf8(const std::string& str,
                                               std::string& encoding)
{
	if(g_utf8_validate(str.c_str(), str.length(), NULL) == TRUE)
	{
		encoding = "UTF-8";
		return str;
	}

	typedef std::vector<std::string> encoding_list_type;
	const encoding_list_type& encodings = get_encodings();

	for(encoding_list_type::const_iterator iter = encodings.begin();
	    iter != encodings.end();
	    ++ iter)
	{
		// Ignore UTF-8 encoding we currently checked
		if(*iter == "UTF-8") continue;

		try
		{
			encoding = *iter;
			return ::convert_to_utf8(str, *iter);
		}
		catch(Glib::ConvertError& e)
		{
			// Retry with next encoding
		}
	}

	// No suitable encoding
	throw Glib::ConvertError(
		Glib::ConvertError::NO_CONVERSION, _(
			"Failed to convert input into UTF-8: Either the "
			"encoding is unknown or it is binary input."
		)
	);
}

