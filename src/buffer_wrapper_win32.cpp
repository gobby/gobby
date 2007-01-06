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

#include <map>
#include <glibmm/main.h>
#include "buffer_wrapper_win32.hpp"

Gobby::Client::Client(const net6::address& addr)
 : net6::client(addr)
{
	m_timer_connection = Glib::signal_timeout().connect(
		sigc::mem_fun(*this, &Client::on_timer), 400);
}

Gobby::Client::~Client()
{
	if(m_timer_connection.connected() )
		m_timer_connection.disconnect();
}

bool Gobby::Client::on_timer()
{
	unsigned int conn_queue_size, new_queue_size;

	do
	{
		conn_queue_size = conn.send_queue_size();
		select(0);
		new_queue_size = conn.send_queue_size();
	} while(conn_queue_size > new_queue_size);

	return true;
}

Gobby::Host::Host(const Glib::ustring& username, bool ipv6)
 : net6::host(username, ipv6)
{
	m_timer_connection = Glib::signal_timeout().connect(
		sigc::mem_fun(*this, &Host::on_timer), 400);
}

Gobby::Host::Host(unsigned int port, const Glib::ustring& username, bool ipv6)
 : net6::host(port, username, ipv6)
{
	m_timer_connection = Glib::signal_timeout().connect(
		sigc::mem_fun(*this, &Host::on_timer), 400);
}

Gobby::Host::~Host()
{
	if(m_timer_connection.connected() )
		m_timer_connection.disconnect();
}

bool Gobby::Host::on_timer()
{
	// Setup map with the queue-sizes of all the client connections
	std::map<net6::host::peer*, unsigned int> queue_sizes;
	std::list<net6::host::peer*>::iterator iter;
	for(iter = peers.begin(); iter != peers.end(); ++ iter)
		if(*iter != self)
			queue_sizes[*iter] = (*iter)->send_queue_size();

	// Select until the queue-sizes do not change any more
	bool re_select = true;
	while(re_select)
	{
		re_select = false;
		select(0);

		// Check each client
		for(iter = peers.begin(); iter != peers.end(); ++ iter)
		{
			std::map<net6::host::peer*, unsigned int>::iterator
				map_iter = queue_sizes.find(*iter);

			// Not in the map: Maybe the client has just been
			// accepted. Send things next time the timer is elapsed
			if(map_iter == queue_sizes.end() )
				continue;

			// Reselect if the queue size changed, so another packet
			// may be written
			if(map_iter->second > (*iter)->send_queue_size() )
				re_select = true;

			// Update queue-size
			queue_sizes[*iter] = (*iter)->send_queue_size();
		}
	}

	return true;
}

