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

#include "buffer_wrapper_generic.hpp"

Gobby::IOConnection::IOConnection(const net6::socket& sock,
                                  Glib::IOCondition io_condition)
 : m_sock(sock), m_io_channel(), m_io_condition(io_condition), m_io_connection()
{
	net6::socket::socket_type fd = sock.cobj();
	m_io_channel = Glib::IOChannel::create_from_fd(fd);
	reconnect(io_condition);
}

Gobby::IOConnection::~IOConnection()
{
	if(m_io_connection.connected() )
		m_io_connection.disconnect();
}

void Gobby::IOConnection::reconnect(Glib::IOCondition io_condition)
{
	if(io_condition != m_io_condition || !m_io_connection.connected() )
	{
		if(m_io_connection.connected() )
			m_io_connection.disconnect();

		m_io_condition = io_condition;
		m_io_connection = Glib::signal_io().connect(
			sigc::mem_fun(*this, &IOConnection::on_io),
			m_io_channel,
			m_io_condition
		);
	}
}

void Gobby::IOConnection::add_connect(Glib::IOCondition io_condition)
{
	reconnect(m_io_condition | io_condition);
}

void Gobby::IOConnection::remove_connect(Glib::IOCondition io_condition)
{
	reconnect(m_io_condition & ~io_condition);
}

bool Gobby::IOConnection::on_io(Glib::IOCondition condition)
{
	net6::socket& sock = const_cast<net6::socket&>(m_sock);

	if(condition & (Glib::IO_ERR | Glib::IO_HUP | Glib::IO_NVAL) )
		sock.io_event().emit(net6::socket::IOERROR);

	if(condition & (Glib::IO_IN) )
		sock.io_event().emit(net6::socket::INCOMING);

	if(condition & (Glib::IO_OUT) )
		sock.io_event().emit(net6::socket::OUTGOING);

	return true;
}

Gobby::Client::Client(const net6::address& addr)
 : net6::client(addr),
   m_ioconn(conn.get_socket(), Glib::IO_IN | Glib::IO_ERR | Glib::IO_HUP)
{
}

Gobby::Client::~Client()
{
}

void Gobby::Client::send(const net6::packet& pack)
{
	m_ioconn.add_connect(Glib::IO_OUT);
	net6::client::send(pack);
}

void Gobby::Client::on_send_event(const net6::packet& pack)
{
	if(conn.send_queue_size() == 0)
		m_ioconn.remove_connect(Glib::IO_OUT);
	net6::client::on_send_event(pack);
}

Gobby::Host::Error::Error(Code error_code, const Glib::ustring& error_message)
 : Glib::Error(g_quark_from_static_string("GOBBY_HOST_ERROR"),
               static_cast<int>(error_code), error_message)
{
}

Gobby::Host::Error::Code Gobby::Host::Error::code() const
{
	return static_cast<Code>(gobject_->code);
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

	// Add Glib::IO_OUT event
	peer_map_type::iterator iter = get_peer_iter(to);
	iter->second->add_connect(Glib::IO_OUT);

	// Call base function
	net6::host::send(pack, to);
}

void Gobby::Host::on_connect(net6::host::peer& new_peer)
{
	// Build IOConnection
	IOConnection* conn = new IOConnection(
		new_peer.get_socket(),
		Glib::IO_IN | Glib::IO_ERR | Glib::IO_HUP
	);

	// Insert into peer map
	m_peer_map[&new_peer] = conn;
		
	// Call base function
	net6::host::on_join(new_peer);
}

void Gobby::Host::on_send_event(const net6::packet& pack,
                                net6::host::peer& to)
{
	// Find peer in peer map
	peer_map_type::iterator iter = get_peer_iter(to);
	IOConnection* conn = iter->second;
	
	// Remove IO_OUT if nothing to send
	if(to.send_queue_size() == 0)
		iter->second->remove_connect(Glib::IO_OUT);

	// Call base function
	net6::host::on_send_event(pack, to);
}

void Gobby::Host::remove_client(net6::host::peer* peer)
{
	// Find peer in peer map
	peer_map_type::iterator iter = get_peer_iter(*peer);

	// Remove IOConnection
	delete iter->second;
	m_peer_map.erase(iter);

	// Call base function
	net6::host::remove_client(peer);
}

Gobby::Host::peer_map_type::iterator
Gobby::Host::get_peer_iter(const net6::host::peer& peer)
{
	peer_map_type::iterator iter = m_peer_map.find(&peer);
	if(iter == m_peer_map.end() )
	{
		throw Error(
			Error::PEER_NOT_FOUND,
			"Peer " + peer.get_name() + "("
			+ peer.get_address().get_name() +
			") not found in peer list"
		);
	}

	return iter;
}

void Gobby::Host::shutdown_impl()
{
	// Remove server IOConnection
	m_serv_connection.reset(NULL);

	// Remove client IOConnections
	peer_map_type::iterator iter;
	for(iter = m_peer_map.begin(); iter != m_peer_map.end(); ++ iter)
		delete iter->second;
	m_peer_map.clear();
}

void Gobby::Host::reopen_impl(unsigned int port)
{
	// Build IO Connection
	m_serv_connection.reset(
		new IOConnection(
			*serv_sock,
			Glib::IO_IN | Glib::IO_ERR | Glib::IO_HUP
		)
	);
}
