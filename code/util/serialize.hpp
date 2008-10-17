/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008 Armin Burgmeier <armin@arbur.net>
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

#ifndef _GOBBY_SERIALIZE_HPP_
#define _GOBBY_SERIALIZE_HPP_

#include <string>
#include <sstream>
#include <stdexcept>

namespace Gobby
{

/** Generic stuff to de/serialize data types to/from strings.
 */
namespace serialize
{

/** Error that is thrown if conversion from a string fails. For example, if
 * "t3" should be converted to int.
 */
class conversion_error: public std::runtime_error
{
public:
	conversion_error(const std::string& message);
};

/** @brief Several built-in type names.
 */
template<typename data_type> struct type_name {};

template<> struct type_name<int> { static const char* name; };
template<> struct type_name<long> { static const char* name; };
template<> struct type_name<short> { static const char* name; };
template<> struct type_name<char> { static const char* name; };
template<> struct type_name<unsigned int> { static const char* name; };
template<> struct type_name<unsigned long> { static const char* name; };
template<> struct type_name<unsigned short> { static const char* name; };
template<> struct type_name<unsigned char> { static const char* name; };
template<> struct type_name<float> { static const char* name; };
template<> struct type_name<double> { static const char* name; };
template<> struct type_name<long double> { static const char* name; };
template<> struct type_name<bool> { static const char* name; };

/** Abstract base context type to convert something to a string.
 */
template<typename data_type>
class context_base_to
{
public:
	virtual ~context_base_to() {}

	/** @brief Converts the given data type to a string.
	 */
	virtual std::string to_string(const data_type& from) const = 0;
};

/** Abstract base context type to convert a string to another type.
 */
template<typename data_type>
class context_base_from
{
public:
	virtual ~context_base_from() {}

	/** @brief Converts a string to a data type. Might throw
	 * serialize::conversion_error.
	 */
	virtual data_type from_string(const std::string& from) const = 0;
};

/** Default context to convert something literally to a string.
 */
template<typename data_type>
class default_context_to: public context_base_to<data_type>
{
public:
	/** @brief Converts the given data type to a string.
	 */
	virtual std::string to_string(const data_type& from) const;

protected:
	/** Method derived classes may overload to alter the conversion.
	 */
	virtual void on_stream_setup(std::stringstream& stream) const;
};

/** Default context to convert a string literally to a type.
 */
template<typename data_type>
class default_context_from: public context_base_from<data_type>
{
public:
	/** @brief Converts the given string to the type specified
	 * as template parameter.
	 *
	 * May throw serialize::conversion_error().
	 */
	virtual data_type from_string(const std::string& from) const;

protected:
	/** Method derived classes may overload to alter the conversion.
	 */
	virtual void on_stream_setup(std::stringstream& stream) const;
};

#if 0
/** Context that uses hexadecimal representation for numerical types.
 */
template<typename data_type>
class hex_context_to: public default_context_to<data_type>
{
protected:
	virtual void on_stream_setup(std::stringstream& stream) const;
};

/** Context that uses hexadecimal representation for numerical types.
 */
template<typename data_type>
class hex_context_from: public default_context_from<data_type>
{
public:
	virtual void on_stream_setup(std::stringstream& stream) const;
};
#endif

template<>
class default_context_to<std::string>: public context_base_to<std::string>
{
public:
	typedef std::string data_type;

	virtual std::string to_string(const data_type& from) const;
};

template<>
class default_context_from<std::string>: public context_base_from<std::string>
{
public:
	typedef std::string data_type;

	virtual data_type from_string(const std::string& from) const;
};

template<>
class default_context_to<const char*>: public context_base_to<const char*>
{
public:
	typedef const char* data_type;

	virtual std::string to_string(const data_type& from) const;
};

template<std::size_t N>
class default_context_to<char[N]>: public context_base_to<char[N]>
{
public:
	typedef char data_type[N];

	virtual std::string to_string(const data_type& from) const;
};

/** A serialized object.
 */
class data
{
public:
	/** Uses the given string as serialized data without converting it.
	 */
	data(const std::string& serialized);

	/** Serialises the given object with the given context. A default
	 * context is used if no one is given.
	 */
	template<typename type>
	data(const type& data,
	     const context_base_to<type>& ctx = default_context_to<type>());

	/** Returns the serialized data.
	 */
	const std::string& serialized() const;

	/** Deserializes the object with the given context. A default context
	 * is used of no one is given.
	 */
	template<typename type>
	type as(const context_base_from<type>& ctx =
		default_context_from<type>()) const;

protected:
	std::string m_serialized;
};

template<typename data_type>
std::string default_context_to<data_type>::
	to_string(const data_type& from) const
{
	std::stringstream stream;
	on_stream_setup(stream);
	stream << from;
	return stream.str();
}

template<typename data_type>
data_type default_context_from<data_type>::
	from_string(const std::string& from) const
{
	std::stringstream stream(from);
	on_stream_setup(stream);
	data_type data;
	stream >> data;

	if(stream.bad() )
	{
		throw conversion_error(
			"Could not convert \"" + from + "\" to " +
			type_name<data_type>::name
		);
	}

	return data;
}

template<typename data_type>
void default_context_to<data_type>::
	on_stream_setup(std::stringstream& stream) const
{
}

template<typename data_type>
void default_context_from<data_type>::
	on_stream_setup(std::stringstream& stream) const
{
}
#if 0
template<typename data_type>
void hex_context_to<data_type>::
	on_stream_setup(std::stringstream& stream) const
{
	stream << std::hex;
}

template<typename data_type>
void hex_context_from<data_type>::
	on_stream_setup(std::stringstream& stream) const
{
	stream >> std::hex;
}
#endif

template<typename data_type>
data::data(const data_type& data, const context_base_to<data_type>& ctx):
	m_serialized(ctx.to_string(data) )
{
}

template<typename data_type>
data_type data::as(const context_base_from<data_type>& ctx) const
{
	return ctx.from_string(m_serialized);
}

template<size_t N>
std::string default_context_to<char[N]>::
	to_string(const data_type& from) const
{
	return from;
}

} // namespace serialize

} // namespace Gobby

#endif // _GOBBY_SERIALIZE_HPP_
