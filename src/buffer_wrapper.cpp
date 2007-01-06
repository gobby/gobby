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

#include <cassert>
#include <glib/gmessages.h>
#include <libobby/client_user_table.hpp>
#include <libobby/host_user_table.hpp>
#include "buffer_wrapper.hpp"

Gobby::Client::Client(const net6::address& addr)
 : net6::client(addr)
{
	net6::socket::socket_type sock = conn.get_socket().cobj();

#ifdef WIN32
	m_io_channel = Glib::IOChannel::create_from_win32_socket(sock);
#else
	m_io_channel = Glib::IOChannel::create_from_fd(sock);
#endif

	m_io_condition = Glib::IO_IN | Glib::IO_ERR | Glib::IO_HUP;
	m_io_connection = Glib::signal_io().connect(
		sigc::mem_fun(*this, &Client::on_io),
		m_io_channel,
		m_io_condition
	);
}

Gobby::Client::~Client()
{
	if(m_io_connection.connected() )
		m_io_connection.disconnect();
}

void Gobby::Client::send(const net6::packet& pack)
{
	if(~m_io_condition & Glib::IO_OUT)
	{
		m_io_condition |= Glib::IO_OUT;
		m_io_connection.disconnect();

		m_io_connection = Glib::signal_io().connect(
			sigc::mem_fun(*this, &Client::on_io),
			m_io_channel,
			m_io_condition
		);
	}

	net6::client::send(pack);
}

void Gobby::Client::on_client_send(const net6::packet& pack)
{
	if(conn.send_queue_size() == 0)
	{
		m_io_condition &= ~Glib::IO_OUT;
		m_io_connection.disconnect();

		m_io_connection = Glib::signal_io().connect(
			sigc::mem_fun(*this, &Client::on_io),
			m_io_channel,
			m_io_condition
		);
	}

	net6::client::on_client_send(pack);
}

bool Gobby::Client::on_io(Glib::IOCondition condition)
{
	net6::tcp_client_socket& sock =
		const_cast<net6::tcp_client_socket&>(conn.get_socket() );

	if(condition & (Glib::IO_ERR | Glib::IO_HUP) )
		sock.error_event().emit(sock, net6::socket::ERROR);
	
	if(condition & (Glib::IO_IN) )
		sock.read_event().emit(sock, net6::socket::INCOMING);

	if(condition & (Glib::IO_OUT) )
		sock.write_event().emit(sock, net6::socket::OUTGOING);

	return true;
}

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

Gobby::Host::Host(const Glib::ustring& username, bool ipv6)
 : net6::host(username, ipv6)
{
}

Gobby::Host::Host(unsigned int port, const Glib::ustring& username, bool ipv6)
 : net6::host(port, username, ipv6)
{
	// Base reopen routine has already been called by the base class 
	// constructor
	reopen_impl(port);
}

Gobby::Host::~Host()
{
	// Base class shutdown routine will be called by the base class
	// destructor
	shutdown_impl();
}

void Gobby::Host::shutdown()
{
	// Own shutdown routine
	shutdown_impl();
	// Call base function
	net6::host::shutdown();
}

void Gobby::Host::reopen(unsigned int port)
{
	// Call base routine
	net6::host::reopen(port);
	// Call own routine
	reopen_impl(port);
}

void Gobby::Host::send(const net6::packet& pack, net6::host::peer& to)
{
	// Prevent from sending packets to ourselves
	if(&to == self) return;

	// Find peer in peer map
	std::map<net6::host::peer*, peer_data*>::iterator iter =
		m_peer_map.find(&to);
	if(iter == m_peer_map.end() )
	{
		// Uh, not found?
		// TODO: Exception class? Would be overhead..?
		g_warning("send(): Could not find peer %p in peer map!", &to);
	}
	else
	{
		peer_data* data = iter->second;
		if(~data->io_condition & Glib::IO_OUT)
		{
			data->io_condition |= Glib::IO_OUT;
			data->io_connection.disconnect();

			data->io_connection = Glib::signal_io().connect(
				sigc::bind(
					sigc::mem_fun(*this, &Host::on_io),
					&to
				),
				data->io_channel,
				data->io_condition
			);
		}

		net6::host::send(pack, to);
	}
}

void Gobby::Host::on_join(net6::host::peer& new_peer)
{
	// Register peer_data
	peer_data* data = new peer_data;
	net6::socket::socket_type sock = new_peer.get_socket().cobj();

	// Create IO channel
#ifdef WIN32
	data->io_channel = Glib::IOChannel::create_from_win32_socket(sock);
#else
	data->io_channel = Glib::IOChannel::create_from_fd(sock);
#endif

	// Connection to signal_io
	data->io_condition = Glib::IO_IN | Glib::IO_ERR | Glib::IO_HUP;
	data->io_connection = Glib::signal_io().connect(
		sigc::bind(sigc::mem_fun(*this, &Host::on_io), &new_peer),
		data->io_channel,
		data->io_condition
	);

	m_peer_map[&new_peer] = data;

	// Call base function
	net6::host::on_join(new_peer);
}

void Gobby::Host::on_client_send(const net6::packet& pack,
                                 net6::host::peer& from)
{
	std::map<net6::host::peer*, peer_data*>::iterator iter =
		m_peer_map.find(&from);
	if(iter == m_peer_map.end() )
	{
		// Uh, not found?
		// TODO: Exception class? Would be overhead..?
		g_warning("on_client_send(): "
		          "Could not find peer %p in peer map!", &from);
	}
	else
	{
		peer_data* data = iter->second;
		if(from.send_queue_size() == 0)
		{
			data->io_condition &= ~Glib::IO_OUT;
			data->io_connection.disconnect();

			data->io_connection = Glib::signal_io().connect(
				sigc::bind(
					sigc::mem_fun(this, &Host::on_io),
					&from
				),
				data->io_channel,
				data->io_condition
			);
		}

		net6::host::on_client_send(pack, from);
	}
}

void Gobby::Host::remove_client(net6::host::peer* peer)
{
	std::map<net6::host::peer*, peer_data*>::iterator iter =
		m_peer_map.find(peer);
	if(iter == m_peer_map.end() )
	{
		// Uh, not found?
		// TODO: Exception class? Would be overhead..?
		g_warning("remove_client(): "
		          "Could not find peer %p in peer map!", peer);
	}
	else
	{
		if(iter->second->io_connection.connected() )
			iter->second->io_connection.disconnect();
		delete iter->second;
		m_peer_map.erase(iter);

		net6::host::remove_client(peer);
	}
}

bool Gobby::Host::on_server_io(Glib::IOCondition condition)
{
	if(condition & (Glib::IO_ERR | Glib::IO_HUP ) )
		serv_sock->error_event().emit(
			*serv_sock, net6::socket::ERROR);

	if(condition & (Glib::IO_IN) )
		serv_sock->read_event().emit(
			*serv_sock, net6::socket::INCOMING);

	if(condition & (Glib::IO_OUT) )
		serv_sock->write_event().emit(
			*serv_sock, net6::socket::OUTGOING);

	return true;
}

bool Gobby::Host::on_io(Glib::IOCondition condition, net6::host::peer* peer)
{
	std::map<net6::host::peer*, peer_data*>::iterator iter =
		m_peer_map.find(peer);
	if(iter == m_peer_map.end() )
	{
		// Uh, not found?
		g_warning("on_io(): Could not find peer %p in peer map!", peer);
		return false;
	}
	else
	{
		peer_data* data = iter->second;
		net6::tcp_client_socket& sock =
			const_cast<net6::tcp_client_socket&>(
				peer->get_socket() );

		if(condition & (Glib::IO_ERR | Glib::IO_HUP) )
			sock.error_event().emit(sock, net6::socket::ERROR);

		if(condition & Glib::IO_IN)
			sock.read_event().emit(sock, net6::socket::INCOMING);

		if(condition & Glib::IO_OUT)
			sock.write_event().emit(sock, net6::socket::OUTGOING);
	}

	return true;
}

void Gobby::Host::shutdown_impl()
{
	// Disconnect from signal_io
	if(m_io_connection.connected() )
		m_io_connection.disconnect();

	// Delete IOChannel on serv socket
	m_io_channel.clear();

	// Clear peer map
	std::map<net6::host::peer*, peer_data*>::iterator iter;
	for(iter = m_peer_map.begin(); iter != m_peer_map.end(); ++ iter)
	{
		if(iter->second->io_connection.connected() )
			iter->second->io_connection.disconnect();
		delete iter->second;
	}
	m_peer_map.clear();
}

void Gobby::Host::reopen_impl(unsigned int port)
{
	// Get C socket object
	net6::socket::socket_type sock = serv_sock->cobj();

	// Build IO Channel
#ifdef WIN32
	m_io_channel = Glib::IOChannel::create_from_win32_socket(sock);
#else
	m_io_channel = Glib::IOChannel::create_from_fd(sock);
#endif

	// Connect to signal_io()
	m_io_connection = Glib::signal_io().connect(
		sigc::mem_fun(*this, &Host::on_server_io),
		m_io_channel,
		Glib::IO_IN
	);
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
