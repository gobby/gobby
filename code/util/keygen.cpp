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

#include "util/keygen.hpp"

#include <libinfinity/common/inf-cert-util.h>

#include <gnutls/x509.h>

namespace
{
	class Keygen: public Gobby::AsyncOperation
	{
	public:
		Keygen(gnutls_pk_algorithm_t algo,
		       unsigned int bits,
		       const Gobby::SlotKeygenDone& slot_done):
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
		virtual void run();
		virtual void finish();

	private:
		const gnutls_pk_algorithm_t m_algo;
		const unsigned int m_bits;
		const Gobby::SlotKeygenDone m_slot_done;

		gnutls_x509_privkey_t m_key;
		GError* m_error;
	};

	void Keygen::run()
	{
		m_key = inf_cert_util_create_private_key(
			m_algo, m_bits, &m_error);
	}

	void Keygen::finish()
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
}

std::auto_ptr<Gobby::KeygenHandle>
Gobby::generate_key(gnutls_pk_algorithm_t algo,
                    unsigned int bits,
                    const SlotKeygenDone& slotDone)
{
	std::auto_ptr<AsyncOperation> operation(
		new Keygen(algo, bits, slotDone));
	return AsyncOperation::start(operation);
}
