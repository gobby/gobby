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

#include "core/selfhoster.hpp"
#include "util/file.hpp"
#include "util/i18n.hpp"

#include <libinfinity/common/inf-cert-util.h>
#include <libinfinity/common/inf-error.h>

#include <gnutls/x509.h>

#include <cassert>

Gobby::SelfHoster::SelfHoster(InfIo* io,
                              InfLocalPublisher* publisher,
                              StatusBar& status_bar,
                              CertificateManager& cert_manager,
                              const Preferences& preferences):
	m_status_bar(status_bar),
	m_cert_manager(cert_manager),
	m_preferences(preferences),
	m_dh_params_loaded(false),
	m_info_handle(status_bar.invalid_handle()),
	m_dh_params_message_handle(status_bar.invalid_handle()),
	m_server(io, publisher),
	m_directory(NULL)
{
	// TODO: Create a directory, and server pool,
	// set server pool for m_server

	m_preferences.user.allow_remote_access.signal_changed().connect(
		sigc::mem_fun(*this, &SelfHoster::apply_preferences));
	m_preferences.user.require_password.signal_changed().connect(
		sigc::mem_fun(*this, &SelfHoster::apply_preferences));
	m_preferences.user.password.signal_changed().connect(
		sigc::mem_fun(*this, &SelfHoster::apply_preferences));
	m_preferences.user.port.signal_changed().connect(
		sigc::mem_fun(*this, &SelfHoster::apply_preferences));
	m_preferences.user.keep_local_documents.signal_changed().connect(
		sigc::mem_fun(*this, &SelfHoster::apply_preferences));
	m_preferences.user.host_directory.signal_changed().connect(
		sigc::mem_fun(*this, &SelfHoster::apply_preferences));
	m_preferences.security.authentication_enabled.signal_changed().
		connect(sigc::mem_fun(*this, &SelfHoster::apply_preferences));
	m_preferences.security.policy.signal_changed().connect(
		sigc::mem_fun(*this, &SelfHoster::apply_preferences));
	m_cert_manager.signal_credentials_changed().connect(
		sigc::mem_fun(*this, &SelfHoster::apply_preferences));

	apply_preferences();
}

Gobby::SelfHoster::~SelfHoster()
{
	g_object_unref(m_directory);
}

bool Gobby::SelfHoster::ensure_dh_params()
{
	// If they are loaded already: perfect. Note this does not mean we
	// actually have them, this flag is also set if we attempted to
	// generate the parameters but the generation failed.
	if(m_dh_params_loaded) return true;

	// Load from certificate manager, if available
	if(m_cert_manager.get_dh_params() != NULL)
	{
		m_dh_params_loaded = true;
		return true;
	}

	// Otherwise go and create a new set of parameters
	if(m_dh_params_handle.get() == NULL)
	{
		m_info_handle = m_status_bar.add_info_message(
			_("Generating 2048-bit Diffie-Hellman "
			  "parameters..."));

		m_dh_params_handle = create_dh_params(
			2048,
			sigc::mem_fun(*this, &SelfHoster::on_dh_params_done));
	}

	return false;
}

void Gobby::SelfHoster::on_dh_params_done(const DHParamsGeneratorHandle* hndl,
                                          gnutls_dh_params_t dh_params,
                                          const GError* error)
{
	g_assert(m_dh_params_message_handle != m_status_bar.invalid_handle());
	m_status_bar.remove_message(m_dh_params_message_handle);

	// Set this flag also when an error occured, to prevent trying to
	// re-generate the parameters all the time.
	m_dh_params_loaded = true;

	if(dh_params != NULL)
	{
		// Set the DH parameters in the certificate manager: this
		// will cause a credentials_changed notification, and we'll
		// retry starting the server.
		m_cert_manager.set_dh_params(dh_params);
	}
	else
	{
		m_status_bar.add_error_message(
			_("Failed to generate Diffie-Hellman parameters"),
			Glib::ustring::compose(
				_("This means that Perfect Forward Secrecy "
				  "(PFS) is not available. Restart Gobby to "
				  "re-try generating the parameters. The "
				  "specific error was:\n\n%1"),
				error->message));
	}
}

void Gobby::SelfHoster::apply_preferences()
{
	if(m_info_handle != m_status_bar.invalid_handle())
	{
		m_status_bar.remove_message(m_info_handle);
		m_info_handle = m_status_bar.invalid_handle();
	}

	if(!m_preferences.user.allow_remote_access)
	{
		// TODO: Disconnect all connections from InfdDirectory
		m_server.close();

		return;
	}

	if(!ensure_dh_params()) return;

	// Make sure TLS credentials are available
	if(m_preferences.security.policy !=
	   INF_XMPP_CONNECTION_SECURITY_ONLY_UNSECURED &&
	   (m_cert_manager.get_private_key() == NULL ||
	    m_cert_manager.get_certificates() == NULL))
	{
		m_info_handle = m_status_bar.add_info_message(
			_("In order to start sharing your documents, "
			  "choose a private key and certificate or "
			  "create a new pair in the preferences"));
	}

	try
	{
		m_server.open(m_preferences.user.port,
		              m_preferences.security.policy,
		              m_cert_manager.get_credentials());
	}
	catch(const std::exception& ex)
	{
		m_status_bar.add_error_message(_("Failed to share documents"),
		                               ex.what());

		return;
	}

	if(m_preferences.user.require_password)
	{
		// TODO: Change m_sasl_context, if necessary
	}
	else
	{
		// TODO: Change m_sasl_context, if necessary
	}

	// If m_sasl_context changed, apply it to the
	// server and all connections

	if(m_preferences.user.keep_local_documents)
	{
		// TODO: Set storage, and set path in storage if it changed
	}
	else
	{
		// TODO: Unset storage
	}
}
