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

#include <cassert>
#include <libobby/client_user_table.hpp>
#include <libobby/host_user_table.hpp>
#include "buffer_wrapper.hpp"

Gobby::ClientBuffer::ClientBuffer()
{
}

Gobby::ClientBuffer::ClientBuffer(const Glib::ustring& hostname,
                                  unsigned int port)
 : obby::client_buffer()
{
	net6::ipv4_address addr(
		net6::ipv4_address::create_from_hostname(hostname, port) );

	m_client = new Client(addr);
	m_usertable = new obby::client_user_table(*m_client, *this);

	register_signal_handlers();
}

Gobby::ClientBuffer::~ClientBuffer()
{
}

Gobby::HostBuffer::HostBuffer()
 : obby::host_buffer()
{
}

Gobby::HostBuffer::HostBuffer(unsigned int port, const Glib::ustring& username,
                              int red, int green, int blue)
 : obby::host_buffer()
{
	net6::host* host = new Host(port, username, false);
	m_server = host;

	m_usertable = new obby::host_user_table(*host, *this);

	assert(host->get_self() != NULL);
	m_self = add_user(*host->get_self(), red, green, blue);

	register_signal_handlers();
}

Gobby::HostBuffer::~HostBuffer()
{
}
