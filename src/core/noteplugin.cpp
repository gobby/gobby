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

#include "core/noteplugin.hpp"

#include <libinftextgtk/inf-text-gtk-buffer.h>
#include <libinftext/inf-text-session.h>
#include <libinftext/inf-text-buffer.h>

#include <libinfinity/common/inf-session.h>
#include <libinfinity/common/inf-io.h>

#include <gtksourceview/gtksourcebuffer.h>

namespace
{
	InfSession*
	session_new(InfIo* io, InfConnectionManager* manager,
	            InfConnectionManagerGroup* sync_group,
	            InfXmlConnection* sync_connection)
	{
		GtkSourceBuffer* textbuffer = gtk_source_buffer_new(NULL);
		InfUserTable* user_table = inf_user_table_new();
		InfTextGtkBuffer* buffer =
			inf_text_gtk_buffer_new(GTK_TEXT_BUFFER(textbuffer),
			                        user_table);
		InfTextSession* session =
			inf_text_session_new_with_user_table(
				manager, INF_TEXT_BUFFER(buffer), io,
				user_table, sync_group, sync_connection);
		return INF_SESSION(session);
	}

	const InfcNotePlugin TEXT_PLUGIN =
	{
		"InfText",
		session_new
	};
}

const InfcNotePlugin* Gobby::Plugins::TEXT = &TEXT_PLUGIN;
