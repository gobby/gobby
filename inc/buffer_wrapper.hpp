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

#ifndef _GOBBY_BUFFER_WRAPPER_HPP_
#define _GOBBY_BUFFER_WRAPPER_HPP_

/** The classes in this file are responsible for waking up the Glib main event
 * loop when network events are pending.
 */

#include <map>
#include <glibmm/ustring.h>
#include <glibmm/refptr.h>
#include <glibmm/iochannel.h>
#include <gtkmm/window.h>
#include <obby/client_buffer.hpp>
#include <obby/host_buffer.hpp>

namespace Gobby
{

/** An MainConnection is a connection between the Glib main event loop and the
 * net6 interface to send or receive data. Its implementation is platform-
 * dependant for we cannot use the Glib::signal_io() on the Windows platform.
 */
class MainConnection : private net6::non_copyable
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

	/** Constructor building a MainConnection for a given socket.
	 * @param window The main GtkWindow
	 * @param sock The socket to watch for events.
	 * @param condition Conditions to initially watch for.
	 */
	MainConnection(Gtk::Window& window, const net6::socket& sock,
	               Condition condition);
	virtual ~MainConnection();

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

	/** Returns the window this MainConnection is associated with.
	 */
	const Gtk::Window& get_window() const { return m_window; }

	/** Returns the socket this MainConnection is associated with.
	 */
	const net6::socket& get_socket() const { return m_socket; }

protected:
	Gtk::Window& m_window;
	const net6::socket& m_socket;
	Condition m_condition;

#ifdef WIN32
#else
	Glib::RefPtr<Glib::IOChannel> m_channel;
	sigc::connection m_connection;
#endif
};

/** Bitwise connection operator definitions for MainConnection::Condition.
 */

inline MainConnection::Condition operator|(
	MainConnection::Condition rhs, MainConnection::Condition lhs
) {
	return static_cast<MainConnection::Condition>(
		static_cast<int>(rhs) | static_cast<int>(lhs)
	);
}

inline MainConnection::Condition operator&(
	MainConnection::Condition rhs, MainConnection::Condition lhs
) {
	return static_cast<MainConnection::Condition>(
		static_cast<int>(rhs) & static_cast<int>(lhs)
	);
}

inline MainConnection::Condition operator^(
	MainConnection::Condition rhs, MainConnection::Condition lhs
) {
	return static_cast<MainConnection::Condition>(
		static_cast<int>(rhs) ^ static_cast<int>(lhs)
	);
}

inline MainConnection::Condition operator|=(
	MainConnection::Condition& rhs, MainConnection::Condition lhs
) {
	rhs = (rhs | lhs);
}

inline MainConnection::Condition operator&=(
	MainConnection::Condition& rhs, MainConnection::Condition lhs
) {
	rhs = (rhs & lhs);
}

inline MainConnection::Condition operator^=(
	MainConnection::Condition& rhs, MainConnection::Condition lhs
) {
	rhs = (rhs ^ lhs);
}

inline MainConnection::Condition operator~(MainConnection::Condition rhs) {
	return static_cast<MainConnection::Condition>(~static_cast<int>(rhs) );
}

/** Wrapper for net6::client. This is pretty easy: We have a MainConnection that
 * watches for data to be read. If something has to be sent, we set the
 * MainConnection::IO_OUT flag as well and remove it, if all data has been
 * written.
 */

class Client : public net6::client
{
public:
	/** Establishes a new client connection to the given host.
	 */
	Client(Gtk::Window& window, const net6::address& addr);
	virtual ~Client();

	/** Sends a packet to the server.
	 */
	virtual void send(const net6::packet& pack);

protected:
	virtual void on_send_event(const net6::packet& pack);

	MainConnection m_ioconn;
};

/** The host is not as easy as the client. We have multiple connections which
 * we all have to watch for. We use a std::map<> to map the peer list to the
 * MainConnection objects.
 */

class Host : public net6::host
{
public:
	/** Error class for Host errors.
	 */
	class Error : public Glib::Error
	{
	public:
		enum Code {
			PEER_NOT_FOUND
		};

		Error(Code error_code, const Glib::ustring& error_message);
		Code code() const;
	};

	typedef std::map<const net6::host::peer*, MainConnection*>
		peer_map_type;

	/** Constructs a new host object. Note that you have to perform a call
	 * to reopen() to accept incoming connections.
	 * @param username User name to use for the local user.
	 * @param ipv6 Whether to use IPv6.
	 */
	Host(Gtk::Window& window, const Glib::ustring& username,
	     bool ipv6 = true);

	/** Constructs a new host object waiting for connections on the given
	 * port.
	 * @param port Port to wait for incoming connections.
	 * @param username User name for the local user.
	 * @param ipv6 Whether to use IPv6.
	 */
	Host(Gtk::Window& window, unsigned int port,
	     const Glib::ustring& username, bool ipv6 = true);
	virtual ~Host();

	/** Closes the server socket. No more connections will be accepted.
	 */
	virtual void shutdown();

	/** (Re)opens the server socket on the given port.
	 */
	virtual void reopen(unsigned int port);

	/** Sends a packet to the given peer. No packet will be send if to
	 * is the local user.
	 */
	virtual void send(const net6::packet& pack, net6::host::peer& to);

protected:
	/** Called when a new peer has connected to the server. This callback
	 * is used to create a new IOConnection for this peer and storing it
	 * in the peer map.
	 */
	virtual void on_connect(net6::host::peer& new_peer);

	/** Called when a packet has been sent to a client. The IO_OUT flag
	 * will be removed if no packets follow.
	 */
	virtual void on_send_event(const net6::packet& pack,
	                           net6::host::peer& to);

	/** Deletes the IOConnection on connection loss.
	 */
	virtual void remove_client(net6::host::peer* client);

	/** Returns the iterator for the given peer. If this peer is not present
	 * in the map (which should never occur), Host::Error is thrown.
	 */
	peer_map_type::iterator get_peer_iter(const net6::host::peer& peer);

	Gtk::Window& m_window;
	std::auto_ptr<MainConnection> m_serv_connection;
	peer_map_type m_peer_map;
private:
	void shutdown_impl();
	void reopen_impl(unsigned int port);
};

/** A obby::client_buffer derived class that uses Gobby::Client.
 */

class ClientBuffer : public obby::client_buffer
{
public:
	ClientBuffer(Gtk::Window& window, const Glib::ustring& hostname,
	             unsigned int port);
	virtual ~ClientBuffer();

protected:
};

/** A obby::host_buffer derived class that uses Gobby::Host.
 */

class HostBuffer : public obby::host_buffer
{
public:
	HostBuffer(Gtk::Window& window, unsigned int port,
	           const Glib::ustring& username, int red, int green, int blue);
	virtual ~HostBuffer();

protected:
	HostBuffer();
};

}

#endif // _GOBBY_BUFFER_WRAPPER_HPP_
