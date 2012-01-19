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

#include <glibmm/main.h>

#include "gselector.hpp"

namespace
{
	inline Glib::IOCondition gcond(net6::io_condition cond)
	{
		Glib::IOCondition g_cond = Glib::IOCondition(0);
		if(cond & net6::IO_INCOMING)
			g_cond |= Glib::IO_IN;
		if(cond & net6::IO_OUTGOING)
			g_cond |= Glib::IO_OUT;
		if(cond & net6::IO_ERROR)
			g_cond |= (Glib::IO_HUP | Glib::IO_NVAL | Glib::IO_ERR);
		return g_cond;
	}

	inline net6::io_condition ncond(Glib::IOCondition cond)
	{
		net6::io_condition n_cond = net6::IO_NONE;
		if(cond & Glib::IO_IN)
			n_cond |= net6::IO_INCOMING;
		if(cond & Glib::IO_OUT)
			n_cond |= net6::IO_OUTGOING;
		if(cond & (Glib::IO_HUP | Glib::IO_NVAL | Glib::IO_ERR) )
			n_cond |= net6::IO_ERROR;
		return n_cond;
	}

	net6::io_condition IO_FLAGS =
		net6::IO_INCOMING | net6::IO_OUTGOING | net6::IO_ERROR;

#ifdef _WIN32
	bool win32_idle_func() { return false; }
#endif
}

Gobby::GSelector::GSelector():
	m_mutex(new Glib::RecMutex)
{
}

Gobby::GSelector::~GSelector()
{
	// Should already be performed by sigc::trackable...
	for(map_type::iterator it = m_map.begin(); it != m_map.end(); ++ it)
		it->second.io_conn.disconnect();
}

void Gobby::GSelector::add_socket(const net6::socket& sock,
                                  net6::io_condition cond)
{
	SelectedSocket& sel = m_map[&sock];

	sel.sock = &sock;
	sel.cond = cond;

	// Timeout is set in set_timeout
	if( (cond & IO_FLAGS) != net6::IO_NONE)
	{
		net6::socket::socket_type fd = sock.cobj();

		sel.io_chan =
#ifdef _WIN32
			Glib::IOChannel::create_from_win32_socket(fd);
#else
			Glib::IOChannel::create_from_fd(fd);
#endif

		sel.io_conn = Glib::signal_io().connect(
			sigc::bind(
				sigc::mem_fun(*this, &GSelector::on_io),
				&sock
			),
			sel.io_chan,
			gcond(cond)
		);
	}
}

void Gobby::GSelector::modify_socket(map_type::iterator iter,
                                     net6::io_condition cond)
{
	// IO_FLAGS did change
	if( (iter->second.cond & IO_FLAGS) != (cond & IO_FLAGS) )
	{
		if(iter->second.io_conn.connected() )
			iter->second.io_conn.disconnect();

		if( (cond & IO_FLAGS) != net6::IO_NONE)
		{
			iter->second.io_conn = Glib::signal_io().connect(
				sigc::bind(
					sigc::mem_fun(*this, &GSelector::on_io),
					iter->first
				),
				iter->second.io_chan,
				gcond(cond)
			);
		}
	}

	// IO_TIMEOUT changed
	if( (iter->second.cond & net6::IO_TIMEOUT) != (cond & net6::IO_TIMEOUT))
	{
		if(iter->second.time_conn.connected() )
			iter->second.time_conn.disconnect();

		// Timeout is set in set_timeout
	}

	iter->second.cond = cond;
}

void Gobby::GSelector::delete_socket(map_type::iterator iter)
{
	if(iter->second.io_conn.connected() )
		iter->second.io_conn.disconnect();
	if(iter->second.time_conn.connected() )
		iter->second.time_conn.disconnect();

	m_map.erase(iter);
}

net6::io_condition Gobby::GSelector::get(const net6::socket& sock) const
{
	Glib::RecMutex::Lock lock(*m_mutex);
	map_type::const_iterator iter = m_map.find(&sock);

	if(iter == m_map.end() )
		return net6::IO_NONE;
	else
		return iter->second.cond;
}

void Gobby::GSelector::set(const net6::socket& sock, net6::io_condition cond)
{
	// Lock mutex - required for connection establishment which happens
	// in a different thread for the GUI to remain responsive.

	// After the connection to Glib::signal_io() the main thread may be
	// woken up immediately by incoming data and call GSelector::set to
	// send out some data even before the assignment to the
	// sigc::connection in the connecting thread has been finished!
	Glib::RecMutex::Lock lock(*m_mutex);

	map_type::iterator iter = m_map.find(&sock);

	if(cond != net6::IO_NONE)
	{
		if(iter == m_map.end() )
			add_socket(sock, cond);
		else
			modify_socket(iter, cond);
	}
	else if(iter != m_map.end() )
	{
		delete_socket(iter);
	}
}

unsigned long Gobby::GSelector::get_timeout(const net6::socket& sock) const
{
	Glib::RecMutex::Lock lock(*m_mutex);
	map_type::const_iterator iter = m_map.find(&sock);

	// No timeout set for this socket
	if(iter == m_map.end() ) return 0;
	if(!iter->second.time_conn.connected() ) return 0;

	// Returns the remaining time for the timeout to be elapsed
	Glib::TimeVal val;
	val.assign_current_time();
	val -= iter->second.timeout_begin;

	unsigned long elapsed = (val.tv_sec * 1000 + val.tv_usec / 1000);
	if(elapsed >= iter->second.timeout) return 1;

	return iter->second.timeout - elapsed;
}

void Gobby::GSelector::set_timeout(const net6::socket& sock,
                                   unsigned long timeout)
{
	Glib::RecMutex::Lock lock(*m_mutex);

	SelectedSocket* sel_sock = NULL;
	map_type::iterator iter = m_map.find(&sock);

	if(iter != m_map.end() )
	{
		if( (iter->second.cond & net6::IO_TIMEOUT) == net6::IO_TIMEOUT)
			sel_sock = &iter->second;
	}

	if(sel_sock == NULL)
	{
		throw std::logic_error(
			"Gobby::GSelector::set_timeout:\n"
			"Socket is not selected of IO_TIMEOUT"
		);
	}

	if(sel_sock->time_conn.connected() )
		sel_sock->time_conn.disconnect();

	sel_sock->timeout_begin.assign_current_time();
	sel_sock->timeout = timeout;
	sel_sock->time_conn = Glib::signal_timeout().connect(
		sigc::bind(
			sigc::mem_fun(*this, &GSelector::on_timeout),
			sel_sock->sock
		),
		timeout
	);
}

bool Gobby::GSelector::on_io(Glib::IOCondition cond,
                             const net6::socket* sock)
{
	{
		Glib::RecMutex::Lock lock(*m_mutex);
		map_type::const_iterator iter = m_map.find(sock);

		// Has been removed by previous handler
		if(iter == m_map.end() ) return false;

		// Occured condition has been removed by previous handler
		if( (gcond(iter->second.cond) & cond) == gcond(net6::IO_NONE))
			return true;
	}

	// Event handler may destroy the selector, so do not reference
	// m_mutex anymore.
	sock->io_event().emit(ncond(cond) );
	return true;
}

bool Gobby::GSelector::on_timeout(const net6::socket* sock)
{
	{
		Glib::RecMutex::Lock lock(*m_mutex);
		map_type::const_iterator iter = m_map.find(sock);

		// Quite impossible... TODO: throw logic error?
		if(iter == m_map.end() ) return false;
		if( (iter->second.cond & net6::IO_TIMEOUT) == net6::IO_NONE)
			return false;
	}

#ifdef _WIN32
	// When the timeout event handler sends data (like net6_ping), glib
	// does not emit a signal_io (with Glib::IO_OUT) until another event
	// occured that wakes up the main loop. This idle event is this other
	// event. Seems to be a bug in Glib/Win32 however.
	Glib::signal_idle().connect(sigc::ptr_fun(win32_idle_func) );
#endif

	// Event handler may destroy the selector, so do not reference
	// m_mutex anymore.
	sock->io_event().emit(net6::IO_TIMEOUT);

	// Timeout is removed after execution
	return false;
}
