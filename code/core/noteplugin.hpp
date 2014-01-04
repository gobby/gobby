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

#ifndef _GOBBY_NOTEPLUGIN_HPP_
#define _GOBBY_NOTEPLUGIN_HPP_

#include <libinfinity/client/infc-note-plugin.h>

namespace Gobby
{
	namespace Plugins
	{
		extern const InfcNotePlugin* TEXT;
		extern const InfcNotePlugin* CHAT;
	}
}

#endif // _GOBBY_NOTEPLUGIN_HPP_
