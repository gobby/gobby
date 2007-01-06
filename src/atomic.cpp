/* gobby - A GTKmm driven libobby client
 * Copyright (C) 2005 0x539 dev group
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

#include <glib/gatomic.h>
#include "atomic.hpp"

#define g_atomic_int_set(atomic, val) \
	(g_atomic_int_add( (atomic), (val) - g_atomic_int_get(atomic) ))

Gobby::Atomic::Atomic(bool value)
 : m_val(value ? 1 : 0)
{
}

Gobby::Atomic& Gobby::Atomic::operator=(const Atomic& other)
{
	g_atomic_int_set(&m_val, other.m_val);
	return *this;
}

Gobby::Atomic& Gobby::Atomic::operator=(bool value)
{
	g_atomic_int_set(&m_val, value ? 1 : 0);
	return *this;
}

Gobby::Atomic::operator bool() const
{
	return g_atomic_int_get(&m_val) != 0;
}
