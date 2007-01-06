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

#include <obby/format_string.hpp>
#include "io/buffer_wrapper.hpp"
#include "common.hpp"

#ifdef WIN32
obby::io::client::client(Gtk::Window& window)
#else
obby::io::client::client()
#endif
 : net6::client(), m_ioconn(NULL)
#ifdef WIN32
   , m_window(window)
#endif
{
}

#ifdef WIN32
obby::io::client::client(Gtk::Window& window, const net6::address& addr)
#else
obby::io::client::client(const net6::address& addr)
#endif
 : net6::client(addr), m_ioconn(NULL)
#ifdef WIN32
   , m_window(window)
#endif
{
	connect_impl(addr);
}

obby::io::client::~client()
{
	if(is_connected() )
		disconnect_impl();
}

void obby::io::client::connect(const net6::address& addr)
{
	net6::client::connect(addr);
	connect_impl(addr);
}

void obby::io::client::disconnect()
{
	disconnect_impl();
	net6::client::disconnect();
}

void obby::io::client::send(const net6::packet& pack)
{
	m_ioconn->add_events(main_connection::IO_OUT);
	net6::client::send(pack);
}

void obby::io::client::on_send_event()
{
	m_ioconn->remove_events(main_connection::IO_OUT);
	net6::client::on_send_event();
}

void obby::io::client::connect_impl(const net6::address& addr)
{
	m_ioconn.reset(
		new main_connection(
#ifdef WIN32
			m_window,
#endif
			conn->get_socket(),
			main_connection::IO_IN | main_connection::IO_ERROR
		)
	);
}

void obby::io::client::disconnect_impl()
{
	m_ioconn.reset(NULL);
}

obby::io::server::Error::Error(Code error_code, const Glib::ustring& error_message)
 : Glib::Error(g_quark_from_static_string("GOBBY_SERVER_ERROR"),
               static_cast<int>(error_code), error_message)
{
}

obby::io::server::Error::Code obby::io::server::Error::code() const
{
	return static_cast<Code>(gobject_->code);
}

#ifdef WIN32
obby::io::server::server(Gtk::Window& window, bool ipv6)
 : net6::server(ipv6), m_window(window)
#else
obby::io::server::server(bool ipv6)
 : net6::server(ipv6)
#endif
{
}

#ifdef WIN32
obby::io::server::server(Gtk::Window& window, unsigned int port, bool ipv6)
 : net6::server(port, ipv6), m_window(window)
#else
obby::io::server::server(unsigned int port, bool ipv6)
 : net6::server(port, ipv6)
#endif
{
	// Base reopen routine has already been called by the base class 
	// constructor
	reopen_impl(port);
}

obby::io::server::~server()
{
	// Base class shutdown routine will be called by the base class
	// destructor
	shutdown_impl();

	// Remove client main_connections
	user_map_type::iterator iter;
	for(iter = m_user_map.begin(); iter != m_user_map.end(); ++ iter)
		delete iter->second;
}

void obby::io::server::shutdown()
{
	// Own shutdown routine
	shutdown_impl();
	// Call base function
	net6::server::shutdown();
}

void obby::io::server::reopen(unsigned int port)
{
	// Call base routine
	net6::server::reopen(port);
	// Call own routine
	reopen_impl(port);
}

void obby::io::server::send(const net6::packet& pack)
{
	// Call base function
	net6::server::send(pack);
}

void obby::io::server::send(const net6::packet& pack, const net6::user& to)
{
	// Add Glib::IO_OUT event
	user_map_type::iterator iter = get_user_iter(to);
	iter->second->add_events(main_connection::IO_OUT);

	// Call base function
	net6::server::send(pack, to);
}

void obby::io::server::on_connect(const net6::user& new_user)
{
	// Build main_connection
	main_connection* conn = new main_connection(
#ifdef WIN32
		m_window,
#endif
		new_user.get_connection().get_socket(),
		main_connection::IO_IN | main_connection::IO_ERROR
	);

	// Insert into user map
	m_user_map[&new_user] = conn;

	// Call base function
	net6::server::on_connect(new_user);
}

void obby::io::server::on_send_event(net6::user& to)
{
	// Find user in user map
	user_map_type::iterator iter = get_user_iter(to);

	// Remove IO_OUT flag because there is no data to be sent anymore
	iter->second->remove_events(main_connection::IO_OUT);

	// Call base function
	net6::server::on_send_event(to);
}

void obby::io::server::remove_client(const net6::user* user)
{
	// Find user in user map
	user_map_type::iterator iter = get_user_iter(*user);

	// Remove main_connection
	delete iter->second;
	m_user_map.erase(iter);

	// Call base function
	net6::server::remove_client(user);
}

obby::io::server::user_map_type::iterator
obby::io::server::get_user_iter(const net6::user& user)
{
	// Find user
	user_map_type::iterator iter = m_user_map.find(&user);

	// Not found?
	if(iter == m_user_map.end() )
	{
		// Should not happen...
		throw Error(
			Error::PEER_NOT_FOUND,
			"obby::io::server::get_user_iter"
		);
	}

	return iter;
}

void obby::io::server::shutdown_impl()
{
	// Remove server main_connection
	m_serv_connection.reset(NULL);
}

void obby::io::server::reopen_impl(unsigned int port)
{
	// Build Main Connection
	m_serv_connection.reset(
		new main_connection(
#ifdef WIN32
			m_window,
#endif
			*serv_sock,
			main_connection::IO_ACCEPT | main_connection::IO_ERROR
		)
	);
}

#ifdef WIN32
obby::io::host::host(Gtk::Window& window, const Glib::ustring& username,
                     bool ipv6)
 : net6::server(ipv6), net6::host(username, ipv6), server(window, ipv6)
#else
obby::io::host::host(const Glib::ustring& username, bool ipv6)
 : net6::server(ipv6), net6::host(username, ipv6), server(ipv6)
#endif
{
}

#ifdef WIN32
obby::io::host::host(Gtk::Window& window, unsigned int port,
                     const Glib::ustring& username, bool ipv6)
 : net6::server(port, ipv6), net6::host(username, ipv6),
   server(window, port, ipv6)
#else
obby::io::host::host(unsigned int port, const Glib::ustring& username,
                     bool ipv6)
 : net6::server(port, ipv6), net6::host(username, ipv6), server(port, ipv6)
#endif
{
}

obby::io::host::~host()
{
}

void obby::io::host::send(const net6::packet& pack)
{
	// Call base function from server
	server::send(pack);
}

void obby::io::host::send(const net6::packet& pack, const net6::user& to)
{
	// Prevent from sendint packets to ourselves
	if(&to == self) return;
	// Call base function
	server::send(pack, to);
}

void obby::io::host::on_send_event(net6::user& to)
{
	// Prevent from sending packets to ourselves
	if(&to == self) return;
	// Call base function otherwise
	server::on_send_event(to);
}

#ifdef WIN32
obby::io::client_buffer::client_buffer(Gtk::Window& window)
#else
obby::io::client_buffer::client_buffer()
#endif
#ifdef WIN32
 : m_window(window)
#endif
{
}

obby::io::client_buffer::base_net_type* obby::io::client_buffer::new_net()
{
#ifdef WIN32
	return new net_type(m_window);
#else
	return new net_type;
#endif
}

#ifdef WIN32
obby::io::server_buffer::server_buffer(Gtk::Window& window)
#else
obby::io::server_buffer::server_buffer()
#endif
#ifdef WIN32
 : m_window(window)
#endif
{
}

obby::io::server_buffer::base_net_type*
obby::io::server_buffer::new_net(unsigned int port)
{
#ifdef WIN32
	return new io::server(m_window, port, false);
#else
	return new io::server(port, false);
#endif
}

#ifdef WIN32
obby::io::host_buffer::host_buffer(Gtk::Window& window,
                                   const Glib::ustring& username,
                                   const colour& colour)
#else
obby::io::host_buffer::host_buffer(const Glib::ustring& username,
                                   const colour& colour)
#endif
 : obby::host_buffer(username, colour),
#ifdef WIN32
   server_buffer(window)
#else
   server_buffer()
#endif
{
}

obby::io::host_buffer::base_net_type*
obby::io::host_buffer::new_net(unsigned int port)
{
#ifdef WIN32
	return new net_type(m_window, port, m_username, false);
#else
	return new net_type(port, m_username, false);
#endif
}

