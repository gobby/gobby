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

#include "core/certificatemanager.hpp"
#include "util/file.hpp"
#include "util/i18n.hpp"

#include <libinfinity/common/inf-cert-util.h>

#include <gnutls/x509.h>

Gobby::CertificateManager::CertificateManager(Preferences& preferences):
	m_preferences(preferences),
	m_dh_params(NULL), m_key(NULL), m_certificates(NULL),
	m_credentials(NULL), m_key_error(NULL), m_certificate_error(NULL),
	m_trust_error(NULL)
{
	m_conn_key_file = m_preferences.security.key_file.
		signal_changed().connect(sigc::mem_fun(
			*this, &CertificateManager::on_key_file_changed));
	m_conn_certificate_file = m_preferences.security.certificate_file.
		signal_changed().connect(sigc::mem_fun(
			*this,
			 &CertificateManager::on_certificate_file_changed));
	m_preferences.security.trust_file.signal_changed().connect(
		sigc::mem_fun(
			*this, &CertificateManager::on_trust_file_changed));

	m_preferences.security.authentication_enabled.
		signal_changed().connect(
			sigc::mem_fun(
				*this,
				 &CertificateManager::
					on_authentication_enabled_changed));

	// TODO: Load these only on request, to improve the startup time
	load_dh_params();
	load_key();
	load_certificate();
	load_trust();

	make_credentials();
}

Gobby::CertificateManager::~CertificateManager()
{
	if(m_credentials != NULL)
		inf_certificate_credentials_unref(m_credentials);
	for(unsigned int i = 0; i < m_trust.size(); ++i)
		gnutls_x509_crt_deinit(m_trust[i]);
	if(m_certificates != NULL)
		inf_certificate_chain_unref(m_certificates);
	if(m_key != NULL)
		gnutls_x509_privkey_deinit(m_key);
	if(m_dh_params != NULL)
		gnutls_dh_params_deinit(m_dh_params);
}

void Gobby::CertificateManager::set_dh_params(gnutls_dh_params_t dh_params)
{
	gnutls_dh_params_t old_dh_params = m_dh_params;

	GError* error = NULL;
	std::string filename = config_filename("dh_params.pem");
	inf_cert_util_write_dh_params(dh_params, filename.c_str(), &error);

	if(error != NULL)
	{
		g_warning(
			_("Failed to write Diffie-Hellman parameters "
			  "to \"%s\": %s"),
			filename.c_str(),
			error->message);
		g_error_free(error);
	}

	m_dh_params = dh_params;

	make_credentials();

	// TODO: Note that the credentials do only store a pointer to the
	// DH params, so we cannot just delete the DH params here, since the
	// old credentials might still be in use.
	// For the moment we don't let this happen -- in principle it should
	// not happen; once we have valid DH params at one point we don't
	// need to change them again.
	// For the future maybe it could make sense to store the DH params
	// in the InfCertificateCredentials struct, so that their lifetime
	// is coupled.
	g_assert(old_dh_params == NULL);
}

void Gobby::CertificateManager::set_private_key(gnutls_x509_privkey_t key,
                                                const GError* error)
{
	g_assert(key == NULL || error == NULL);

	gnutls_x509_privkey_t old_key = m_key;
	InfCertificateChain* old_certificates = m_certificates;

	if(old_certificates != NULL)
		inf_certificate_chain_ref(old_certificates);

	m_key = key;
	if(m_key_error != NULL)
		g_error_free(m_key_error);
	if(error != NULL)
		m_key_error = g_error_copy(error);
	else
		m_key_error = NULL;

	// Attempt to re-load the certificate if there was an error -- maybe
	// the new key fixes the problem. This makes sure that if the new key
	// is compatible to the certificate, the certificate is loaded.

	// TODO: It would be nicer to still keep the certificate in memory
	// when it does not match the key, so we don't need to re-load it.
	// Basically we just need to be able to handle the case when both
	// cert_error and certificate itself are non-NULL.
	if(m_certificate_error != NULL)
	{
		load_certificate();
	}
	else
	{
		check_certificate_signature();
		make_credentials();
	}

	// Note that this relies on the fact that
	// gnutls_certificate_set_x509_key makes a copy of the key
	// and certificate
	if(old_certificates != NULL)
		inf_certificate_chain_unref(old_certificates);
	if(old_key != NULL) gnutls_x509_privkey_deinit(old_key);
}

void Gobby::CertificateManager::set_private_key(gnutls_x509_privkey_t key,
                                                const char* filename,
                                                const GError* error)
{
	if(error != NULL)
	{
		g_assert(key == NULL);

		set_private_key(NULL, error);
	}
	else
	{
		GError* local_error = NULL;
		if(filename != NULL)
		{
			m_conn_key_file.block();
			m_preferences.security.key_file = filename;
			m_conn_key_file.unblock();

			if(key != NULL)
			{
				inf_cert_util_write_private_key(
					key, filename, &local_error);
			}
		}

		if(local_error != NULL)
		{
			set_private_key(NULL, local_error);
			if(key != NULL) gnutls_x509_privkey_deinit(key);
			g_error_free(local_error);
		}
		else
		{
			set_private_key(key, NULL);
		}
	}
}

void Gobby::CertificateManager::set_certificates(gnutls_x509_crt_t* certs,
                                                 guint n_certs,
                                                 const GError* error)
{
	g_assert(n_certs == 0 || error == NULL);

	InfCertificateChain* old_certificates = m_certificates;
	m_certificates = NULL;

	if(n_certs > 0)
		m_certificates = inf_certificate_chain_new(certs, n_certs);
	else
		m_certificates = NULL;

	if(m_certificate_error != NULL)
		g_error_free(m_certificate_error);

	if(error != NULL)
		m_certificate_error = g_error_copy(error);
	else
		m_certificate_error = NULL;

	check_certificate_signature();
	make_credentials();

	// Note that this relies on the fact that
	// gnutls_certificate_set_x509_key makes a copy of the certificates
	if(old_certificates != NULL)
		inf_certificate_chain_unref(old_certificates);
}

void Gobby::CertificateManager::set_certificates(gnutls_x509_crt_t* certs,
                                                 guint n_certs,
                                                 const char* filename,
                                                 const GError* error)
{
	if(error != NULL)
	{
		g_assert(n_certs == 0);

		set_certificates(NULL, 0, error);
	}
	else
	{
		GError* local_error = NULL;
		if(filename != NULL)
		{
			m_conn_certificate_file.block();
			m_preferences.security.certificate_file = filename;
			m_conn_certificate_file.unblock();

			if(n_certs > 0)
			{
				inf_cert_util_write_certificate(
					certs, n_certs, filename,
					&local_error);
			}
		}

		if(local_error != NULL)
		{
			set_certificates(NULL, 0, local_error);
			for(guint i = 0; i < n_certs; ++i)
				gnutls_x509_crt_deinit(certs[i]);
			g_error_free(local_error);
		}
		else
		{
			set_certificates(certs, n_certs, NULL);
		}
	}
}

void Gobby::CertificateManager::load_dh_params()
{
	const std::string filename = config_filename("dh_params.pem");

	GError* error = NULL;
	gnutls_dh_params_t dh_params =
		inf_cert_util_read_dh_params(filename.c_str(), &error);

	if(error != NULL)
	{
		if(error->domain != G_FILE_ERROR ||
		   error->code != G_FILE_ERROR_NOENT)
		{
			g_warning(_("Failed to read Diffie-Hellman "
			            "parameters: %s"),
			          error->message);
		}

		g_error_free(error);
	}

	if(dh_params != NULL)
		set_dh_params(dh_params);
}

void Gobby::CertificateManager::load_key()
{
	const std::string& filename = m_preferences.security.key_file;

	if(!filename.empty())
	{
		GError* error = NULL;
		gnutls_x509_privkey_t key = inf_cert_util_read_private_key(
			filename.c_str(), &error);

		set_private_key(key, error);
		if(error != NULL)
			g_error_free(error);
	}
	else
	{
		set_private_key(NULL, NULL);
	}
}

void Gobby::CertificateManager::load_certificate()
{
	const std::string& filename = m_preferences.security.certificate_file;

	if(!filename.empty())
	{
		GError* error = NULL;
		GPtrArray* array = inf_cert_util_read_certificate(
			filename.c_str(), NULL, &error);
		if(array != NULL)
		{
			g_assert(error == NULL);
			guint n_certs = array->len;
			gnutls_x509_crt_t* certs =
				reinterpret_cast<gnutls_x509_crt_t*>(
					g_ptr_array_free(array, FALSE));

			if(n_certs > 0)
			{
				set_certificates(certs, n_certs, NULL);
			}
			else
			{
				g_set_error(
					&error,
					g_quark_from_static_string(
						"GOBBY_CERTIFICATE_MANAGER_"
						"ERROR"),
					1,
					"%s",
					_("File does not contain a "
					  "X.509 certificate")
				);
				
				set_certificates(NULL, 0, error);

				g_error_free(error);
				g_free(certs);
			}
		}
		else
		{
			g_assert(error != NULL);
			set_certificates(NULL, 0, error);
			g_error_free(error);
		}
	}
	else
	{
		set_certificates(NULL, 0, NULL);
	}
}

void Gobby::CertificateManager::load_trust()
{
	std::vector<gnutls_x509_crt_t> old_trust;
	old_trust.swap(m_trust);

	GError* error = NULL;
	const std::string& filename = m_preferences.security.trust_file;

	if(!filename.empty())
	{
		GPtrArray* array = inf_cert_util_read_certificate(
			filename.c_str(), NULL, &error);
		if(array != NULL)
		{
			guint n_certs = array->len;
			gnutls_x509_crt_t* certs =
				reinterpret_cast<gnutls_x509_crt_t*>(
					g_ptr_array_free(array, FALSE));
			m_trust.assign(certs, certs + n_certs);
			g_free(certs);
		}
	}

	if(m_trust_error != NULL)
		g_error_free(m_trust_error);
	m_trust_error = error;

	make_credentials();

	// Note that this relies on the fact that
	// gnutls_certificate_set_x509_trust makes a copy of the certificates
	for(unsigned int i = 0; i < old_trust.size(); ++i)
		gnutls_x509_crt_deinit(old_trust[i]);
}

void Gobby::CertificateManager::check_certificate_signature()
{
	if(!m_key || !m_certificates) return;
	g_assert(m_key_error == NULL && m_certificate_error == NULL);

	gnutls_x509_crt_t crt =
		inf_certificate_chain_get_own_certificate(m_certificates);
	if(!inf_cert_util_check_certificate_key(crt, m_key))
	{
		inf_certificate_chain_unref(m_certificates);
		m_certificates = NULL;

		g_set_error(
			&m_certificate_error,
			g_quark_from_static_string(
				"GOBBY_CERTIFICATE_MANAGER_ERROR"),
			0,
			"%s",
			_("Certificate does not belong to the chosen key")
		);
	}
}

void Gobby::CertificateManager::make_credentials()
{
	InfCertificateCredentials* creds = inf_certificate_credentials_new();
	gnutls_certificate_credentials_t gnutls_creds =
		inf_certificate_credentials_get(creds);

	if(m_preferences.security.authentication_enabled &&
	   m_key != NULL && m_certificates != NULL)
	{
		gnutls_certificate_set_x509_key(
			gnutls_creds,
			inf_certificate_chain_get_raw(m_certificates),
			inf_certificate_chain_get_n_certificates(m_certificates),
			m_key
		);
	}

	if(!m_trust.empty())
	{
		gnutls_certificate_set_x509_trust(
			gnutls_creds,
			&m_trust[0],
			m_trust.size()
		);
	}

	if(m_dh_params != NULL)
		gnutls_certificate_set_dh_params(gnutls_creds, m_dh_params);

	gnutls_certificate_set_verify_flags(
		gnutls_creds, GNUTLS_VERIFY_ALLOW_X509_V1_CA_CRT);

	InfCertificateCredentials* old_creds = m_credentials;
	m_credentials = creds;

	m_signal_credentials_changed.emit();

	if(old_creds != NULL)
		inf_certificate_credentials_unref(old_creds);
}

void Gobby::CertificateManager::on_key_file_changed()
{
	load_key();
	//make_credentials();
}

void Gobby::CertificateManager::on_certificate_file_changed()
{
	load_certificate();
	//make_credentials();
}

void Gobby::CertificateManager::on_trust_file_changed()
{
	load_trust();
	//make_credentials();
}

void Gobby::CertificateManager::on_authentication_enabled_changed()
{
	make_credentials();
}
