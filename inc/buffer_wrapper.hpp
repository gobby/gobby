/* gobby - A GTKmm driven libobby client
 * Copyright (C) 2005 0x539 dev group
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef _GOBBY_BUFFER_WRAPPER_HPP_
#define _GOBBY_BUFFER_WRAPPER_HPP_

#include <map>
#include <sigc++/connection.h>
#include <glibmm/refptr.h>
#include <glibmm/iochannel.h>
#include <net6/packet.hpp>
#include <net6/client.hpp>
#include <net6/host.hpp>
#include <libobby/client_buffer.hpp>
#include <libobby/host_buffer.hpp>

/** The classes in this file inherit from corresponding net6 and obby classes
 * but instead of using the net6 selector they use Glib::signal_io() which
 * works with the Glib Main event loop.
 */
	
namespace Gobby
{

/** Client: Just use a Glib::IOChannel for the connection and an IOCondition
 * to store the conditions we are currently watching for.
 */
	
class Client : public net6::client
{
public:
	Client(const net6::address& addr);
	virtual ~Client();

	virtual void send(const net6::packet& pack);

protected:
	virtual void on_client_send(const net6::packet& pack);

	virtual bool on_io(Glib::IOCondition condition);

	Glib::RefPtr<Glib::IOChannel> m_io_channel;
	Glib::IOCondition m_io_condition;
	sigc::connection m_io_connection;
};
	
class ClientBuffer : public obby::client_buffer
{
public:
	ClientBuffer(const Glib::ustring& hostname, unsigned int port);
	virtual ~ClientBuffer();

protected:
	ClientBuffer();
};

/** Host: It is not as easy as with the client because we have a multiple
 * amount of connections. We store the IOChannel and the IOCondition for
 * each client in a struct called peer_data and use a std::map to associate
 * a peer with its data. Additionally, an IOChannel is hold for the server
 * socket to accept incoming connections.
 */

class Host : public net6::host
{
public:
	Host(const Glib::ustring& username, bool ipv6 = true);
	Host(unsigned int port, const Glib::ustring& username,
	     bool ipv6 = true);
	virtual ~Host();

	virtual void shutdown();
	virtual void reopen(unsigned int port);

	virtual void send(const net6::packet& pack, net6::host::peer& to);

protected:
	struct peer_data
	{
		Glib::RefPtr<Glib::IOChannel> io_channel;
		Glib::IOCondition io_condition;
		sigc::connection io_connection;
	};

	virtual void on_join(net6::host::peer& new_peer);
	virtual void on_client_send(const net6::packet& pack,
	                            net6::host::peer& from);
	virtual void remove_client(net6::host::peer* client);

	virtual bool on_server_io(Glib::IOCondition condition);
	virtual bool on_io(Glib::IOCondition condition, net6::host::peer* peer);

	Glib::RefPtr<Glib::IOChannel> m_io_channel;
	sigc::connection m_io_connection;
	std::map<net6::host::peer*, peer_data*> m_peer_map;
private:
	void shutdown_impl();
	void reopen_impl(unsigned int port);
};

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
