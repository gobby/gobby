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
}

Gobby::GSelector::~GSelector()
{
	// Should already be performed by sigc::trackable...
	for(map_type::iterator it = m_map.begin(); it != m_map.end(); ++ it)
		it->second.conn.disconnect();
}

net6::io_condition Gobby::GSelector::get(const net6::socket& sock) const
{
	map_type::const_iterator iter = m_map.find(&sock);

	if(iter == m_map.end() )
		return net6::IO_NONE;
	else
		return iter->second.cond;
}

void Gobby::GSelector::add_socket(const net6::socket& sock,
                                  net6::io_condition cond)
{
	SelectedSocket& sel = m_map[&sock];

	net6::socket::socket_type fd = sock.cobj();

	sel.chan =
#ifdef _WIN32
		Glib::IOChannel::create_from_win32_socket(fd);
#else
		Glib::IOChannel::create_from_fd(fd);
#endif

	sel.conn = Glib::signal_io().connect(
		sigc::bind(
			sigc::mem_fun(*this, &GSelector::on_io),
			&sock
		),
		sel.chan,
		gcond(cond)
	);

	sel.sock = &sock;
	sel.cond = cond;
}

void Gobby::GSelector::modify_socket(map_type::iterator iter,
                                     net6::io_condition cond)
{
	// Flags are already set
	if(iter->second.cond == cond)
		return;

	iter->second.cond = cond;

	iter->second.conn.disconnect();
	iter->second.conn = Glib::signal_io().connect(
		sigc::bind(
			sigc::mem_fun(*this, &GSelector::on_io),
			iter->first
		),
		iter->second.chan,
		gcond(iter->second.cond)
	);
}

void Gobby::GSelector::delete_socket(map_type::iterator iter)
{
	iter->second.conn.disconnect();
	m_map.erase(iter);
}

void Gobby::GSelector::set(const net6::socket& sock, net6::io_condition cond)
{
	map_type::iterator iter = m_map.find(&sock);

	if(cond != net6::IO_NONE)
		if(iter == m_map.end() )
			add_socket(sock, cond);
		else
			modify_socket(iter, cond);
	else if(iter != m_map.end() )
		delete_socket(iter);
}

bool Gobby::GSelector::on_io(Glib::IOCondition cond,
                             const net6::socket* sock) const
{
	map_type::const_iterator iter = m_map.find(sock);

	// Has been removed by previous handler
	if(iter == m_map.end() ) return false;

	// Occured condition has been removed by previous handler
	if( (gcond(iter->second.cond) & cond) == gcond(net6::IO_NONE))
		return true;

	sock->io_event().emit(ncond(cond) );
	return true;
}
