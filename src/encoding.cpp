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

#include "common.hpp"
#include "encoding.hpp"

namespace
{
	// Available encodings
	Gobby::Encoding encodings[] = {
		Gobby::Encoding("UTF-8", Gobby::Encoding::UTF_8),
		Gobby::Encoding("ISO_8859-1", Gobby::Encoding::ISO_8859_1),
		Gobby::Encoding("ISO_8859-15", Gobby::Encoding::ISO_8859_15),
		Gobby::Encoding("UTF-7", Gobby::Encoding::UTF_7),
		Gobby::Encoding("UTF-16", Gobby::Encoding::UTF_16),
		Gobby::Encoding("UCS-2", Gobby::Encoding::UCS_2),
		Gobby::Encoding("UCS-4", Gobby::Encoding::UCS_4)
	};

	// Amount of encodings we have
	unsigned int encoding_count = sizeof(encodings) / sizeof(encodings[0]);
};

Gobby::Encoding::Encoding(const Glib::ustring& name, Charset charset)
 : m_name(name), m_charset(charset)
{
}

Gobby::Encoding::Encoding(const Encoding& other)
 : m_name(other.m_name), m_charset(other.m_charset)
{
}

Gobby::Encoding::~Encoding()
{
}

Gobby::Encoding& Gobby::Encoding::operator=(const Encoding& other)
{
	m_name = other.m_name;
	m_charset = other.m_charset;
}

const Glib::ustring& Gobby::Encoding::get_name() const
{
	return m_name;
}

Gobby::Encoding::Charset Gobby::Encoding::get_charset() const
{
	return m_charset;
}

Glib::ustring Gobby::Encoding::convert_to_utf8(const std::string& str)
{
	return Glib::convert(str, "UTF-8", m_name);
}

Glib::ustring Gobby::convert_to_utf8(const std::string& str)
{
	if(g_utf8_validate(str.c_str(), str.length(), NULL) == TRUE)
		return str;

	// Ignore UTF-8 encoding we currently checked
	for(unsigned int i = 1; i < encoding_count; ++ i)
	{
		try
		{
			return encodings[i].convert_to_utf8(str);
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

