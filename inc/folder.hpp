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

#ifndef _GOBBY_FOLDER_HPP_
#define _GOBBY_FOLDER_HPP_

#include <sigc++/signal.h>
#include <gtkmm/notebook.h>
#include <obby/user.hpp>
#include <obby/document.hpp>

#include "document.hpp"

#ifdef WITH_GTKSOURCEVIEW
# include "sourceview/sourcelanguage.hpp"
# include "sourceview/sourcelanguagesmanager.hpp"
# include "mimemap.hpp"
#endif

namespace Gobby
{

/** Thing containing multiple documents.
 */
	
class Folder : public Gtk::Notebook
{
public:
	typedef sigc::signal<void, Document&> signal_document_update_type;

	Folder();
	~Folder();

#ifdef WITH_GTKSOURCEVIEW
	// Access to the mime map
	const MimeMap& get_mime_map() const;

	Glib::RefPtr<Gtk::SourceLanguagesManager> get_lang_manager() const;
#endif

	// Calls from the window
	void obby_start();
	void obby_end();
	void obby_user_join(obby::user& user);
	void obby_user_part(obby::user& user);
	void obby_document_insert(obby::document& document);
	void obby_document_remove(obby::document& document);

	signal_document_update_type document_update_event() const;

protected:
	// Signal handlers
	virtual void on_switch_page(GtkNotebookPage* page, guint page_num);
	virtual void on_document_update(Document& document);

	signal_document_update_type m_signal_document_update;

#ifdef WITH_GTKSOURCEVIEW
	Glib::RefPtr<Gtk::SourceLanguagesManager> m_lang_manager;
	MimeMap m_mime_map;
#endif
};

}

#endif // _GOBBY_FOLDER_HPP_
