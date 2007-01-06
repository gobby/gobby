/* gobby - A GTKmm driven libobby client
 * Copyright (C) 2005 0x539 dev group
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

#ifndef _GOBBY_DOCWINDOW_HPP_
#define _GOBBY_DOCWINDOW_HPP_

#include <gtkmm/scrolledwindow.h>
#include <obby/local_document_info.hpp>

#include "features.hpp"
#include "document.hpp"

namespace Gobby
{

class Folder;

class DocWindow : public Gtk::ScrolledWindow
{
public:
	typedef Document::signal_cursor_moved_type signal_cursor_moved_type;
	typedef Document::signal_content_changed_type
		signal_content_changed_type;
#ifdef WITH_GTKSOURCEVIEW
	typedef Document::signal_language_changed_type
		signal_language_changed_type;
#endif

	DocWindow(obby::local_document_info& doc, const Folder& folder,
	          const Preferences& preferences);
	virtual ~DocWindow();

	const Document& get_document() const;
	Document& get_document();

	/** Calls from the folder.
	 */
	void obby_user_join(obby::user& user);
	void obby_user_part(obby::user& user);

protected:
	Document m_doc;
};

}

#endif // _GOBBY_DOCWINDOW_HPP_
