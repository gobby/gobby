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
#include <obby/format_string.hpp>
#include "common.hpp"
#include "buffer_wrapper.hpp"

Gobby::Client::Client(Gtk::Window& window, const net6::address& addr)
 : net6::client(addr),
   m_ioconn(
	window,
	conn.get_socket(),
	MainConnection::IO_IN | MainConnection::IO_ERROR
   )
{
}

Gobby::Client::~Client()
{
}

void Gobby::Client::send(const net6::packet& pack)
{
	m_ioconn.add_events(MainConnection::IO_OUT);
	net6::client::send(pack);
}

void Gobby::Client::on_send_event(const net6::packet& pack)
{
	if(conn.send_queue_size() == 0)
		m_ioconn.remove_events(MainConnection::IO_OUT);

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

Gobby::Host::Host(Gtk::Window& window, const Glib::ustring& username, bool ipv6)
 : net6::host(username, ipv6), m_window(window)
{
}

Gobby::Host::Host(Gtk::Window& window, unsigned int port,
                  const Glib::ustring& username, bool ipv6)
 : net6::host(port, username, ipv6), m_window(window)
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

	// Remove client MainConnections
	peer_map_type::iterator iter;
	for(iter = m_peer_map.begin(); iter != m_peer_map.end(); ++ iter)
		delete iter->second;
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
	iter->second->add_events(MainConnection::IO_OUT);

	// Call base function
	net6::host::send(pack, to);
}

void Gobby::Host::on_connect(net6::host::peer& new_peer)
{
	// Build MainConnection
	MainConnection* conn = new MainConnection(
		m_window,
		new_peer.get_socket(),
		MainConnection::IO_IN | MainConnection::IO_ERROR
	);

	// Insert into peer map
	m_peer_map[&new_peer] = conn;

	// Call base function
	net6::host::on_connect(new_peer);
}

void Gobby::Host::on_send_event(const net6::packet& pack,
                                net6::host::peer& to)
{
	// Find peer in peer map
	peer_map_type::iterator iter = get_peer_iter(to);
	MainConnection* conn = iter->second;

	// Remove IO_OUT if nothing to send
	if(to.send_queue_size() == 0)
		iter->second->remove_events(MainConnection::IO_OUT);

	// Call base function
	net6::host::on_send_event(pack, to);
}

void Gobby::Host::remove_client(net6::host::peer* peer)
{
	// Find peer in peer map
	peer_map_type::iterator iter = get_peer_iter(*peer);

	// Remove MainConnection
	delete iter->second;
	m_peer_map.erase(iter);

	// Call base function
	net6::host::remove_client(peer);
}

Gobby::Host::peer_map_type::iterator
Gobby::Host::get_peer_iter(const net6::host::peer& peer)
{
	// Find peer
	peer_map_type::iterator iter = m_peer_map.find(&peer);

	// Not found?
	if(iter == m_peer_map.end() )
	{
		// Should not happen...
		obby::format_string str(
			_("Peer %0 (%1) not found in peer list") );
		str << peer.get_name() << peer.get_address().get_name();
		throw Error(Error::PEER_NOT_FOUND, str.str() );
	}

	return iter;
}

void Gobby::Host::shutdown_impl()
{
	// Remove server MainConnection
	m_serv_connection.reset(NULL);
}

void Gobby::Host::reopen_impl(unsigned int port)
{
	// Build Main Connection
	m_serv_connection.reset(
		new MainConnection(
			m_window,
			*serv_sock,
			MainConnection::IO_ACCEPT | MainConnection::IO_ERROR
		)
	);
}

Gobby::ClientBuffer::ClientBuffer(Gtk::Window& window,
                                  const Glib::ustring& hostname,
                                  unsigned int port)
 : obby::client_buffer()
{
	net6::ipv4_address addr(
		net6::ipv4_address::create_from_hostname(hostname, port) );

	m_client = new Client(window, addr);
	register_signal_handlers();
}

Gobby::ClientBuffer::~ClientBuffer()
{
}

Gobby::HostBuffer::HostBuffer()
 : obby::host_buffer()
{
}

Gobby::HostBuffer::HostBuffer(Gtk::Window& window, unsigned int port,
                              const Glib::ustring& username, int red,
                              int green, int blue)
 : obby::host_buffer()
{
	net6::host* host = new Host(window, port, username, false);
	m_server = host;

	assert(host->get_self() != NULL);
	m_self = m_usertable.add_user(*host->get_self(), red, green, blue);
	register_signal_handlers();
}

Gobby::HostBuffer::~HostBuffer()
{
}

