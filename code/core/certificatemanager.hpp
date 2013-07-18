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

#ifndef _GOBBY_CERTIFICATEMANAGER_HPP_
#define _GOBBY_CERTIFICATEMANAGER_HPP_

#include "core/preferences.hpp"

namespace Gobby
{
	class CertificateManager
	{
	public:
		CertificateManager(const Preferences& preferences);
		~CertificateManager();

		gnutls_x509_privkey_t get_private_key() const
			{ return m_key; }
		InfCertificateChain* get_certificates() const
			{ return m_certificates; }

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
		const Preferences& m_preferences;

		gnutls_x509_privkey_t m_key;
		InfCertificateChain* m_certificates;
		std::vector<gnutls_x509_crt_t> m_trust;

		InfCertificateCredentials* m_credentials;

		GError* m_key_error;
		GError* m_certificate_error;
		GError* m_trust_error;

		signal_credentials_changed_type m_signal_credentials_changed;
	private:
		void load_key();
		void load_certificate();
		void load_trust();
		void make_credentials();

		void on_key_file_changed();
		void on_certificate_file_changed();
		void on_trust_file_changed();

		void on_authentication_enabled_changed();
	};
}

#endif // _GOBBY_CERTIFICATEMANAGER_HPP_
