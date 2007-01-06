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

#ifndef _GOBBY_BUFFER_WRAPPER_WIN32_HPP_
#define _GOBBY_BUFFER_WRAPPER_WIN32_HPP_

#include <sigc++/connection.h>
#include <glibmm/ustring.h>
#include <net6/address.hpp>
#include <net6/client.hpp>
#include <net6/host.hpp>

/** Win32 network wrappers. See comment in buffer_wrapper.hpp to understand
 * what's going on here.
 */

namespace Gobby
{

/** Object derived from net6::client using a timer which select()s periodically.
 */

class Client : public net6::client
{
public:
	/** Establishes a new client connection to the given host.
	 */
	Client(const net6::address& addr);
	virtual ~Client();

protected:
	/** Timer callback.
	 */
	virtual bool on_timer();

	/** Connection to Glib::signal_timeout().
	 */
	sigc::connection m_timer_connection;
};

/** Object derived from net6::host using a timer that calls select()
 * periodically.
 */

class Host : public net6::host
{
public:
        /** Constructs a new host object. Note that you have to perform a call
	 * to reopen() to accept incoming connections.
	 * @param username User name to use for the local user.
	 * @param ipv6 Whether to use IPv6.
	 */
	Host(const Glib::ustring& username, bool ipv6 = true);

	/** Constructs a new host object waiting for connections on the given
	 * port.
	 * @param port Port to wait for incoming connections.
	 * @param username User name for the local user
	 * @param ipv6 Whether to use IPv6
	 */
	Host(unsigned int port, const Glib::ustring& username,
	     bool ipv6 = true);

	virtual ~Host();

protected:
	/** Callback from Glib::signal_timeout().
	 */
	virtual bool on_timer();

	/** Connection to Glib::signal_timeout().
	 */
	sigc::connection m_timer_connection;

private:
};

}

#endif // _GOBBY_BUFFER_WRAPPER_WIN32_HPP_
