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

#ifndef _GOBBY_CERTIFICATEMANAGER_HPP_
#define _GOBBY_CERTIFICATEMANAGER_HPP_

#include "core/preferences.hpp"

namespace Gobby
{
	class CertificateManager
	{
	public:
		CertificateManager(Preferences& preferences);
		~CertificateManager();

		gnutls_dh_params_t get_dh_params() const
			{ return m_dh_params; }
		gnutls_x509_privkey_t get_private_key() const
			{ return m_key; }
		InfCertificateChain* get_certificates() const
			{ return m_certificates; }

		void set_dh_params(gnutls_dh_params_t dh_params);

		void set_private_key(gnutls_x509_privkey_t key,
		                     const GError* error);
		void set_private_key(gnutls_x509_privkey_t key,
		                     const char* filename,
		                     const GError* error);

		void set_certificates(gnutls_x509_crt_t* certs,
		                      guint n_certs,
		                      const GError* error);
		void set_certificates(gnutls_x509_crt_t* certs,
		                      guint n_certs,
		                      const char* filename,
		                      const GError* error);

		unsigned int get_n_trusted_cas() const
			{ return m_trust.size(); }
		const gnutls_x509_crt_t* get_trusted_cas() const
			{ return m_trust.empty() ? NULL : &m_trust[0]; }

		InfCertificateCredentials* get_credentials() const
			{ return m_credentials; }

		// If an error occurs with the loading, the error can be
		// queried here.
		const GError* get_key_error() const
			{ return m_key_error; }
		const GError* get_certificate_error() const
			{ return m_certificate_error; }
		const GError* get_trust_error() const
			{ return m_trust_error; }

		typedef sigc::signal<void> signal_credentials_changed_type;
		signal_credentials_changed_type
		signal_credentials_changed() const
		{
			return m_signal_credentials_changed;
		}
	protected:
		Preferences& m_preferences;

		sigc::connection m_conn_key_file;
		sigc::connection m_conn_certificate_file;

		gnutls_dh_params_t m_dh_params;
		gnutls_x509_privkey_t m_key;
		InfCertificateChain* m_certificates;
		std::vector<gnutls_x509_crt_t> m_trust;

		InfCertificateCredentials* m_credentials;

		GError* m_key_error;
		GError* m_certificate_error;
		GError* m_trust_error;

		signal_credentials_changed_type m_signal_credentials_changed;
	private:
		void load_dh_params();
		void load_key();
		void load_certificate();
		void load_trust();

		void check_certificate_signature();
		void make_credentials();

		void on_key_file_changed();
		void on_certificate_file_changed();
		void on_trusted_cas_changed();

		void on_authentication_enabled_changed();
	};
}

#endif // _GOBBY_CERTIFICATEMANAGER_HPP_
