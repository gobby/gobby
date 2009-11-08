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

#ifndef _GOBBY_USER_JOIN_COMMANDS_HPP_
#define _GOBBY_USER_JOIN_COMMANDS_HPP_

#include "commands/subscription-commands.hpp"
#include "core/preferences.hpp"

#include <sigc++/trackable.h>

namespace Gobby
{

class UserJoinCommands: public sigc::trackable
{
public:
	UserJoinCommands(SubscriptionCommands& subscription_commands,
	                 const Preferences& preferences);
	~UserJoinCommands();

protected:
	void on_subscribe_session(InfcSessionProxy* proxy,
	                          Folder& folder, SessionView& view);
	void on_unsubscribe_session(InfcSessionProxy* proxy,
	                            Folder& folder, SessionView& view);

	const Preferences& m_preferences;

	class UserJoinInfo;
	typedef std::map<InfcSessionProxy*, UserJoinInfo*> UserJoinMap;
	UserJoinMap m_user_join_map;
};

}

#endif // _GOBBY_USER_JOIN_COMMANDS_HPP_
