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
#ifdef LIBINFINITY_HAVE_AVAHI
	m_discovery = inf_discovery_avahi_new(m_io, m_xmpp_manager,
			                      m_cert_manager.get_credentials(),
			                      NULL, NULL);
	// TODO: Should be a constructor argument...
	inf_discovery_avahi_set_security_policy(
		m_discovery, m_preferences.security.policy);
#endif

	m_preferences.security.policy.signal_changed().connect(
		sigc::mem_fun(
			*this,
			&ConnectionManager::on_security_policy_changed));
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
	g_object_unref(m_xmpp_manager);
	g_object_unref(m_communication_manager);
	g_object_unref(m_io);
}

InfXmppConnection* Gobby::ConnectionManager::make_connection(
	const InfIpAddress* address,
	guint port,
	unsigned int device_index,
	const std::string& hostname)
{
	// Check whether we do have such a connection already:
	InfXmppConnection* xmpp =
		inf_xmpp_manager_lookup_connection_by_address(
			m_xmpp_manager, address, port);

	if(!xmpp)
	{
		InfTcpConnection* connection = inf_tcp_connection_new(
			m_io,
			address,
			port);

		g_object_set(G_OBJECT(connection),
			"device-index", device_index,
			NULL);

		GError* error = NULL;
		if(!inf_tcp_connection_open(connection, &error))
		{
			std::string message = error->message;
			g_error_free(error);
			throw std::runtime_error(message);
		}
		else
		{
			xmpp = inf_xmpp_connection_new(
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
		}

		g_object_unref(connection);
	}
	else
	{
		InfXmlConnectionStatus status;
		g_object_get(G_OBJECT(xmpp), "status", &status, NULL);
		if(status == INF_XML_CONNECTION_CLOSED)
		{
			// TODO: Set new credentials and SASL
			// context before connecting... needs change in
			// libinfinity to allow re-setting credentials

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

void Gobby::ConnectionManager::on_security_policy_changed()
{
#ifdef LIBINFINITY_HAVE_AVAHI
	inf_discovery_avahi_set_security_policy(
		m_discovery, m_preferences.security.policy);
#endif
}

void Gobby::ConnectionManager::on_credentials_changed()
{
	// Keep existing connections with current credentials

#ifdef LIBINFINITY_HAVE_AVAHI
	g_object_set(
		G_OBJECT(m_discovery),
		"credentials",
		m_cert_manager.get_credentials(),
		NULL);
#endif
}

