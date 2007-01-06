/* gobby - A GTKmm driven libobby client
 * Copyright (C) 2005 0x539 dev group
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * x
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

#ifndef _GOBBY_BUFFER_WRAPPER_HPP_
#define _GOBBY_BUFFER_WRAPPER_HPP_

/** This file is just a wrapper for including the system-dependant header file
 * for the network wrappers. The classes in these files are responsible for
 * waking up the Glib main event loop when network events are pending.
 *
 * The approach on UNIX-like systems is, to connect to Glib::signal_io() instead
 * of using the net6::selector. On Windows, this doesn't work. Reconnecting
 * to Glib::signal_io() to set or unset the Glib::IO_OUT flag seems to remove
 * the signal connection without establishing a new one. Therefore, we try the
 * following:
 *
 * We start a new Thread which blocks in a net6::select(). If some events
 * arrive we send a notification through a UDP socket to the main event loop.
 * Then, we wait until the main event loop has processed these events and
 * we fall back into the select(). If we want to quit, we send a message
 * through the UDP socket that cancells the select() call. Note that we cannot
 * use pipes for select() on windows works only for sockets since sockets are
 * no file descriptors.
 */

#include <glibmm/ustring.h>
#include <libobby/client_buffer.hpp>
#include <libobby/host_buffer.hpp>

#ifndef WIN32
# include "buffer_wrapper_win32.hpp"
#else
# include "buffer_wrapper_generic.hpp"
#endif

namespace Gobby
{

/** A obby::client_buffer derived class that uses Gobby::Client
 * from buffer_wrapper_win32.hpp on buffer_wrapper_generic.hpp.
 */

class ClientBuffer : public obby::client_buffer
{
public:
	ClientBuffer(const Glib::ustring& hostname, unsigned int port);
	virtual ~ClientBuffer();

protected:
	ClientBuffer();
};

/** A obby::host_buffer derived class that uses Gobby::Host
 * from buffer_wrapper_win32.hpp or buffer_wrapper_generic.hpp.
 */

class HostBuffer : public obby::host_buffer
{
public:
	HostBuffer(unsigned int port, const Glib::ustring& username, int red,
	           int green, int blue);
	virtual ~HostBuffer();

protected:
	HostBuffer();
};

}

#endif // _GOBBY_BUFFER_WRAPPER_HPP_
