/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2015 Armin Burgmeier <armin@arbur.net>
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

std::unique_ptr<Gobby::KeyGeneratorHandle>
Gobby::create_key(gnutls_pk_algorithm_t algo,
                  unsigned int bits,
                  const SlotKeyGeneratorDone& done_slot)
{
	std::unique_ptr<AsyncOperation> operation(
		new Keygen(algo, bits, done_slot));
	return AsyncOperation::start(std::move(operation));
}

std::unique_ptr<Gobby::CertificateGeneratorHandle>
Gobby::create_self_signed_certificate(
	gnutls_x509_privkey_t key,
	const SlotCertificateGeneratorDone& done_slot)
{
	// TODO: Does this really need to be asynchronous? I don't think here
	// are any blocking calls involved...

	// TODO: We should make a copy of the key, so that the caller can
	// delete theirs.

	std::unique_ptr<AsyncOperation> operation(new Certgen(key, done_slot));
	return AsyncOperation::start(std::move(operation));
}

std::unique_ptr<Gobby::DHParamsGeneratorHandle>
Gobby::create_dh_params(unsigned int bits,
                        const SlotDHParamsGeneratorDone& done_slot)
{
	std::unique_ptr<AsyncOperation> operation(new DHgen(bits, done_slot));
	return AsyncOperation::start(std::move(operation));
}
