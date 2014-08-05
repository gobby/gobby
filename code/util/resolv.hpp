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

#ifndef _GOBBY_RESOLV_HPP_
#define _GOBBY_RESOLV_HPP_

#include "util/asyncoperation.hpp"

#include <libinfinity/common/inf-ip-address.h>

#include <glibmm/ustring.h>
#include <sigc++/sigc++.h>

#include <stdexcept>
#include <memory>

namespace Gobby
{

typedef AsyncOperation::Handle ResolvHandle;

typedef sigc::slot<void, const ResolvHandle*, InfIpAddress*, guint>
	SlotResolvDone;
typedef sigc::slot<void, const ResolvHandle*, const std::runtime_error&>
	SlotResolvError;

std::auto_ptr<ResolvHandle> resolve(const Glib::ustring& hostname,
                                    const Glib::ustring& service,
                                    const SlotResolvDone& done_slot,
                                    const SlotResolvError& error_slot);

}

#endif // _GOBBY_RESOLV_HPP_
