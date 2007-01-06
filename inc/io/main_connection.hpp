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

#ifndef _OBBY_IO_MAIN_CONNECTION_HPP_
#define _OBBY_IO_MAIN_CONNECTION_HPP_

#include <glibmm/ustring.h>
#include <glibmm/refptr.h>
#include <glibmm/iochannel.h>
#include <net6/non_copyable.hpp>
#include <net6/socket.hpp>

namespace obby
{

/** This is the IO namespace of obby. These classes connect the net6 socket
 * functions with the Glib main event loop to wake up the GUI when some
 * network events occur. Note that all the functions require a Gtk::Window&
 * on WIN32 to get the HWND for a WSAAsyncSelect call. This means that obby::io
 * cannot be used for non-GTK programs when compiled on WIN32 (at least with
 * Glib-2.6. The unix-API of obby::io should work with WIN32 with glib >= 2.7).
 */

namespace io
{

/** A main_connection is a connection between the Glib main event loop and the
 * net6 interface to send or receive data. Its implementation is platform-
 * dependant for we cannot use the Glib::signal_io() on the Windows platform.
 */
class main_connection : private net6::non_copyable
{
public:
	/** Possible conditions to watch for.
	 */
	enum Condition {
		IO_IN = 0x01,
		IO_ACCEPT = 0x02,
		IO_OUT = 0x04,
		IO_ERROR = 0x08
	};

	/** Constructor building a main_connection for a given socket.
	 * @param sock The socket to watch for events.
	 * @param condition Conditions to initially watch for.
	 */
#ifdef WIN32
	main_connection(Gtk::Window& window, const net6::socket& sock,
	               Condition condition);
#else
	main_connection(const net6::socket& sock, Condition condition);
#endif
	virtual ~main_connection();

	/** Changes the events to watch for.
	 */
	void set_events(Condition condition);

	/** Adds new events to watch for.
	 */
	void add_events(Condition condition);

	/** Removes existing events to watch for.
	 */
	void remove_events(Condition condition);

	/** Returns the events that are currently watched for.
	 */
	Condition get_events() const { return m_condition; }

#ifdef WIN32
	/** Returns the window this main_connection is associated with.
	 */
	const Gtk::Window& get_window() const { return m_window; }
#endif

	/** Returns the socket this main_connection is associated with.
	 */
	const net6::socket& get_socket() const { return m_socket; }

protected:
#ifdef WIN32
	Gtk::Window& m_window;
#endif
	const net6::socket& m_socket;
	Condition m_condition;

#ifdef WIN32
#else
	Glib::RefPtr<Glib::IOChannel> m_channel;
	sigc::connection m_connection;
#endif
};

/** Bitwise connection operator definitions for main_connection::Condition.
 */

inline main_connection::Condition operator|(
	main_connection::Condition rhs, main_connection::Condition lhs
) {
	return static_cast<main_connection::Condition>(
		static_cast<int>(rhs) | static_cast<int>(lhs)
	);
}

inline main_connection::Condition operator&(
	main_connection::Condition rhs, main_connection::Condition lhs
) {
	return static_cast<main_connection::Condition>(
		static_cast<int>(rhs) & static_cast<int>(lhs)
	);
}

inline main_connection::Condition operator^(
	main_connection::Condition rhs, main_connection::Condition lhs
) {
	return static_cast<main_connection::Condition>(
		static_cast<int>(rhs) ^ static_cast<int>(lhs)
	);
}

inline main_connection::Condition operator|=(
	main_connection::Condition& rhs, main_connection::Condition lhs
) {
	rhs = (rhs | lhs);
}

inline main_connection::Condition operator&=(
	main_connection::Condition& rhs, main_connection::Condition lhs
) {
	rhs = (rhs & lhs);
}

inline main_connection::Condition operator^=(
	main_connection::Condition& rhs, main_connection::Condition lhs
) {
	rhs = (rhs ^ lhs);
}

inline main_connection::Condition operator~(main_connection::Condition rhs) {
	return static_cast<main_connection::Condition>(~static_cast<int>(rhs) );
}

} // namespace io

} // namespace obby

#endif // _OBBY_IO_MAIN_CONNECTION_HPP_

