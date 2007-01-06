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

#ifndef _GOBBY_UNIX_HPP_
#define _GOBBY_UNIX_HPP_

// Unix domain socket wrapper

#include <sys/socket.h>
#include <sys/un.h>

#include <net6/address.hpp>

namespace Gobby
{

namespace Unix
{

/** @brief Unix address.
 */
class Address: public net6::address
{
public:
	Address();

	/** @brief Creates an address by copying a sockaddr_un object.
	 */
	Address(const sockaddr_un* other);

	/** @brief Copies a unix address.
	 */
	Address(const Address& other);

	virtual ~Address();

	/** @brief Creates a copy of this address.
	 */
	virtual net6::address* clone() const;

	/** @brief Returns a name for this socket.
	 *
	 * This is either the filename or, in the case of a socket in the
	 * abstract namespace, a static string.
	 *
	 * TODO: One might think about base64 representation of the unique
	 * byte sequence identifying the socket in the namespace.
	 */
	virtual std::string get_name() const;

	/** @brief Returns the size of the underlaying object.
	 */
	virtual socklen_t get_size() const;

	/** @brief Copies a sockaddr_un.
	 */
	Address& operator=(const sockaddr_un* other);

	/** @brief Creates a copy of another unix address.
	 */
	Address& operator=(const Address& other);

	sockaddr_un* cobj() { return reinterpret_cast<sockaddr_un*>(addr); }
	const sockaddr_un* cobj() const
		{ return reinterpret_cast<sockaddr_un*>(addr); }

	static const size_t UNIX_PATH_MAX = 108;
protected:
	Address(const char* un_path, size_t len);
};

/** @brief Unix address in the abstract namespace.
 */
class AbstractAddress: public Address
{
public:
	static const size_t NAME_LENGTH = UNIX_PATH_MAX - 1;

	AbstractAddress(const char unique_name[NAME_LENGTH]);
};

/** @brief Unix address in the filesystem.
 */
class FileAddress: public Address
{
public:
	FileAddress(const char* filename);
};

// tcp_client_socket is slightly misnamed. It should be called
// stream_socket or something since it works perfectly with unix
// domain socket addresses.

} // namespace Unix

} // namespace Gobby

#endif // _GOBBY_UNIX_HPP_
