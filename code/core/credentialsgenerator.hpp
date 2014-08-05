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
