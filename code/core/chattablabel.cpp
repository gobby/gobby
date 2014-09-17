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

#include "core/chattablabel.hpp"

Gobby::ChatTabLabel::ChatTabLabel(Folder& folder, ChatSessionView& view,
                                  bool always_show_close_button):
	TabLabel(folder, view,
	         always_show_close_button ?
	         "chat" :
	         "network-idle"),
	m_always_show_close_button(always_show_close_button)
{
	if(!m_always_show_close_button)
		m_button.hide(); // Only show when disconnected

	InfChatBuffer* buffer = INF_CHAT_BUFFER(
		inf_session_get_buffer(INF_SESSION(view.get_session())));

	m_add_message_handle = g_signal_connect_after(
		G_OBJECT(buffer), "add-message",
		G_CALLBACK(on_add_message_static), this);
}

Gobby::ChatTabLabel::~ChatTabLabel()
{
	InfChatBuffer* buffer = INF_CHAT_BUFFER(
		inf_session_get_buffer(INF_SESSION(m_view.get_session())));

	g_signal_handler_disconnect(buffer, m_add_message_handle);
}

void Gobby::ChatTabLabel::on_notify_subscription_group()
{
	InfSession* session = INF_SESSION(m_view.get_session());
	if(inf_session_get_subscription_group(session) != NULL &&
	   !m_always_show_close_button)
	{
		m_button.hide();
	}
	else
	{
		m_button.show();
	}
}

void Gobby::ChatTabLabel::on_changed(InfUser* author)
{
	if(!m_changed)
	{
		InfSession* session = INF_SESSION(m_view.get_session());
		if(inf_session_get_status(session) == INF_SESSION_RUNNING)
			set_changed();
	}
}
