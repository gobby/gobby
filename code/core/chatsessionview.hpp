/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2010 Armin Burgmeier <armin@arbur.net>
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

#ifndef _GOBBY_CHATSESSIONVIEW_HPP_
#define _GOBBY_CHATSESSIONVIEW_HPP_

#include "core/sessionview.hpp"
#include "core/preferences.hpp"

#include <libinfgtk/inf-gtk-chat.h>
#include <libinfinity/common/inf-chat-session.h>

namespace Gobby
{

class ChatSessionView: public SessionView
{
public:
	ChatSessionView(InfChatSession* session, const Glib::ustring& title,
	                const Glib::ustring& path,
	                const Glib::ustring& hostname,
	                Preferences& preferences);

	// Override base class covariantly
	InfChatSession* get_session() { return INF_CHAT_SESSION(m_session); }
	InfGtkChat* get_chat() { return m_chat; }

	virtual InfUser* get_active_user() const;
	void set_active_user(InfUser* user);

protected:
	Preferences& m_preferences;

	InfGtkChat* m_chat;
};

}

#endif // _GOBBY_CHATSESSIONVIEW_HPP_
