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
