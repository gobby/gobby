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

#include "core/noteplugin.hpp"

#include <libinftextgtk/inf-text-gtk-buffer.h>
#include <libinftext/inf-text-session.h>
#include <libinftext/inf-text-buffer.h>
#include <libinftext/inf-text-filesystem-format.h>

#include <libinfinity/server/infd-filesystem-storage.h>
#include <libinfinity/server/infd-chat-filesystem-format.h>

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

		const gboolean result = inf_text_filesystem_format_read(
			INFD_FILESYSTEM_STORAGE(storage),
			path, user_table, buffer, error);

		InfTextSession* session = NULL;
		if(result)
		{
			session = inf_text_session_new_with_user_table(
				manager, buffer, io, user_table,
				INF_SESSION_RUNNING, NULL, NULL);
		}

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
			path,
			inf_session_get_user_table(session),
			INF_TEXT_BUFFER(inf_session_get_buffer(session)),
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
		InfChatBuffer* buffer = inf_chat_buffer_new(256);
		InfChatSession* session = inf_chat_session_new(
			manager, buffer, status, sync_group, sync_connection);
		g_object_unref(buffer);

		return INF_SESSION(session);
	}

	InfSession*
	chat_session_read(InfdStorage* storage,
	                  InfIo* io,
	                  InfCommunicationManager* manager,
	                  const gchar* path,
	                  gpointer user_data,
	                  GError** error)
	{
		InfChatBuffer* buffer = inf_chat_buffer_new(256);

		const gboolean result = infd_chat_filesystem_format_read(
			INFD_FILESYSTEM_STORAGE(storage),
			path, buffer, error);

		InfChatSession* session = NULL;
		if(result)
		{
			session = inf_chat_session_new(
				manager, buffer,
				INF_SESSION_RUNNING, NULL, NULL);
		}

		g_object_unref(buffer);
		return INF_SESSION(session);
	}

	gboolean
	chat_session_write(InfdStorage* storage,
	                   InfSession* session,
	                   const gchar* path,
	                   gpointer user_data,
	                   GError** error)
	{
		return infd_chat_filesystem_format_write(
			INFD_FILESYSTEM_STORAGE(storage),
			path,
			INF_CHAT_BUFFER(inf_session_get_buffer(session)),
			error
		);
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

	const InfdNotePlugin D_CHAT_PLUGIN =
	{
		NULL,
		"InfdFilesystemStorage",
		"InfChat",
		chat_session_new,
		chat_session_read,
		chat_session_write
	};
}

const InfcNotePlugin* Gobby::Plugins::C_TEXT = &C_TEXT_PLUGIN;
const InfcNotePlugin* Gobby::Plugins::C_CHAT = &C_CHAT_PLUGIN;
const InfdNotePlugin* Gobby::Plugins::D_TEXT = &D_TEXT_PLUGIN;
const InfdNotePlugin* Gobby::Plugins::D_CHAT = &D_CHAT_PLUGIN;
