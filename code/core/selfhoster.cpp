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
#include "util/i18n.hpp"

#include <libinfinity/server/infd-filesystem-storage.h>

Gobby::SelfHoster::SelfHoster(InfIo* io,
                              InfCommunicationManager* communication_manager,
                              InfLocalPublisher* publisher,
                              InfSaslContext* sasl_context,
                              StatusBar& status_bar,
                              CertificateManager& cert_manager,
                              const Preferences& preferences):
	m_sasl_context(sasl_context),
	m_status_bar(status_bar),
	m_cert_manager(cert_manager),
	m_preferences(preferences),
	m_dh_params_loaded(false),
	m_info_handle(status_bar.invalid_handle()),
	m_dh_params_message_handle(status_bar.invalid_handle()),
	m_directory(infd_directory_new(io, NULL, communication_manager)),
	m_server(io, publisher)
{
	inf_sasl_context_ref(m_sasl_context);

	if(m_preferences.user.keep_local_documents)
	{
		const std::string directory =
			m_preferences.user.host_directory;
		InfdFilesystemStorage* storage =
			infd_filesystem_storage_new(directory.c_str());
		g_object_set(G_OBJECT(m_directory), "storage", storage, NULL);
		g_object_unref(storage);
	}

	InfdServerPool* pool = infd_server_pool_new(m_directory);
	m_server.set_pool(pool);
	g_object_unref(pool);

	m_preferences.user.require_password.signal_changed().connect(
		sigc::mem_fun(
			*this, &SelfHoster::on_require_password_changed));
	/*m_preferences.user.password.signal_changed().connect(
		sigc::mem_fun(*this, &SelfHoster::on_password));*/

	m_preferences.user.allow_remote_access.signal_changed().connect(
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
	m_preferences.network.keepalive.signal_changed().connect(
		sigc::mem_fun(*this, &SelfHoster::apply_preferences));
	m_cert_manager.signal_credentials_changed().connect(
		sigc::mem_fun(*this, &SelfHoster::apply_preferences));

	apply_preferences();
}

Gobby::SelfHoster::~SelfHoster()
{
	g_object_unref(m_directory);
	inf_sasl_context_unref(m_sasl_context);
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
		m_dh_params_message_handle = m_status_bar.add_info_message(
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

void Gobby::SelfHoster::directory_foreach_func_close_static(
	InfXmlConnection* connection,
	gpointer user_data)
{
	inf_xml_connection_close(connection);
}

void Gobby::SelfHoster::directory_foreach_func_set_sasl_context_static(
	InfXmlConnection* connection,
	gpointer user_data)
{
	g_assert(INF_IS_XMPP_CONNECTION(connection));

	SelfHoster* hoster = static_cast<SelfHoster*>(user_data);

	inf_xmpp_connection_reset_sasl_authentication(
		INF_XMPP_CONNECTION(connection),
		hoster->m_sasl_context, hoster->get_sasl_mechanisms());
}

const char* Gobby::SelfHoster::get_sasl_mechanisms() const
{
	if(m_preferences.user.require_password)
		return "PLAIN";
	else
		return "ANONYMOUS";
}

void Gobby::SelfHoster::on_require_password_changed()
{
	// Update SASL context and mechanisms for new connections:
	m_server.set_sasl_context(m_sasl_context, get_sasl_mechanisms());

	// Also update the SASL context for all existing connections. This is
	// important, so that the new password requirement setting also
	// affects already connected but not yet authorized clients.
	infd_directory_foreach_connection(
		m_directory, directory_foreach_func_set_sasl_context_static,
		this);
}

void Gobby::SelfHoster::apply_preferences()
{
	// Update directory storage
	if(m_preferences.user.keep_local_documents)
	{
		InfdStorage* storage =
			infd_directory_get_storage(m_directory);
		g_assert(storage == NULL ||
		         INFD_IS_FILESYSTEM_STORAGE(storage));
		InfdFilesystemStorage* fs_storage =
			INFD_FILESYSTEM_STORAGE(storage);

		const std::string new_directory =
			m_preferences.user.host_directory;

		bool set_new_storage = true;
		if(fs_storage != NULL)
		{
			gchar* root_directory;
			g_object_get(
				G_OBJECT(fs_storage),
				"root-directory", &root_directory, NULL);

			if(strcmp(root_directory, new_directory.c_str()) == 0)
				set_new_storage = false;

			g_free(root_directory);
		}

		if(set_new_storage)
		{
			fs_storage = infd_filesystem_storage_new(
				new_directory.c_str());
			g_object_set(
				G_OBJECT(m_directory),
				"storage", fs_storage, NULL);
			g_object_unref(fs_storage);
		}
	}
	else
	{
		if(infd_directory_get_storage(m_directory) != NULL)
		{
			g_object_set(
				G_OBJECT(m_directory), "storage", NULL, NULL);
		}
	}

	// Remove old statusbar message, if any
	if(m_info_handle != m_status_bar.invalid_handle())
	{
		m_status_bar.remove_message(m_info_handle);
		m_info_handle = m_status_bar.invalid_handle();
	}

	// Close server and all connections if no access is required
	if(!m_preferences.user.allow_remote_access)
	{
		infd_directory_foreach_connection(
			m_directory, directory_foreach_func_close_static,
			this);
		if(m_server.is_open())
			m_server.close();
		return;
	}

	// Okay, we want to share our documents, so let's try to start a
	// server for it.

	// Make sure TLS credentials are available.
	if(m_preferences.security.policy !=
	   INF_XMPP_CONNECTION_SECURITY_ONLY_UNSECURED &&
	   (m_preferences.security.authentication_enabled != true ||
            m_cert_manager.get_private_key() == NULL ||
	    m_cert_manager.get_certificates() == NULL))
	{
		m_info_handle = m_status_bar.add_info_message(
			_("In order to start sharing your documents, "
			  "choose a private key and certificate or "
			  "create a new pair in the preferences"));
		return;
	}

	// Make sure we have DH parameters
	if(!ensure_dh_params()) return;

	// Okay, go and open a server. If the server is already open the
	// command below will only change the port and/or security policy.
	try
	{
		const InfKeepalive& keepalive =
			m_preferences.network.keepalive;
		m_server.open(m_preferences.user.port, &keepalive,
		              m_preferences.security.policy,
		              m_cert_manager.get_credentials(),
		              m_sasl_context, get_sasl_mechanisms());
	}
	catch(const std::exception& ex)
	{
		m_status_bar.add_error_message(_("Failed to share documents"),
		                               ex.what());

		return;
	}
}
