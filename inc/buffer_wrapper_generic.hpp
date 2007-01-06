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

#ifndef _GOBBY_BUFFER_WRAPPER_GENERIC_HPP_
#define _GOBBY_BUFFER_WRAPPER_GENERIC_HPP_

#include <map>
#include <memory>
#include <sigc++/connection.h>
#include <sigc++/slot.h>
#include <glibmm/refptr.h>
#include <glibmm/iochannel.h>
#include <net6/non_copyable.hpp>
#include <net6/socket.hpp>
#include <net6/client.hpp>
#include <net6/host.hpp>

/** Non-win32 network wrappers. See comment in buffer_wrapper.hpp to understand
 * what's going on here.
 */

namespace Gobby
{

/** Class holding a connection to Glib::signal_io().
 */

class IOConnection : private net6::non_copyable
{
public:
	/** Slot type for Glib::signal_io().
	 */
	typedef sigc::slot<bool, Glib::IOCondition> slot_conn_type;

	/** Constructor. It builds an IOConnection for a given net6::socket.
	 * @param conn The net6::socket to watch for events.
	 * @param io_condition Conditions to watch for.
	 * @param io_func Slot to call if a condition has occured.
	 */
	IOConnection(const net6::socket& sock,
	             Glib::IOCondition io_condition);
	virtual ~IOConnection();

	/** Reconnects to Glib::signal_io() watching for the given conditions.
	 * If we are currently watching for exactle these conditions, no
	 * reconnect will be performed.
	 */
	void reconnect(Glib::IOCondition io_condition);

	/** reconnect()s with the current conditions plus the given ones.
	 */
	void add_connect(Glib::IOCondition io_condition);

	/** reconnect()s with the current conditions except the given ones.
	 */
	void remove_connect(Glib::IOCondition io_condition);

protected:
	virtual bool on_io(Glib::IOCondition condition);

	const net6::socket& m_sock;
	Glib::RefPtr<Glib::IOChannel> m_io_channel;
	Glib::IOCondition m_io_condition;
	sigc::connection m_io_connection;
};

/** Wrapper for net6::client. This is pretty easy: We have a Glib::IOChannel
 * we are watching for events, we store the conditions we are watching
 * (to decide when to reconnect with Glib::IO_OUT) and a sigc::connection
 * to Glib::signal_io() we have to reconnect.
 */
	
class Client : public net6::client
{
public:
	/** Establishes a new client connection to the given host.
	 */
	Client(const net6::address& addr);
	virtual ~Client();

	/** Sends a packet to the server.
	 */
	virtual void send(const net6::packet& pack);

protected:
	virtual void on_client_send(const net6::packet& pack);

	IOConnection m_ioconn;
};
	
/** The host is not as easy as the client. We have multiple connections which
 * we all have to watch for. We use a std::map<> to map the peer list on the
 * IOConnection objects.
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

	typedef std::map<const net6::host::peer*, IOConnection*> peer_map_type;

	/** Constructs a new host object. Note that you have to perform a call
	 * to reopen() to accept incoming connections.
	 * @param username User name to use for the local user.
	 * @param ipv6 Whether to use IPv6.
	 */
	Host(const Glib::ustring& username, bool ipv6 = true);

	/** Constructs a new host object waiting for connections on the given
	 * port.
	 * @param port Port to wait for incoming connections.
	 * @param username User name for the local user.
	 * @param ipv6 Whether to use IPv6.
	 */
	Host(unsigned int port, const Glib::ustring& username,
	     bool ipv6 = true);
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
	virtual void on_join(net6::host::peer& new_peer);
	virtual void on_client_send(const net6::packet& pack,
	                            net6::host::peer& to);
	virtual void remove_client(net6::host::peer* client);

	peer_map_type::iterator get_peer_iter(const net6::host::peer& peer);

	std::auto_ptr<IOConnection> m_serv_connection;
	peer_map_type m_peer_map;
private:
	void shutdown_impl();
	void reopen_impl(unsigned int port);
};

}

#endif // _GOBBY_BUFFER_WRAPPER_GENERIC_HPP_
