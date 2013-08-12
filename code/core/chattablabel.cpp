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

#include "core/chattablabel.hpp"
#include "core/iconmanager.hpp"
#include <gtkmm/stock.h>

Gobby::ChatTabLabel::ChatTabLabel(Folder& folder, ChatSessionView& view,
                                  bool always_show_close_button):
	TabLabel(folder, view,
	         always_show_close_button ?
	         Gobby::IconManager::STOCK_CHAT :
	         Gtk::Stock::NETWORK),
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
