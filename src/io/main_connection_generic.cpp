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

#include "io/main_connection.hpp"

namespace
{
	// Glib callback
	bool on_io(Glib::IOCondition condition,
	           const obby::io::main_connection& connection)
	{
		const net6::socket/*&*/ /*const_*/sock = connection.get_socket();
//		net6::socket& sock = const_cast<net6::socket&>(const_sock);

		if(condition & (Glib::IO_ERR | Glib::IO_HUP | Glib::IO_NVAL) )
			sock.io_event().emit(net6::socket::IOERROR);

		if(condition & (Glib::IO_IN) )
			sock.io_event().emit(net6::socket::INCOMING);

		if(condition & (Glib::IO_OUT) )
			sock.io_event().emit(net6::socket::OUTGOING);

		return true;
	}
}

obby::io::main_connection::main_connection(const net6::socket& sock,
                                           Condition condition)
 : m_socket(sock),
   m_condition(static_cast<main_connection::Condition>(0) )
{
	net6::socket::socket_type fd = sock.cobj();
	m_channel = Glib::IOChannel::create_from_fd(fd);
	set_events(condition);
}

obby::io::main_connection::~main_connection()
{
	if(m_connection.connected() )
		m_connection.disconnect();
}

void obby::io::main_connection::set_events(Condition condition)
{
	// Do only do something if we shall watch for other conditions
	if(m_condition != condition)
	{
		// Build Glib::IOCondition for Glib::signal_io
		Glib::IOCondition gcondition =
			static_cast<Glib::IOCondition>(0);

		if(condition & IO_IN) gcondition |= Glib::IO_IN;
		if(condition & IO_ACCEPT) gcondition |= Glib::IO_IN;
		if(condition & IO_OUT) gcondition |= Glib::IO_OUT;

		if(condition & IO_ERROR)
			gcondition |=
				(Glib::IO_HUP | Glib::IO_NVAL | Glib::IO_ERR);

		// Disconnect current connection
		if(m_connection.connected() )
			m_connection.disconnect();

		// Save conditions we are currently watching for
		m_condition = condition;

		// Get notified if something happens
		m_connection = Glib::signal_io().connect(
			sigc::bind(
				sigc::ptr_fun(&on_io),
				sigc::ref(*this)
			),
			m_channel,
			gcondition
		);
	}
}

void obby::io::main_connection::add_events(Condition condition)
{
	set_events(m_condition | condition);
}

void obby::io::main_connection::remove_events(Condition condition)
{
	set_events(m_condition & ~condition);
}
