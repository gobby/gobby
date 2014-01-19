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

#include "core/noteplugin.hpp"

#include <libinftextgtk/inf-text-gtk-buffer.h>
#include <libinftext/inf-text-session.h>
#include <libinftext/inf-text-buffer.h>
#include <libinftext/inf-text-filesystem-format.h>

#include <libinfinity/server/infd-filesystem-storage.h>

#include <libinfinity/common/inf-chat-session.h>
#include <libinfinity/common/inf-session.h>
#include <libinfinity/common/inf-io.h>

#include <gtksourceview/gtksourcebuffer.h>

namespace
{
	InfTextBuffer*
	make_buffer(InfUserTable* user_table)
	{
		GtkSourceBuffer* textbuffer = gtk_source_buffer_new(NULL);

		// We never end this non-undoable action since we have our
		// own (collaborative) Undo implementanion, and we don't want
		// GtkSourceView to get in our way:
		gtk_source_buffer_begin_not_undoable_action(textbuffer);

		InfTextGtkBuffer* buffer =
			inf_text_gtk_buffer_new(GTK_TEXT_BUFFER(textbuffer),
			                        user_table);

		g_object_unref(textbuffer);
		return INF_TEXT_BUFFER(buffer);
	}

	InfSession*
	text_session_new(InfIo* io, InfCommunicationManager* manager,
	                 InfSessionStatus status,
	                 InfCommunicationGroup* sync_group,
	                 InfXmlConnection* sync_connection,
	                 gpointer user_data)
	{
		InfUserTable* user_table = inf_user_table_new();
		InfTextBuffer* buffer = make_buffer(user_table);

		InfTextSession* session =
			inf_text_session_new_with_user_table(
				manager, INF_TEXT_BUFFER(buffer), io,
				user_table,
				status,
				sync_group,
				sync_connection);

		g_object_unref(buffer);
		g_object_unref(user_table);

		return INF_SESSION(session);
	}

	InfSession*
	text_session_read(InfdStorage* storage,
	                  InfIo* io,
	                  InfCommunicationManager* manager,
	                  const gchar* path,
	                  gpointer user_data,
	                  GError** error)
	{
		InfUserTable* user_table = inf_user_table_new();
		InfTextBuffer* buffer = make_buffer(user_table);

		InfTextSession* session = 
			inf_text_filesystem_format_read(
				INFD_FILESYSTEM_STORAGE(storage),
				io,
				manager,
				path,
				user_table,
				buffer,
				error);

		g_object_unref(buffer);
		g_object_unref(user_table);

		return INF_SESSION(session);
	}

	gboolean
	text_session_write(InfdStorage* storage,
	                   InfSession* session,
	                   const gchar* path,
	                   gpointer user_data,
	                   GError** error)
	{
		return inf_text_filesystem_format_write(
			INFD_FILESYSTEM_STORAGE(storage),
			INF_TEXT_SESSION(session),
			path,
			error
		);
	}

	InfSession*
	chat_session_new(InfIo* io,
	                 InfCommunicationManager* manager,
	                 InfSessionStatus status,
	                 InfCommunicationGroup* sync_group,
	                 InfXmlConnection* sync_connection,
	                 gpointer user_data)
	{
		InfChatSession* session = inf_chat_session_new(
			manager, 256, status, sync_group, sync_connection);

		return INF_SESSION(session);
	}

	const InfcNotePlugin C_TEXT_PLUGIN =
	{
		NULL,
		"InfText",
		text_session_new
	};

	const InfcNotePlugin C_CHAT_PLUGIN =
	{
		NULL,
		"InfChat",
		chat_session_new
	};
	
	const InfdNotePlugin D_TEXT_PLUGIN =
	{
		NULL,
		"InfdFilesystemStorage",
		"InfText",
		text_session_new,
		text_session_read,
		text_session_write
	};
}

const InfcNotePlugin* Gobby::Plugins::C_TEXT = &C_TEXT_PLUGIN;
const InfcNotePlugin* Gobby::Plugins::C_CHAT = &C_CHAT_PLUGIN;
const InfdNotePlugin* Gobby::Plugins::D_TEXT = &D_TEXT_PLUGIN;
