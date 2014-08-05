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

#ifndef _GOBBY_FOLDER_COMMANDS_HPP_
#define _GOBBY_FOLDER_COMMANDS_HPP_

#include "core/folder.hpp"

#include <sigc++/trackable.h>

namespace Gobby
{

class FolderCommands: public sigc::trackable
{
public:
	FolderCommands(const Folder& folder);
	~FolderCommands();

protected:

	void on_document_added(SessionView& view);
	void on_document_removed(SessionView& view);
	void on_document_changed(SessionView* view);

	const Folder& m_folder;
	SessionView* m_current_view;

	class DocInfo;
	typedef std::map<SessionView*, DocInfo*> DocumentMap;
	DocumentMap m_doc_map;

	class TextDocInfo;
};

}

#endif // _GOBBY_FOLDER_COMMANDS_HPP_
