/* gobby - A GTKmm driven libobby client
 * Copyright (C) 2005 - 2008 0x539 dev group
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
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef _GOBBY_RESOLV_HPP_
#define _GOBBY_RESOLV_HPP_

#include <libinfinity/common/inf-ip-address.h>

#include <glibmm/ustring.h>
#include <sigc++/sigc++.h>
#include <stdexcept>

namespace Gobby
{

struct ResolvHandle;

typedef sigc::slot<void, InfIpAddress*, guint> SlotResolvDone;
typedef sigc::slot<void, const std::runtime_error&> SlotResolvError;

ResolvHandle* resolve(const Glib::ustring& hostname,
                      const Glib::ustring& service,
                      const SlotResolvDone& done_slot,
                      const SlotResolvError& error_slot);

void cancel(ResolvHandle* handle);

}

#endif // _GOBBY_RESOLV_HPP_
