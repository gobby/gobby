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

#ifndef _GOBBY_CREDENTIALS_GENERATOR_HPP_
#define _GOBBY_CREDENTIALS_GENERATOR_HPP_

#include "util/asyncoperation.hpp"

#include <gnutls/x509.h>

namespace Gobby
{

typedef AsyncOperation::Handle KeyGeneratorHandle;
typedef sigc::slot<
	void, const KeyGeneratorHandle*, gnutls_x509_privkey_t, const GError*
> SlotKeyGeneratorDone;

typedef AsyncOperation::Handle CertificateGeneratorHandle;
typedef sigc::slot<
	void,
	const CertificateGeneratorHandle*, gnutls_x509_crt_t, const GError*
> SlotCertificateGeneratorDone;

typedef AsyncOperation::Handle DHParamsGeneratorHandle;
typedef sigc::slot<
	void,
	const DHParamsGeneratorHandle*, gnutls_dh_params_t, const GError*
> SlotDHParamsGeneratorDone;

std::auto_ptr<KeyGeneratorHandle>
create_key(gnutls_pk_algorithm_t algo,
           unsigned int bits,
           const SlotKeyGeneratorDone& done_slot);

std::auto_ptr<CertificateGeneratorHandle>
create_self_signed_certificate(gnutls_x509_privkey_t key,
                               const SlotCertificateGeneratorDone& done_slot);

std::auto_ptr<DHParamsGeneratorHandle>
create_dh_params(unsigned int bits,
                 const SlotDHParamsGeneratorDone& done_slot);

}

#endif // _GOBBY_CREDENTIALS_GENERATOR_HPP_
