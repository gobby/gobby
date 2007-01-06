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
#include <sigc++/connection.h>
#include <glibmm/iochannel.h>
#include <net6/socket.hpp>

namespace Gobby
{

class GSelector: public sigc::trackable
{
public:
	struct SelectedSocket {
		sigc::connection conn;
		const net6::socket* sock;
		Glib::RefPtr<Glib::IOChannel> chan;
		net6::io_condition cond;
	};

	~GSelector();

	void add(const net6::socket& sock, net6::io_condition cond);
	void remove(const net6::socket& sock, net6::io_condition cond);
	void set(const net6::socket& sock, net6::io_condition cond);
	net6::io_condition check(const net6::socket& sock,
	                         net6::io_condition mask) const;

protected:
	bool on_io(Glib::IOCondition cond, const net6::socket* sock) const;

	typedef std::map<const net6::socket*, SelectedSocket> map_type;

	map_type m_map;
};

} // namespace Gobby

#endif // _GOBBY_GSELECTOR_HPP_
