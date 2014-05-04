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

#include "core/credentialsgenerator.hpp"

#include <libinfinity/common/inf-cert-util.h>
#include <libinfinity/common/inf-error.h>

#include <cassert>

namespace
{
	class Keygen: public Gobby::AsyncOperation
	{
	public:
		Keygen(gnutls_pk_algorithm_t algo,
		       unsigned int bits,
		       const Gobby::SlotKeyGeneratorDone& slot_done):
			m_algo(algo), m_bits(bits), m_slot_done(slot_done),
			m_key(NULL), m_error(NULL)
		{
		}

		~Keygen()
		{
			if(m_key != NULL)
				gnutls_x509_privkey_deinit(m_key);
			if(m_error != NULL)
				g_error_free(m_error);
		}

	protected:
		virtual void run()
		{
			m_key = inf_cert_util_create_private_key(
				m_algo, m_bits, &m_error);
		}

		virtual void finish()
		{
			if(m_error != NULL)
			{
				m_slot_done(get_handle(), NULL, m_error);
			}
			else
			{
				gnutls_x509_privkey_t key = m_key;
				m_key = NULL;

				m_slot_done(get_handle(), key, NULL);
			}
		}

	private:
		const gnutls_pk_algorithm_t m_algo;
		const unsigned int m_bits;
		const Gobby::SlotKeyGeneratorDone m_slot_done;

		gnutls_x509_privkey_t m_key;
		GError* m_error;
	};

	class Certgen: public Gobby::AsyncOperation
	{
	public:
		Certgen(gnutls_x509_privkey_t key,
		        const Gobby::SlotCertificateGeneratorDone& slot_done):
			m_key(key), m_slot_done(slot_done),
			m_cert(NULL), m_error(NULL)
		{
		}

		~Certgen()
		{
			if(m_cert != NULL)
				gnutls_x509_crt_deinit(m_cert);
			if(m_error != NULL)
				g_error_free(m_error);
		}

	protected:
		virtual void run()
		{
			InfCertUtilDescription desc;
			desc.validity = 365 * 24 * 3600;
			desc.dn_common_name = g_get_user_name();
			desc.san_dnsname = g_get_host_name();

			m_cert = inf_cert_util_create_self_signed_certificate(
				m_key, &desc, &m_error);
		}

		virtual void finish()
		{
			if(m_error != NULL)
			{
				m_slot_done(get_handle(), NULL, m_error);
			}
			else
			{
				gnutls_x509_crt_t cert = m_cert;
				m_cert = NULL;

				m_slot_done(get_handle(), cert, NULL);
			}
		}

	private:
		const gnutls_x509_privkey_t m_key;
		const Gobby::SlotCertificateGeneratorDone m_slot_done;

		gnutls_x509_crt_t m_cert;
		GError* m_error;
	};

	class DHgen: public Gobby::AsyncOperation
	{
	public:
		DHgen(unsigned int bits,
		      const Gobby::SlotDHParamsGeneratorDone& slot_done):
			m_bits(bits), m_slot_done(slot_done),
			m_dh_params(NULL), m_error(NULL)
		{
		}

		~DHgen()
		{
			if(m_dh_params != NULL)
				gnutls_dh_params_deinit(m_dh_params);
			if(m_error != NULL)
				g_error_free(m_error);
		}

	protected:
		virtual void run()
		{
			// TODO: Actually use the bits number here
			m_dh_params =
				inf_cert_util_create_dh_params(&m_error);
		}

		virtual void finish()
		{
			if(m_error != NULL)
			{
				m_slot_done(get_handle(), NULL, m_error);
			}
			else
			{
				gnutls_dh_params_t dh_params = m_dh_params;
				m_dh_params = NULL;

				m_slot_done(get_handle(), dh_params, NULL);
			}
		}

	private:
		const unsigned int m_bits;
		const Gobby::SlotDHParamsGeneratorDone m_slot_done;

		gnutls_dh_params_t m_dh_params;
		GError* m_error;
	};
}

std::auto_ptr<Gobby::KeyGeneratorHandle>
Gobby::create_key(gnutls_pk_algorithm_t algo,
                  unsigned int bits,
                  const SlotKeyGeneratorDone& done_slot)
{
	std::auto_ptr<AsyncOperation> operation(
		new Keygen(algo, bits, done_slot));
	return AsyncOperation::start(operation);
}

std::auto_ptr<Gobby::CertificateGeneratorHandle>
Gobby::create_self_signed_certificate(
	gnutls_x509_privkey_t key,
	const SlotCertificateGeneratorDone& done_slot)
{
	// TODO: Does this really need to be asynchronous? I don't think here
	// are any blocking calls involved...

	// TODO: We should make a copy of the key, so that the caller can
	// delete theirs.

	std::auto_ptr<AsyncOperation> operation(new Certgen(key, done_slot));
	return AsyncOperation::start(operation);
}

std::auto_ptr<Gobby::DHParamsGeneratorHandle>
Gobby::create_dh_params(unsigned int bits,
                        const SlotDHParamsGeneratorDone& done_slot)
{
	std::auto_ptr<AsyncOperation> operation(new DHgen(bits, done_slot));
	return AsyncOperation::start(operation);
}
