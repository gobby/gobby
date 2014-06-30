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

#ifndef _GOBBY_CONNECTIONMANAGER_HPP_
#define _GOBBY_CONNECTIONMANAGER_HPP_

#include "core/preferences.hpp"
#include "core/certificatemanager.hpp"

#include <libinfinity/communication/inf-communication-manager.h>
#include <libinfinity/common/inf-discovery-avahi.h>
#include <libinfinity/common/inf-discovery.h>
#include <libinfinity/common/inf-local-publisher.h>
#include <libinfinity/common/inf-xmpp-manager.h>
#include <libinfinity/inf-config.h>

namespace Gobby
{

// This class manages outgoing connections. It also manages an
// InfDiscoveryAvahi object. Incoming connections are handled by
// SelfHoster.
class ConnectionManager: public sigc::trackable
{
public:
	ConnectionManager(const CertificateManager& cert_manager,
	                  const Preferences& preferences);
	~ConnectionManager();

	InfIo* get_io() { return m_io; }
	InfCommunicationManager* get_communication_manager() { return m_communication_manager; }
	InfXmppManager* get_xmpp_manager() { return m_xmpp_manager; }

#ifdef LIBINFINITY_HAVE_AVAHI
	InfDiscovery* get_discovery() { return INF_DISCOVERY(m_discovery); }
	InfLocalPublisher* get_publisher() { return INF_LOCAL_PUBLISHER(m_discovery); }
#else
	InfDiscovery* get_discovery() { return NULL; }
	InfLocalPublisher* get_publisher() { return NULL; }
#endif

	// Use existing connection if any, otherwise make new one. May throw.
	InfXmppConnection* make_connection(const InfIpAddress* address,
	                                   guint port,
	                                   unsigned int device_index,
	                                   const std::string& hostname);

	void remove_connection(InfXmppConnection* connection);

	// SASL context to be used for all new connections
	void set_sasl_context(InfSaslContext* sasl_context,
	                      const char* mechanisms);

protected:
	void on_security_policy_changed();
	void on_credentials_changed();

	const CertificateManager& m_cert_manager;
	const Preferences& m_preferences;

	InfIo* m_io;
	InfCommunicationManager* m_communication_manager;
	InfXmppManager* m_xmpp_manager;

	InfSaslContext* m_sasl_context;
	std::string m_sasl_mechanisms;
#ifdef LIBINFINITY_HAVE_AVAHI
	InfDiscoveryAvahi* m_discovery;
#endif
};

}
	
#endif // _GOBBY_CONNECTIONMANAGER_HPP_
