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

#include "core/chatsessionview.hpp"

Gobby::ChatSessionView::ChatSessionView(InfChatSession* session,
                                        const Glib::ustring& title,
                                        const Glib::ustring& path,
                                        const Glib::ustring& hostname,
                                        Preferences& preferences):
	SessionView(INF_SESSION(session), title, path, hostname),
	m_preferences(preferences), m_chat(INF_GTK_CHAT(inf_gtk_chat_new()))
{
	inf_gtk_chat_set_session(m_chat, session);
	gtk_widget_show(GTK_WIDGET(m_chat));

	gtk_box_pack_start(GTK_BOX(gobj()), GTK_WIDGET(m_chat),
	                   TRUE, TRUE, 0);
}

InfUser* Gobby::ChatSessionView::get_active_user() const
{
	return inf_gtk_chat_get_active_user(m_chat);
}

void Gobby::ChatSessionView::set_active_user(InfUser* user)
{
	g_assert(
		user == NULL ||
		inf_user_table_lookup_user_by_id(
			inf_session_get_user_table(INF_SESSION(m_session)),
			inf_user_get_id(INF_USER(user)))
		== INF_USER(user));

	inf_gtk_chat_set_active_user(m_chat, user);
	m_signal_active_user_changed.emit(user);
}
