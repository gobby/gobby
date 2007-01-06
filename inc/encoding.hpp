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

#ifndef _GOBBY_ENCODING_HPP_
#define _GOBBY_ENCODING_HPP_

#include <list>
#include <gtkmm/textview.h>

namespace Gobby
{

class Encoding
{
public:
	enum Charset {
		UTF_7,
		UTF_8,
		UTF_16,
		ISO_8859_1,
		ISO_8859_15,
		UCS_2,
		UCS_4
	};

	Encoding(const Glib::ustring& name, Charset charset);
	Encoding(const Encoding& other);
	~Encoding();

	Encoding& operator=(const Encoding& other);

	const Glib::ustring& get_name() const;
	Charset get_charset() const;

	Glib::ustring convert_to_utf8(const std::string& str);
protected:
	Glib::ustring m_name;
	Charset m_charset;
};

Glib::ustring convert_to_utf8(const std::string& str);

}

#endif // _GOBBY_ENCODING_HPP_

