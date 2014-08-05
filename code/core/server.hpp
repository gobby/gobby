/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2014 Armin Burgmeier <armin@arbur.net>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
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
