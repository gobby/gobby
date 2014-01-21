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
