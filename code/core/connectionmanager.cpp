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

#include "core/connectionmanager.hpp"

#include <libinfgtk/inf-gtk-io.h>

Gobby::ConnectionManager::ConnectionManager(const CertificateManager& manager,
                                            const Preferences& preferences):
	m_cert_manager(manager),
	m_preferences(preferences),
	m_io(INF_IO(inf_gtk_io_new())),
	m_communication_manager(inf_communication_manager_new()),
	m_xmpp_manager(inf_xmpp_manager_new()),
	m_sasl_context(NULL)
{
	m_connection_added_handler = g_signal_connect_after(
		G_OBJECT(m_xmpp_manager), "connection-added",
		G_CALLBACK(on_connection_added_static), this);

	m_connection_removed_handler = g_signal_connect_after(
		G_OBJECT(m_xmpp_manager), "connection-removed",
		G_CALLBACK(on_connection_removed_static), this);

#ifdef LIBINFINITY_HAVE_AVAHI
	m_discovery = inf_discovery_avahi_new(m_io, m_xmpp_manager,
			                      m_cert_manager.get_credentials(),
			                      NULL, NULL);
	// TODO: Should be a constructor argument...
	inf_discovery_avahi_set_security_policy(
		m_discovery, m_preferences.security.policy);
	const InfKeepalive& keepalive = m_preferences.network.keepalive;
	inf_discovery_avahi_set_keepalive(m_discovery, &keepalive);
#endif

	m_preferences.security.policy.signal_changed().connect(
		sigc::mem_fun(
			*this,
			&ConnectionManager::on_security_policy_changed));
	m_preferences.network.keepalive.signal_changed().connect(
		sigc::mem_fun(
			*this,
			&ConnectionManager::on_keepalive_changed));
	m_cert_manager.signal_credentials_changed().connect(
		sigc::mem_fun(
			*this,
			&ConnectionManager::on_credentials_changed));
}

Gobby::ConnectionManager::~ConnectionManager()
{
#ifdef LIBINFINITY_HAVE_AVAHI
	g_object_unref(m_discovery);
#endif
	if(m_sasl_context)
		inf_sasl_context_unref(m_sasl_context);

	for(std::map<InfXmppConnection*, ConnectionInfo>::const_iterator it =
		m_connections.begin();
	    it != m_connections.end(); ++it)
	{
		g_signal_handler_disconnect(
			G_OBJECT(it->first),
			it->second.notify_status_handler);
	}

	g_signal_handler_disconnect(G_OBJECT(m_xmpp_manager),
	                            m_connection_added_handler);
	g_signal_handler_disconnect(G_OBJECT(m_xmpp_manager),
	                            m_connection_removed_handler);

	g_object_unref(m_xmpp_manager);
	g_object_unref(m_communication_manager);
	g_object_unref(m_io);
}

InfXmppConnection* Gobby::ConnectionManager::make_connection(
	const std::string& hostname, const std::string& service,
	unsigned int device_index, bool connect)
{
	// Check whether we do have such a connection already:
	InfXmppConnection* xmpp =
		inf_xmpp_manager_lookup_connection_by_hostname(
			m_xmpp_manager, hostname.c_str(),
			service.c_str(), "_infinote._tcp");

	if(!xmpp)
	{
		InfNameResolver* resolver = inf_name_resolver_new(
			m_io, hostname.c_str(),
			service.c_str(), "_infinote._tcp");

		InfTcpConnection* connection = inf_tcp_connection_new_resolve(
			m_io, resolver);

		g_object_unref(resolver);

		xmpp = create_connection(
			connection, device_index, hostname, connect);
	}
	else if(connect)
	{
		InfXmlConnectionStatus status;
		g_object_get(G_OBJECT(xmpp), "status", &status, NULL);
		if(status == INF_XML_CONNECTION_CLOSED)
		{
			GError* error = NULL;
			inf_xml_connection_open(INF_XML_CONNECTION(xmpp),
			                        &error);
			if(error != NULL)
			{
				std::string message = error->message;
				g_error_free(error);
				throw std::runtime_error(message);
			}
		}
	}

	g_assert(xmpp != NULL);
	return xmpp;

}

InfXmppConnection* Gobby::ConnectionManager::make_connection(
	const InfIpAddress* address,
	guint port,
	unsigned int device_index,
	const std::string& hostname,
	bool connect)
{
	// Check whether we do have such a connection already:
	InfXmppConnection* xmpp =
		inf_xmpp_manager_lookup_connection_by_address(
			m_xmpp_manager, address, port);

	if(!xmpp)
	{
		InfTcpConnection* connection = inf_tcp_connection_new(
			m_io, address, port);
		xmpp = create_connection(
			connection, device_index, hostname, connect);
	}
	else if(connect)
	{
		InfXmlConnectionStatus status;
		g_object_get(G_OBJECT(xmpp), "status", &status, NULL);
		if(status == INF_XML_CONNECTION_CLOSED)
		{
			GError* error = NULL;
			inf_xml_connection_open(INF_XML_CONNECTION(xmpp),
			                        &error);
			if(error != NULL)
			{
				std::string message = error->message;
				g_error_free(error);
				throw std::runtime_error(message);
			}
		}
	}

	g_assert(xmpp != NULL);
	return xmpp;
}

InfXmppConnection*
Gobby::ConnectionManager::create_connection(InfTcpConnection* connection,
                                            unsigned int device_index,
                                            const std::string& hostname,
                                            bool connect)
{
	g_object_set(G_OBJECT(connection),
		"device-index", device_index,
		NULL);

	const InfKeepalive& keepalive = m_preferences.network.keepalive;
	GError* error = NULL;
	if(!inf_tcp_connection_set_keepalive(connection, &keepalive, &error))
	{
		/* This should not happen, since set_keepalive can only fail
		 * if the connection is open already. */
		g_warning("Failed to set keepalive: %s", error->message);
		g_error_free(error);
		error = NULL;
	}

	if(connect && !inf_tcp_connection_open(connection, &error))
	{
		std::string message = error->message;
		g_error_free(error);
		g_object_unref(connection);
		throw std::runtime_error(message);
	}
	else
	{
		InfXmppConnection* xmpp = inf_xmpp_connection_new(
			connection, INF_XMPP_CONNECTION_CLIENT,
			NULL, hostname.c_str(),
			m_preferences.security.policy,
			m_cert_manager.get_credentials(),
			m_sasl_context,
			m_sasl_mechanisms.empty()
				? ""
				: m_sasl_mechanisms.c_str());

		inf_xmpp_manager_add_connection(m_xmpp_manager, xmpp);
		g_object_unref(xmpp);

		g_object_unref(connection);
		return xmpp;
	}
}

void Gobby::ConnectionManager::remove_connection(InfXmppConnection* connection)
{
	inf_xmpp_manager_remove_connection(
		m_xmpp_manager, connection);
}

void Gobby::ConnectionManager::set_sasl_context(InfSaslContext* sasl_context,
                                                const char* mechanisms)
{
	if(m_sasl_context) inf_sasl_context_unref(m_sasl_context);
	m_sasl_context = sasl_context;
	if(m_sasl_context) inf_sasl_context_ref(m_sasl_context);
	m_sasl_mechanisms = mechanisms ? mechanisms : "";

	// TODO: Should we also change the SASL context
	// for existing connections? Doesn't really matter because
	// SASL context does not change for client side...

#ifdef LIBINFINITY_HAVE_AVAHI
	g_object_set(G_OBJECT(m_discovery),
		"sasl-context", m_sasl_context,
		"sasl-mechanisms", mechanisms,
		NULL);
#endif
}

void Gobby::ConnectionManager::on_connection_added(InfXmppConnection* xmpp)
{
	g_assert(m_connections.find(xmpp) == m_connections.end());

	ConnectionInfo info;

	info.notify_status_handler = g_signal_connect(
		G_OBJECT(xmpp), "notify::status",
		G_CALLBACK(on_notify_status_static), this);

	m_connections[xmpp] = info;
}

void Gobby::ConnectionManager::on_connection_removed(
	InfXmppConnection* xmpp,
	InfXmppConnection* replaced_by)
{
	std::map<InfXmppConnection*, ConnectionInfo>::iterator iter =
		m_connections.find(xmpp);
	g_assert(iter != m_connections.end());

	ConnectionInfo& info = iter->second;
	g_signal_handler_disconnect(G_OBJECT(xmpp),
	                            info.notify_status_handler);

	m_connections.erase(iter);

	if(replaced_by != NULL)
	{
		m_signal_connection_replaced.emit(xmpp, replaced_by);

		// This is not needed, since the connection will be
		// removed anyway, because nobody holds a reference
		// anymore. If it turns out that the connection stays
		// alive, and for example the certificate dialog shows
		// up, then it means we have a memory leak somewhere.
		//inf_xml_connection_close(INF_XML_CONNECTION(xmpp));
	}
}

void Gobby::ConnectionManager::on_security_policy_changed()
{
#ifdef LIBINFINITY_HAVE_AVAHI
	inf_discovery_avahi_set_security_policy(
		m_discovery, m_preferences.security.policy);
#endif
}

void Gobby::ConnectionManager::on_keepalive_changed()
{
	const InfKeepalive& keepalive = m_preferences.network.keepalive;

#ifdef LIBINFINITY_HAVE_AVAHI
	inf_discovery_avahi_set_keepalive(m_discovery, &keepalive);
#endif

	for(std::map<InfXmppConnection*, ConnectionInfo>::const_iterator it =
		m_connections.begin();
	    it != m_connections.end(); ++it)
	{
		InfTcpConnection* tcp;
		g_object_get(G_OBJECT(it->first), "tcp-connection", &tcp, NULL);

		GError* error = NULL;
		inf_tcp_connection_set_keepalive(tcp, &keepalive, &error);
		g_object_unref(tcp);

		if(error != NULL)
		{
			g_warning("Failed to set keepalive: %s",
			          error->message);
			g_error_free(error);
		}
	}
}

void Gobby::ConnectionManager::on_credentials_changed()
{
	// Keep existing connections with current credentials but set
	// new credentials for all closed connections.
	for(std::map<InfXmppConnection*, ConnectionInfo>::const_iterator it =
		m_connections.begin();
	    it != m_connections.end(); ++it)
	{
		InfXmlConnectionStatus status;
		g_object_get(G_OBJECT(it->first), "status", &status, NULL);
		if(status == INF_XML_CONNECTION_CLOSED)
		{
			g_object_set(
				G_OBJECT(it->first), "credentials",
				m_cert_manager.get_credentials(), NULL);
		}
	}

#ifdef LIBINFINITY_HAVE_AVAHI
	g_object_set(
		G_OBJECT(m_discovery),
		"credentials",
		m_cert_manager.get_credentials(),
		NULL);
#endif
}

void Gobby::ConnectionManager::on_notify_status(InfXmppConnection* connection)
{
	InfXmlConnectionStatus status;
	g_object_get(G_OBJECT(connection), "status", &status, NULL);

	// When the connection was closed, update the certificate credentials,
	// so that in case it is reopened the current credentials are used.
	if(status == INF_XML_CONNECTION_CLOSED)
	{
		g_object_set(
			G_OBJECT(connection), "credentials",
			m_cert_manager.get_credentials(), NULL);
	}
}
