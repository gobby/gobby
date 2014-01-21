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

#ifndef _GOBBY_SYNCHRONIZATION_COMMANDS_HPP_
#define _GOBBY_SYNCHRONIZATION_COMMANDS_HPP_

#include <sigc++/trackable.h>

#include "core/folder.hpp"

namespace Gobby
{

class SynchronizationCommands: public sigc::trackable
{
public:
	SynchronizationCommands(const Folder& text_folder,
	                        const Folder& chat_folder);
	~SynchronizationCommands();

protected:
	class SyncInfo;

	void on_document_added(SessionView& view);
	void on_document_removed(SessionView& view);

	void on_synchronization_failed(InfSession* session,
	                               InfXmlConnection* connection,
	                               const GError* error);
	void on_synchronization_complete(InfSession* session,
	                                 InfXmlConnection* connection);

	typedef std::map<InfSession*, SyncInfo*> SyncMap;
	SyncMap m_sync_map;
};

}

#endif // _GOBBY_SYNCHRONIZATION_COMMANDS_HPP_
