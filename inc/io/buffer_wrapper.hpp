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

#ifndef _OBBY_IO_BUFFER_WRAPPER_HPP_
#define _OBBY_IO_BUFFER_WRAPPER_HPP_

#include <map>
#include <glibmm/ustring.h>
#ifdef WIN32
# include <gtkmm/window.h>
#endif
#include <obby/client_buffer.hpp>
#include <obby/server_buffer.hpp>
#include <obby/host_buffer.hpp>
#include "io/main_connection.hpp"

namespace obby
{

namespace io
{

/** Wrapper for net6::client. This is pretty easy: We have a main_connection that
 * watches for data to be read. If something has to be sent, we set the
 * main_connection::IO_OUT flag as well and remove it, if all data has been
 * written.
 */

class client : virtual public net6::client
{
public:
	/** Establishes a new client connection to the given host.
	 */
#ifdef WIN32
	client(Gtk::Window& window, const net6::address& addr);
#else
	client(const net6::address& addr);
#endif
	virtual ~client();

	/** Sends a packet to the server.
	 */
	virtual void send(const net6::packet& pack);

protected:
	virtual void on_send_event();

	main_connection m_ioconn;
};

/** The server manages a std::map<> from a peer to its main_connection.
 * There is one additional main_connection for the server socket.
 */

class server : virtual public net6::server
{
public:
	/** Error class for server errors.
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

	typedef std::map<const net6::user*, main_connection*>
		user_map_type;

	/** Constructs a new server object. Note that you have to perform a call
	 * to reopen() to accept incoming connections.
	 * @param ipv6 Whether to use IPv6.
	 */
#ifdef WIN32
	server(Gtk::Window& window, bool ipv6 = true);
#else
	server(bool ipv6 = true);
#endif

	/** Constructs a new server object waiting for connections on the given
	 * port.
	 * @param port Port to wait for incoming connections.
	 * @param ipv6 Whether to use IPv6.
	 */
#ifdef WIN32
	server(Gtk::Window& window, unsigned int port, bool ipv6 = true);
#else
	server(unsigned int port, bool ipv6 = true);
#endif
	virtual ~server();

	/** Closes the server socket. No more connections will be accepted.
	 */
	virtual void shutdown();

	/** (Re)opens the server socket on the given port.
	 */
	virtual void reopen(unsigned int port);

	/** Sends a packet to all currently connected users.
	 */
	virtual void send(const net6::packet& pack);

	/** Sends a packet to the given user.
	 */
	virtual void send(const net6::packet& pack, const net6::user& to);

protected:
	/** Called when a new user has connected to the server. This callback
	 * is used to create a new main_connection for this user and storing it
	 * in the user map.
	 */
	virtual void on_connect(const net6::user& new_user);

	/** Called when a all data in the send queue of a client connection has
	 * been sent. This is used to remove the IO_OUT flag.
	 */
	virtual void on_send_event(net6::user& to);

	/** Deletes the main_connection on connection loss.
	 */
	virtual void remove_client(const net6::user* client);

	/** Returns the iterator for the given user. If this user is not present
	 * in the map (which should never occur), server::Error is thrown.
	 */
	user_map_type::iterator get_user_iter(const net6::user& user);

#ifdef WIN32
	Gtk::Window& m_window;
#endif
	std::auto_ptr<main_connection> m_serv_connection;
	user_map_type m_user_map;
private:
	void shutdown_impl();
	void reopen_impl(unsigned int port);
};

/** The host is bit tricky:
 * We derive from io::server to get all the IO handling and from net6::host to
 * get the underlaying host with its local user.
 */

class host : virtual public net6::host,
             virtual public server
{
public:
#ifdef WIN32
	/** Creates a new host object. The local user will be named
	 * <em>username</em>.
	 */
	host(Gtk::Window& window, const Glib::ustring& username,
	     bool ipv6 = true);
	/** Creates a new host object and opens it on port <em>port</em>.
	 * The local user will be named <em>username</em>.
	 */
	host(Gtk::Window& window, unsigned int port,
	     const Glib::ustring& username, bool ipv6 = true);
#else
	/** Creates a new host object. The local user will be named
	 * <em>username</em>.
	 */
	host(const Glib::ustring& username, bool ipv6 = true);
	/** Creates a new host object and opens it on port <em>port</em>.
	 * The local user will be named <em>username</em>.
	 */
	host(unsigned int port, const Glib::ustring& username,
	     bool ipv6 = true);
#endif
	virtual ~host();

	/** Sends a packet to all connected users.
	 */
	virtual void send(const net6::packet& pack);

	/** Sends a packet to the given user. No packet will be sent if to
	 * is the local user.
	 */
	virtual void send(const net6::packet& pack, const net6::user& to);
protected:
	/** Called when a all data in the send queue of a client connection has
	 * been sent. This is used to remove the IO_OUT flag.
	 */
	virtual void on_send_event(net6::user& to);
};

/** A obby::client_buffer derived class that uses io::client.
 */

class client_buffer : virtual public obby::client_buffer
{
public:
#ifdef WIN32
	client_buffer(Gtk::Window& window, const Glib::ustring& hostname,
	             unsigned int port);
#else
	client_buffer(const Glib::ustring& hostname, unsigned int port);
#endif
	virtual ~client_buffer();

protected:
};

/** A obby::server_buffer derived class that uses io::server.
 */
class server_buffer : virtual public obby::server_buffer
{
public:
#ifdef WIN32
	server_buffer(Gtk::Window& window, unsigned int port);
#else
	server_buffer(unsigned int port);
#endif
	virtual ~server_buffer();

protected:
	server_buffer();
};

/** A obby::host_buffer derived class that uses io::host.
 */

class host_buffer : virtual public obby::host_buffer
{
public:
#ifdef WIN32
	host_buffer(Gtk::Window& window, unsigned int port,
	           const Glib::ustring& username, int red, int green, int blue);
#else
	host_buffer(unsigned int port, const Glib::ustring& username, int red,
	            int green, int blue);
#endif
	virtual ~host_buffer();

protected:
	host_buffer();
};

} // namespace io

} // namespace obby

#endif // _OBBY_IO_BUFFER_WRAPPER_HPP_
