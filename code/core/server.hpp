/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2014 Armin Burgmeier <armin@arbur.net>
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
 * Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#ifndef _GOBBY_SERVER_HPP_
#define _GOBBY_SERVER_HPP_

#include <libinfinity/server/infd-xmpp-server.h>
#include <libinfinity/server/infd-server-pool.h>
#include <libinfinity/common/inf-local-publisher.h>
#include <libinfinity/common/inf-io.h>

namespace Gobby
{

class Server
{
public:
	Server(InfIo* io, InfLocalPublisher* publisher);
	~Server();

	// Can throw; can be used on already open servers:
	void open(unsigned int port,
	          InfXmppConnectionSecurityPolicy security_policy,
	          InfCertificateCredentials* creds,
	          InfSaslContext* sasl_context,
	          const char* sasl_mechanisms);

	void close();

	bool is_open() const;

	unsigned int get_port() const;

	// Use new credentials on running server, for new connections:
	void set_credentials(InfXmppConnectionSecurityPolicy security_policy,
	                     InfCertificateCredentials* credentials);

	// Set SASL context, for new connections.
	void set_sasl_context(InfSaslContext* sasl_context,
	                      const char* m_sasl_mechanisms);

	// Set a server pool to which to add the servers, and which to
	// use for publishing with the publisher.
	void set_pool(InfdServerPool* pool);

protected:
	InfIo* m_io;
	InfLocalPublisher* m_publisher;

	InfdXmppServer* m_xmpp4;
	InfdXmppServer* m_xmpp6;
	InfdServerPool* m_pool;
};

}
	
#endif // _GOBBY_SERVER_HPP_
