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
