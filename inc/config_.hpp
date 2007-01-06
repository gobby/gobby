/* gobby - A GTKmm driven libobby client
 * Copyright (C) 2005 0x539 dev group
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef _GOBBY_CONFIG_HPP_
#define _GOBBY_CONFIG_HPP_

#include <map>
#include <sstream>
#include <iostream>
#include <glibmm/error.h>
#include <glibmm/ustring.h>
#include <gdkmm/color.h>
#include <libxml++/nodes/element.h>

namespace Gobby
{

// String conversion
template<typename T> Glib::ustring to_string(const T& val)
{
	std::stringstream str;
	str << val;
	return str.str();
}

template<typename T> T from_string(const Glib::ustring& val)
{
	std::stringstream str(val);
	T t;
	str >> t;
	return t;
}
	
class Config
{
public:
	class Error : public Glib::Error
	{
	public:
		enum Code {
			PATH_CREATION_FAILED
		};

		Error(Code error_code, const Glib::ustring& error_message);
		Code code() const;
	};

	class Entry
	{
	public:
		Entry();
		Entry(const Entry& other);
		~Entry();

		Entry& operator=(const Entry& other);

		void load(const xmlpp::Element& element);
		void save(xmlpp::Element& element) const;

		Entry& operator[](const Glib::ustring& index);

		template<typename T> T get(const T& default_value = T()) const
		{
			if(m_value.empty() ) return default_value;
			return from_string<T>(m_value);
		}

		template<typename T> void set(const T& value)
		{
			m_value = to_string<T>(value);
		}
	protected:
		std::map<Glib::ustring, Entry> m_table;
		Glib::ustring m_value;
	};

	Config(const Glib::ustring& file);
	~Config();

	Entry& operator[](const Glib::ustring& to);

protected:
	void create_path_to(const Glib::ustring& to);

	std::map<Glib::ustring, Entry> m_table;
	Glib::ustring m_filename;
};

std::ostream& operator<<(std::ostream& out, const Gdk::Color& color);
std::istream& operator>>(std::istream& in, Gdk::Color& color);

}

#endif // _GOBBY_CONFIG_HPP_
