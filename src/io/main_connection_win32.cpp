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

#include <winsock2.h>
#include <windows.h>
#include <gdk/gdkwin32.h>
#include <net6/error.hpp>
#include "io/main_connection.hpp"

#include <iostream>

namespace
{
	// Note that this is not threadsafe, but we do not need threading here
	// anyways: We register all existing main_connections in a map that
	// associates their socket to the connection to regain the
	// main_connection object in the event handler.
	typedef std::map<net6::socket::socket_type, obby::io::main_connection*>
		connmap_type;

	// The map of main_connections
	connmap_type connmap;

	// Registers a new main_connection
	void register_connection(obby::io::main_connection& conn)
	{
		connmap[conn.get_socket().cobj()] = &conn;
	}

	// Unregisters a main_connection
	void unregister_connection(obby::io::main_connection& conn)
	{
		connmap_type::iterator iter =
			connmap.find(conn.get_socket().cobj() );

		if(iter != connmap.end() )
			connmap.erase(iter);
	}

	// Returns the main_connection for a given socket
	obby::io::main_connection*
	get_connection(net6::socket::socket_type socket)
	{
		connmap_type::iterator iter = connmap.find(socket);

		if(iter == connmap.end() )
			return NULL;
		else
			return iter->second;
	}

	// Network message
	const UINT WM_NETWORK = RegisterWindowMessageA("GOBBY_NETWORK_EVENT");

	// Gets a HWND for a given Gtk::Window
	HWND hwnd_from_window(Gtk::Window& window)
	{
		return reinterpret_cast<HWND>(
			GDK_WINDOW_HWND(window.get_window()->gobj() )
		);
	}

	// Callback function for window filter
	GdkFilterReturn on_filter(GdkXEvent* xevent,
	                          GdkEvent* event,
	                          gpointer data)
	{
		// Get window message
		MSG* msg = static_cast<MSG*>(xevent);

		// Network event?
		if(msg->message == WM_NETWORK)
		{
			long error, event;
			net6::socket::socket_type c_socket;

			// Get parameter from windows message
			c_socket = msg->wParam;
			error = WSAGETSELECTERROR(msg->lParam);
			event = WSAGETSELECTEVENT(msg->lParam);

			// Get main_connection, if any
			obby::io::main_connection* conn =
				get_connection(c_socket);
			if(conn == NULL) return GDK_FILTER_REMOVE;
			const net6::socket& sock = conn->get_socket();

			// Check for error
			if(error != 0)
			{
				if(conn->get_events() &
				   obby::io::main_connection::IO_ERROR)
					sock.io_event().emit(
						net6::socket::IOERROR
					);

				return GDK_FILTER_REMOVE;
			}

			// No error occured. Check for events
			// which cause a net6::socket::INCOMING.
			if(event == FD_READ || event == FD_ACCEPT ||
			   event == FD_CLOSE)
			{
				try
				{
					// Read until the read call would block
					// which means that no more data is to
					// read. Only perform one read operation
					// in case of a close or accept event.
					do
					{
						// TODO: On connection loss, 0
						// bytes are read -> we run into
						// an endless loop.
						sock.io_event().emit(
							net6::socket::INCOMING
						);
					} while(event == FD_READ);
				}
				catch(net6::error& e)
				{
					// Ignore WOULD_BLOCK errors.
					if(e.get_code() !=
					   net6::error::WOULD_BLOCK)
						throw e;
				}
			}

			// Check for something to write.
			if(event == FD_WRITE)
			{
				try
				{
					// Write until the call would block
					// (which means that no more buffer
					// space is available for writing) or
					// until there is nothing to write
					// anymore.
					while(conn->get_events() &
					      obby::io::main_connection::IO_OUT)
					{
						sock.io_event().emit(
							net6::socket::OUTGOING
						);
					}
				}
				catch(net6::error& e)
				{
					// Ignore WOULD_BLOCK errors
					if(e.get_code() !=
					   net6::error::WOULD_BLOCK)
						throw e;
				}
			}

			return GDK_FILTER_REMOVE;
		}
		else
		{
			// No network event: Handle normally.
			return GDK_FILTER_CONTINUE;
		}
	}
}

obby::io::main_connection::main_connection(Gtk::Window& window,
                                           const net6::socket& sock,
                                           Condition condition)
 : m_window(window), m_socket(sock),
   m_condition(static_cast<main_connection::Condition>(0) )
{
	// Add filter if there is none set
	if(connmap.size() == 0)
		window.get_window()->add_filter(&on_filter, NULL);

	// Register object
	register_connection(*this);

	// Wait for events
	set_events(condition);
}

obby::io::main_connection::~main_connection()
{
	// Unregister object
	unregister_connection(*this);

	// Remove filter if it is not needed anymore
	if(connmap.size() == 0)
		m_window.get_window()->remove_filter(&on_filter, NULL);

	// Do not wait for anything any longer
	WSAAsyncSelect(
		m_socket.cobj(),
		hwnd_from_window(m_window),
		WM_NETWORK,
		0
	);

	// Do not do any error handling because we are running a destructor.
	// Throwing exceptions in destructors is evil.
}

void obby::io::main_connection::set_events(Condition condition)
{
	// Do only do something if we shall watch for other conditions
//	if(m_condition != condition)
	{
		// Build lEvent parameter for WSAAsyncSelect
		long levent = 0;

		if(condition & IO_IN) levent |= (FD_READ | FD_CLOSE);
		if(condition & IO_ACCEPT) levent |= FD_ACCEPT;
		if(condition & IO_OUT) levent |= FD_WRITE;

		// Save conditions we are currently watching for
		m_condition = condition;

		// Select for new conditions
		int result = WSAAsyncSelect(
			m_socket.cobj(),
			hwnd_from_window(m_window),
			WM_NETWORK,
			levent
		);

		if(result != 0)
			throw net6::error(net6::error::SYSTEM);
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
