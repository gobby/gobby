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

#ifndef _GOBBY_SUBSCRIPTION_COMMANDS_HPP_
#define _GOBBY_SUBSCRIPTION_COMMANDS_HPP_

#include "core/folder.hpp"

namespace Gobby
{

// This class does not do much anymore. All it does is showing an error
// message when a session loses the connection to its publisher, and resets
// the active user to NULL.
// TODO: The name of this class is a bit misleading now.
class SubscriptionCommands: public sigc::trackable
{
public:
	SubscriptionCommands(const Folder& text_folder,
	                     const Folder& chat_folder);
	~SubscriptionCommands();

protected:
	void on_text_document_added(SessionView& view);
	void on_chat_document_added(SessionView& view);
	void on_document_removed(SessionView& view);

	void on_notify_subscription_group(InfSession* session);

	const Folder& m_text_folder;
	const Folder& m_chat_folder;

	class SessionInfo;
	typedef std::map<InfSession*, SessionInfo*> SessionMap;
	SessionMap m_session_map;
};

}

#endif // _GOBBY_SUBSCRIPTION_COMMANDS_HPP_
