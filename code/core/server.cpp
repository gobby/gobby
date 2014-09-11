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

#include "core/selfhoster.hpp"
#include "util/file.hpp"
#include "util/i18n.hpp"

#include <libinfinity/common/inf-cert-util.h>
#include <libinfinity/common/inf-error.h>

#include <gnutls/x509.h>

#include <cassert>

Gobby::Server::Server(InfIo* io,
                      InfLocalPublisher* publisher):
	m_io(io), m_publisher(publisher),
	m_xmpp4(NULL), m_xmpp6(NULL), m_pool(NULL)
{
	g_object_ref(m_io);
}

Gobby::Server::~Server()
{
	if(is_open()) close();
	set_pool(NULL);

	g_object_unref(m_io);
}

void Gobby::Server::open(unsigned int port,
                         const InfKeepalive* keepalive,
                         InfXmppConnectionSecurityPolicy security_policy,
                         InfCertificateCredentials* creds,
                         InfSaslContext* context,
                         const char* sasl_mechanisms)
{
	// If we can open one of tcp4 or tcp6 that's a success.
	InfdTcpServer* tcp4;
	InfdTcpServer* tcp6;

	// If the server is already open and we do not need to change the
	// port, then just change the credentials and SASL context without
	// doing anything else.
	if(is_open() && get_port() == port)
	{
		set_keepalive(keepalive);
		set_credentials(security_policy, creds);
		set_sasl_context(context, sasl_mechanisms);
		return;
	}

	static const guint8 ANY6_ADDR[16] =
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	InfIpAddress* any6 = inf_ip_address_new_raw6(ANY6_ADDR);

	tcp4 = INFD_TCP_SERVER(g_object_new(
		INFD_TYPE_TCP_SERVER,
		"io", m_io, "local-address", NULL,
		"local-port", port,
		NULL));
	tcp6 = INFD_TCP_SERVER(g_object_new(
		INFD_TYPE_TCP_SERVER,
		"io", m_io, "local-address", any6,
		"local-port", port,
		NULL));
	inf_ip_address_free(any6);

	if(!infd_tcp_server_open(tcp6, NULL))
	{
		g_object_unref(tcp6);
		tcp6 = NULL;

		GError* error = NULL;
		if(!infd_tcp_server_open(tcp4, &error))
		{
			g_object_unref(tcp4);

			const std::string message = error->message;
			g_error_free(error);

			throw std::runtime_error(message);
		}
	}
	else
	{
		if(!infd_tcp_server_open(tcp4, NULL))
		{
			g_object_unref(tcp4);
			tcp4 = NULL;
		}
	}

	// We have the new server open, from this point on there is nothing
	// that can go wrong anymore. Therefore, close the old server and
	// take over the new one.
	if(is_open()) close();

	InfXmppConnectionSecurityPolicy policy =
		INF_XMPP_CONNECTION_SECURITY_ONLY_UNSECURED;
	if(creds != NULL) policy = security_policy;

	if(tcp4)
	{
		infd_tcp_server_set_keepalive(tcp4, keepalive);

		m_xmpp4 = infd_xmpp_server_new(
			tcp4, policy, creds,
			context, sasl_mechanisms);
		g_object_unref(tcp4);
	}

	if(tcp6)
	{
		infd_tcp_server_set_keepalive(tcp6, keepalive);

		m_xmpp6 = infd_xmpp_server_new(
			tcp6, policy, creds,
			context, sasl_mechanisms);
		g_object_unref(tcp6);
	}

	if(m_pool)
	{
		if(m_xmpp4 != NULL)
		{
			infd_server_pool_add_server(
				m_pool, INFD_XML_SERVER(m_xmpp4));
			infd_server_pool_add_local_publisher(
				m_pool, m_xmpp4, m_publisher);
		}

		if(m_xmpp6 != NULL)
		{
			infd_server_pool_add_server(
				m_pool, INFD_XML_SERVER(m_xmpp6));
			infd_server_pool_add_local_publisher(
				m_pool, m_xmpp6, m_publisher);
		}
	}
}

void Gobby::Server::close()
{
	g_assert(is_open());

	if(m_xmpp6 != NULL)
	{
		// Will be removed from server pool automatically
		infd_xml_server_close(INFD_XML_SERVER(m_xmpp6));
		g_object_unref(m_xmpp6);
		m_xmpp6 = NULL;
	}

	if(m_xmpp4 != NULL)
	{
		// Will be removed from server pool automatically
		infd_xml_server_close(INFD_XML_SERVER(m_xmpp4));
		g_object_unref(m_xmpp4);
		m_xmpp4 = NULL;
	}
}

bool Gobby::Server::is_open() const
{
	return m_xmpp4 != NULL || m_xmpp6 != NULL;
}

unsigned int Gobby::Server::get_port() const
{
	g_assert(is_open());

	InfdXmppServer* xmpp = m_xmpp6;
	if(xmpp == NULL) xmpp = m_xmpp4;
	g_assert(xmpp != NULL);

	InfdTcpServer* tcp;
	g_object_get(G_OBJECT(xmpp), "tcp-server", &tcp, NULL);

	guint port;
	g_object_get(G_OBJECT(tcp), "local-port", &port, NULL);
	g_object_unref(tcp);

	return port;
}

void Gobby::Server::set_keepalive(const InfKeepalive* keepalive)
{
	if(m_xmpp6 != NULL)
	{
		InfdTcpServer* tcp;
		g_object_get(G_OBJECT(m_xmpp6), "tcp-server", &tcp, NULL);
		infd_tcp_server_set_keepalive(tcp, keepalive);
		g_object_unref(tcp);
	}

	if(m_xmpp4 != NULL)
	{
		InfdTcpServer* tcp;
		g_object_get(G_OBJECT(m_xmpp4), "tcp-server", &tcp, NULL);
		infd_tcp_server_set_keepalive(tcp, keepalive);
		g_object_unref(tcp);
	}
}

void Gobby::Server::set_sasl_context(InfSaslContext* context,
                                     const char* sasl_mechanisms)
{
	if(m_xmpp6 != NULL)
	{
		g_object_set(
			G_OBJECT(m_xmpp6),
			"sasl-context", context,
			"sasl-mechanisms", sasl_mechanisms,
			NULL);
	}

	if(m_xmpp4 != NULL)
	{
		g_object_set(
			G_OBJECT(m_xmpp4),
			"sasl-context", context,
			"sasl-mechanisms", sasl_mechanisms,
			NULL);
	}
}

void Gobby::Server::set_credentials(InfXmppConnectionSecurityPolicy policy,
                                    InfCertificateCredentials* credentials)
{
	if(m_xmpp6 != NULL)
	{
		g_object_set(
			G_OBJECT(m_xmpp6),
			"security-policy", policy,
			"credentials", credentials,
			NULL);
	}

	if(m_xmpp4 != NULL)
	{
		g_object_set(
			G_OBJECT(m_xmpp4),
			"security-policy", policy,
			"credentials", credentials,
			NULL);
	}
}

void Gobby::Server::set_pool(InfdServerPool* pool)
{
	if(m_pool != NULL)
	{
		if(m_xmpp4 != NULL)
			infd_server_pool_remove_server(
				m_pool, INFD_XML_SERVER(m_xmpp4));
		if(m_xmpp6 != NULL)
			infd_server_pool_remove_server(
				m_pool, INFD_XML_SERVER(m_xmpp6));

		g_object_unref(m_pool);
	}

	m_pool = pool;

	if(m_pool != NULL)
	{
		g_object_ref(m_pool);

		if(m_xmpp4 != NULL)
		{
			infd_server_pool_add_server(
				m_pool, INFD_XML_SERVER(m_xmpp4));
			infd_server_pool_add_local_publisher(
				m_pool, m_xmpp4, m_publisher);
		}

		if(m_xmpp6 != NULL)
		{
			infd_server_pool_add_server(
				m_pool, INFD_XML_SERVER(m_xmpp6));
			infd_server_pool_add_local_publisher(
				m_pool, m_xmpp6, m_publisher);
		}
	}
}

