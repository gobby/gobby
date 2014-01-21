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

#ifndef _GOBBY_USER_JOIN_COMMANDS_HPP_
#define _GOBBY_USER_JOIN_COMMANDS_HPP_

#include "core/foldermanager.hpp"
#include "core/preferences.hpp"

#include <sigc++/trackable.h>

namespace Gobby
{

class UserJoinCommands: public sigc::trackable
{
public:
	UserJoinCommands(FolderManager& folder_manager,
	                 const Preferences& preferences);
	~UserJoinCommands();

protected:
	void on_document_added(InfBrowser* browser,
	                       const InfBrowserIter* iter,
	                       InfSessionProxy* proxy,
	                       Folder& folder,
                               SessionView& view);
	void on_document_removed(InfBrowser* browser,
	                         const InfBrowserIter* iter,
	                         InfSessionProxy* proxy,
	                         Folder& folder,
	                         SessionView& view);

	const Preferences& m_preferences;

	class UserJoinInfo;
	typedef std::map<InfSessionProxy*, UserJoinInfo*> UserJoinMap;
	UserJoinMap m_user_join_map;
};

}

#endif // _GOBBY_USER_JOIN_COMMANDS_HPP_
