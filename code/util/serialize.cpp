/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2011 Armin Burgmeier <armin@arbur.net>
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
 * Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include "util/serialize.hpp"

const char* Gobby::serialize::type_name<int>::name = "int";
const char* Gobby::serialize::type_name<long>::name = "long";
const char* Gobby::serialize::type_name<short>::name = "short";
const char* Gobby::serialize::type_name<char>::name = "char";
const char* Gobby::serialize::type_name<unsigned int>::name =
	"unsigned int";
const char* Gobby::serialize::type_name<unsigned long>::name =
	"unsigned long";
const char* Gobby::serialize::type_name<unsigned short>::name =
	"unsigned short";
const char* Gobby::serialize::type_name<unsigned char>::name =
	"unsigned char";
const char* Gobby::serialize::type_name<float>::name = "float";
const char* Gobby::serialize::type_name<double>::name = "double";
const char* Gobby::serialize::type_name<long double>::name = "long double";
const char* Gobby::serialize::type_name<bool>::name = "bool";

Gobby::serialize::conversion_error::
	conversion_error(const std::string& message):
	std::runtime_error(message)
{
}

Gobby::serialize::data::data(const std::string& serialized):
	m_serialized(serialized)
{
}

const std::string& Gobby::serialize::data::serialized() const
{
	return m_serialized;
}

std::string Gobby::serialize::default_context_to<std::string>::
	to_string(const data_type& from) const
{
	return from;
}

Gobby::serialize::default_context_from<std::string>::data_type
Gobby::serialize::default_context_from<std::string>::
	from_string(const data_type& from) const
{
	return from;
}

std::string Gobby::serialize::default_context_to<const char*>::
	to_string(const data_type& from) const
{
	return from;
}
