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

#ifndef _GOBBY_KEYGEN_HPP_
#define _GOBBY_KEYGEN_HPP_

#include "util/asyncoperation.hpp"

#include <gnutls/gnutls.h>

namespace Gobby
{

typedef AsyncOperation::Handle KeygenHandle;

typedef sigc::slot<
	void, const KeygenHandle*, gnutls_x509_privkey_t, const GError*
> SlotKeygenDone;

std::auto_ptr<KeygenHandle> generate_key(gnutls_pk_algorithm_t algo,
                                         unsigned int bits,
                                         const SlotKeygenDone& slotDone);

}

#endif // _GOBBY_KEYGEN_HPP_
