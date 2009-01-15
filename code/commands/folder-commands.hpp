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

#ifndef _GOBBY_FOLDER_COMMANDS_HPP_
#define _GOBBY_FOLDER_COMMANDS_HPP_

#include "core/folder.hpp"

#include <sigc++/trackable.h>

namespace Gobby
{

class FolderCommands: public sigc::trackable
{
public:
	FolderCommands(Folder& folder);
	~FolderCommands();

protected:

	void on_document_added(DocWindow& document);
	void on_document_removed(DocWindow& document);
	void on_document_changed(DocWindow* document);

	Folder& m_folder;

	DocWindow* m_current_document;

	class DocInfo;
	typedef std::map<DocWindow*, DocInfo*> DocumentMap;
	DocumentMap m_doc_map;
};

}
	
#endif // _GOBBY_FOLDER_COMMANDS_HPP_
