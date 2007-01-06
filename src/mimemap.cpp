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

#include "mimemap.hpp"

Gobby::MimeMap::MimeMap()
{
	m_map["ada"] = "text/x-ada";
	m_map["c"] = "text/x-c";
	m_map["h"] = "text/x-c++";
	m_map["hh"] = "text/x-c++";
	m_map["cpp"] = "text/x-c++";
	m_map["hpp"] = "text/x-c++";
	m_map["cc"] = "text/x-c++";
	m_map["css"] = "text/css";
	m_map["diff"] = "text/x-diff";
	m_map["f"] = "text/x-fortran";
	m_map["f77"] = "text/x-fortran";
	m_map["hs"] = "text/x-haskell";
	m_map["htm"] = "text/html";
	m_map["html"] = "text/html";
	m_map["xhtml"] = "text/html";
	// Wi geth IDL?
	m_map["java"] = "text/x-java";
	m_map["js"] = "text/x-javascript";
	m_map["tex"] = "text/x-tex";
	m_map["latex"] = "text/x-tex";
	m_map["lua"] = "text/x-lua";
	// Wi geth MSIL?
	m_map["dpr"] = "text/x-pascal";
	m_map["pas"] = "text/x-pascal";
	m_map["pl"] = "text/x-perl";
	m_map["pm"] = "text/x-perl";
	m_map["php"] = "text/x-php";
	m_map["php3"] = "text/x-php";
	m_map["php4"] = "text/x-php";
	m_map["php5"] = "text/x-php";
	m_map["po"] = "text/x-gettext-translation";
	m_map["py"] = "text/x-python";
	m_map["rb"] = "text/x-ruby";
	m_map["sql"] = "text/x-sql";
	// Wi geth texinfo?
	// Wi geth vb.NET?
	// Wi geth verilog?
	m_map["xml"] = "text/xml";
}

Gobby::MimeMap::~MimeMap()
{
}

Glib::ustring Gobby::MimeMap::get_mime_type_by_file(
	const Glib::ustring& filename) const
{
	Glib::ustring::size_type pos = filename.rfind('.');
	if(pos == Glib::ustring::npos) return "";

	Glib::ustring extension = filename.substr(pos + 1);
	return get_mime_type_by_extension(extension);
}

Glib::ustring Gobby::MimeMap::get_mime_type_by_extension(
	const Glib::ustring& extension) const
{
	map_type::const_iterator iter = m_map.find(extension);
	if(iter == m_map.end() )
		return "";
	else
		return iter->second;
}

