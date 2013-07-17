/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2013 Armin Burgmeier <armin@arbur.net>
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

#include "core/certificatemanager.hpp"

#include <libinfinity/common/inf-cert-util.h>

#include <gnutls/x509.h>

Gobby::CertificateManager::CertificateManager(const Preferences& preferences):
	m_preferences(preferences),
	m_key(NULL), m_certificates(NULL), m_credentials(NULL),
	m_key_error(NULL), m_certificate_error(NULL),
	m_trust_error(NULL)
{
	m_preferences.security.key_file.signal_changed().connect(
		sigc::mem_fun(
			*this, &CertificateManager::on_key_file_changed));
	m_preferences.security.certificate_file.signal_changed().connect(
		sigc::mem_fun(
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
}

void Gobby::CertificateManager::load_key()
{
	gnutls_x509_privkey_t old_key = m_key;

	GError* error = NULL;
	const std::string& filename = m_preferences.security.key_file;

	if(!filename.empty())
	{
		m_key = inf_cert_util_read_private_key(
			filename.c_str(), &error);
	}
	else
	{
		m_key = NULL;
	}

	if(m_key_error != NULL)
		g_error_free(m_key_error);
	m_key_error = error;

	make_credentials();

	// Note that this relies on the fact that
	// gnutls_certificate_set_x509_key makes a copy of the key
	if(old_key != NULL) gnutls_x509_privkey_deinit(old_key);
}

void Gobby::CertificateManager::load_certificate()
{
	InfCertificateChain* old_certificates = m_certificates;

	GError* error = NULL;
	const std::string& filename = m_preferences.security.certificate_file;

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
			m_certificates =
				inf_certificate_chain_new(certs, n_certs);
		}
	}
	else
	{
		m_certificates = NULL;
	}

	if(m_certificate_error != NULL)
		g_error_free(m_certificate_error);
	m_certificate_error = error;

	make_credentials();

	// Note that this relies on the fact that
	// gnutls_certificate_set_x509_key makes a copy of the certificates
	if(old_certificates != NULL)
		inf_certificate_chain_unref(old_certificates);
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
	make_credentials();
}

void Gobby::CertificateManager::on_certificate_file_changed()
{
	load_certificate();
	make_credentials();
}

void Gobby::CertificateManager::on_trust_file_changed()
{
	load_trust();
	make_credentials();
}

void Gobby::CertificateManager::on_authentication_enabled_changed()
{
	make_credentials();
}
