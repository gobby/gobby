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

#ifndef _GOBBY_GSELECTOR_HPP_
#define _GOBBY_GSELECTOR_HPP_

#include <map>
#include <memory>
#include <sigc++/connection.h>
#include <glibmm/iochannel.h>
#include <glibmm/thread.h>
#include <net6/socket.hpp>

namespace Gobby
{

class GSelector: private net6::non_copyable, public sigc::trackable
{
public:
	struct SelectedSocket {
		const net6::socket* sock;
		net6::io_condition cond;

		Glib::RefPtr<Glib::IOChannel> io_chan;
		sigc::connection io_conn;

		Glib::TimeVal timeout_begin;
		unsigned long timeout;
		sigc::connection time_conn;
	};

	GSelector();
	~GSelector();

	net6::io_condition get(const net6::socket& sock) const;
	void set(const net6::socket& sock, net6::io_condition cond);

	unsigned long get_timeout(const net6::socket& sock) const;
	void set_timeout(const net6::socket& sock, unsigned long timeout);
protected:
	typedef std::map<const net6::socket*, SelectedSocket> map_type;

	void add_socket(const net6::socket& sock, net6::io_condition cond);
	void modify_socket(map_type::iterator iter, net6::io_condition cond);
	void delete_socket(map_type::iterator iter);

	bool on_io(Glib::IOCondition cond, const net6::socket* sock);
	bool on_timeout(const net6::socket* sock);

	map_type m_map;

	// Is a auto ptr to allow locking in const get function
	std::auto_ptr<Glib::RecMutex> m_mutex;
};

} // namespace Gobby

#endif // _GOBBY_GSELECTOR_HPP_
