/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2014 Armin Burgmeier <armin@arbur.net>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
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
