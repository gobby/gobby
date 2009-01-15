/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008, 2009 Armin Burgmeier <armin@arbur.net>
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
	session_new(InfIo* io, InfCommunicationManager* manager,
	            InfCommunicationJoinedGroup* sync_group,
	            InfXmlConnection* sync_connection, gpointer user_data)
	{
		GtkSourceBuffer* textbuffer = gtk_source_buffer_new(NULL);
		// We never end this non-undoable action since we have our
		// own (collaborative) Undo implementanion, and we don't want
		// GtkSourceView to get in our way:
		gtk_source_buffer_begin_not_undoable_action(textbuffer);

		InfUserTable* user_table = inf_user_table_new();
		InfTextGtkBuffer* buffer =
			inf_text_gtk_buffer_new(GTK_TEXT_BUFFER(textbuffer),
			                        user_table);
		InfTextSession* session =
			inf_text_session_new_with_user_table(
				manager, INF_TEXT_BUFFER(buffer), io,
				user_table,
				INF_COMMUNICATION_GROUP(sync_group),
				sync_connection);
		return INF_SESSION(session);
	}

	const InfcNotePlugin TEXT_PLUGIN =
	{
		NULL,
		"InfText",
		session_new
	};
}

const InfcNotePlugin* Gobby::Plugins::TEXT = &TEXT_PLUGIN;
