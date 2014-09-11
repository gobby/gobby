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
	typedef sigc::signal<void, InfXmppConnection*, InfXmppConnection*>
		SignalConnectionReplaced;

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
	InfXmppConnection* make_connection(const std::string& hostname,
	                                   const std::string& service,
					   unsigned int device_index);
	InfXmppConnection* make_connection(const InfIpAddress* address,
	                                   guint port,
	                                   unsigned int device_index,
	                                   const std::string& hostname);

	void remove_connection(InfXmppConnection* connection);

	// SASL context to be used for all new connections
	void set_sasl_context(InfSaslContext* sasl_context,
	                      const char* mechanisms);

	SignalConnectionReplaced signal_connection_replaced() const
		{ return m_signal_connection_replaced; }
private:
	InfXmppConnection* create_connection(InfTcpConnection* connection,
	                                     unsigned int device_index,
	                                     const std::string& hostname);

	static void on_notify_status_static(GObject* object,
	                                    GParamSpec* pspec,
	                                    gpointer user_data)
	{
		static_cast<ConnectionManager*>(user_data)->on_notify_status(
			INF_XMPP_CONNECTION(object));
	}

	static void on_connection_added_static(InfXmppManager* manager,
	                                       InfXmppConnection* xmpp,
	                                       gpointer user_data)
	{
		static_cast<ConnectionManager*>(user_data)->
			on_connection_added(xmpp);
	}

	static void on_connection_removed_static(InfXmppManager* manager,
	                                         InfXmppConnection* xmpp,
						 InfXmppConnection* replaced,
	                                         gpointer user_data)
	{
		static_cast<ConnectionManager*>(user_data)->
			on_connection_removed(xmpp, replaced);
	}

protected:
	void on_connection_added(InfXmppConnection* xmpp);
	void on_connection_removed(InfXmppConnection* xmpp,
	                           InfXmppConnection* replaced_by);
	void on_security_policy_changed();
	void on_keepalive_changed();
	void on_credentials_changed();
	void on_notify_status(InfXmppConnection* connection);

	const CertificateManager& m_cert_manager;
	const Preferences& m_preferences;

	InfIo* m_io;
	InfCommunicationManager* m_communication_manager;
	InfXmppManager* m_xmpp_manager;

	struct ConnectionInfo
	{
		gulong notify_status_handler;
	};

	std::map<InfXmppConnection*, ConnectionInfo> m_connections;

	gulong m_connection_added_handler;
	gulong m_connection_removed_handler;

	InfSaslContext* m_sasl_context;
	std::string m_sasl_mechanisms;
#ifdef LIBINFINITY_HAVE_AVAHI
	InfDiscoveryAvahi* m_discovery;
#endif

	SignalConnectionReplaced m_signal_connection_replaced;
};

}
	
#endif // _GOBBY_CONNECTIONMANAGER_HPP_
