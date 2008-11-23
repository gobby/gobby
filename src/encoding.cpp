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
	static std::vector<std::string> encoding_vec;

	if(!encoding_vec.empty())
		return encoding_vec;

	static const char *encoding_list =
	/* Translators: the msgid should not be localized.
	 * The msgstr is the list of encodings separated by bar. e.g.
	 * msgstr "EUC-JP|SHIFT-JIS|ISO-2022-JP|UTF-8|UCS-2|UCS-4" */
		N_("UTF-8|ISO-8859-1|ISO-8859-15|UTF-7|UTF-16|UCS-2|UCS-4");

	static gchar **encodings = g_strsplit(_(encoding_list), "|", 0);

	unsigned int encoding_count = g_strv_length(encodings);

	encoding_vec.reserve(encoding_count);
	for(int i = 0; i < encoding_count; i++)
		encoding_vec.push_back(encodings[i]);

	g_strfreev(encodings);

	std::string current_encoding;
	if(!Glib::get_charset(current_encoding))
	{
		const std::vector<std::string>::const_iterator begin =
			encoding_vec.begin();
		const std::vector<std::string>::const_iterator end =
			encoding_vec.end();

		if(std::find(begin, end, current_encoding) == end)
			encoding_vec.push_back(current_encoding);
	}

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

