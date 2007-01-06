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

#include <stdexcept>
#include "unix.hpp"

namespace
{
	const size_t UNIX_PATH_MAX = Gobby::Unix::Address::UNIX_PATH_MAX;
	const size_t NAME_LENGTH = Gobby::Unix::AbstractAddress::NAME_LENGTH;

	// Makes a byte sequence required by sockaddr_un out of
	// a unique byte order to identify the socket.
	class UniqueName
	{
	public:
		UniqueName(const char unique_name[NAME_LENGTH])
		{
			name[0] = '\0';
			std::memcpy(name + 1, unique_name, NAME_LENGTH);
		}

		const char* data() { return name; }

	private:
		char name[UNIX_PATH_MAX];
	};
}

Gobby::Unix::Address::Address():
	net6::address()
{
	sockaddr_un* unaddr = new sockaddr_un;
	addr = reinterpret_cast<sockaddr*>(unaddr);

	unaddr->sun_family = AF_UNIX;
}

Gobby::Unix::Address::Address(const char* un_path, size_t len):
	net6::address()
{
	if(len > UNIX_PATH_MAX)
	{
		throw std::logic_error(
			"Gobby::Unix::Address::Address:\n"
			"Given address path exceeds maximum"
		);
	}

	sockaddr_un* unaddr = new sockaddr_un;
	addr = reinterpret_cast<sockaddr*>(unaddr);

	unaddr->sun_family = AF_UNIX;
	std::memcpy(unaddr->sun_path, un_path, len);
}

Gobby::Unix::Address::Address(const sockaddr_un* other):
	net6::address()
{
	addr = reinterpret_cast<sockaddr*>(new sockaddr_un);
	std::memcpy(addr, other, sizeof(sockaddr_un) );
}

Gobby::Unix::Address::Address(const Address& other)
{
	addr = reinterpret_cast<sockaddr*>(new sockaddr_un);
	std::memcpy(addr, other.cobj(), sizeof(sockaddr_un) );
}

Gobby::Unix::Address::~Address()
{
	delete addr;
	addr = NULL;
}

net6::address* Gobby::Unix::Address::clone() const
{
	return new Address(*this);
}

std::string Gobby::Unix::Address::get_name() const
{
	if(reinterpret_cast<sockaddr_un*>(addr)->sun_path[0] == '\0')
		return "<abstract namespace address>";

	return reinterpret_cast<sockaddr_un*>(addr)->sun_path;
}

socklen_t Gobby::Unix::Address::get_size() const
{
	return sizeof(sockaddr_un);
}

Gobby::Unix::Address& Gobby::Unix::Address::operator=(const sockaddr_un* other)
{
	std::memcpy(addr, other, sizeof(sockaddr_un) );
	return *this;
}

Gobby::Unix::Address& Gobby::Unix::Address::operator=(const Address& other)
{
	if(&other == this)
		return *this;

	std::memcpy(addr, other.cobj(), sizeof(sockaddr_un) );
	return *this;
}

Gobby::Unix::AbstractAddress::
	AbstractAddress(const char unique_name[NAME_LENGTH]):
	Address(UniqueName(unique_name).data(), UNIX_PATH_MAX)
{
}

Gobby::Unix::FileAddress::FileAddress(const char* filename):
	Address(filename, strlen(filename) + 1)
{
}
