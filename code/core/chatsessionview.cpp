/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2015 Armin Burgmeier <armin@arbur.net>
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

	gtk_grid_attach_next_to(GTK_GRID(gobj()), GTK_WIDGET(m_chat),
	                        GTK_WIDGET(m_info_frame.gobj()),
	                        GTK_POS_BOTTOM, 1, 1);
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
	active_user_changed(user);
}
