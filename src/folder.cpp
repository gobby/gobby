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

#include "document.hpp"
#include "folder.hpp"

Gobby::Folder::Folder()
 : Gtk::Notebook()
#ifdef WITH_GTKSOURCEVIEW
   , m_lang_manager(Gtk::SourceLanguagesManager::create() )
#endif
{
}

Gobby::Folder::~Folder()
{
	// Delete documents
	for(int i = 0; i < get_n_pages(); ++ i)
		delete get_nth_page(i);
}

const Gobby::MimeMap& Gobby::Folder::get_mime_map() const
{
	return m_mime_map;
}

#ifdef WITH_GTKSOURCEVIEW
Glib::RefPtr<Gtk::SourceLanguagesManager>
Gobby::Folder::get_lang_manager() const
{
	return m_lang_manager;
}
#endif

void Gobby::Folder::obby_start()
{
	// Remove existing pages
	while(get_n_pages() )
	{
		delete get_nth_page(0);
		remove_page(0);
	}

	set_sensitive(true);
}

void Gobby::Folder::obby_end()
{
	// TODO: Just remove the editable-attribute to allow the user to still
	// save the documents.
	set_sensitive(false);
}

void Gobby::Folder::obby_user_join(obby::user& user)
{
}

void Gobby::Folder::obby_user_part(obby::user& user)
{
}

void Gobby::Folder::obby_document_insert(obby::document& document)
{
	Document* new_doc = new Document(document, *this);
	new_doc->update_event().connect(
		sigc::bind(
			sigc::mem_fun(*this, &Folder::on_document_update),
			sigc::ref(*new_doc)
		)
	);

	append_page(*new_doc, document.get_title());
	new_doc->show_all();
}

void Gobby::Folder::obby_document_remove(obby::document& document)
{
	for(int i = 0; i < get_n_pages(); ++ i)
	{
		Widget* doc = get_nth_page(i);
		if(&static_cast<Document*>(doc)->get_document() == &document)
		{
			remove_page(i);
			delete doc;
			break;
		}
	}
}

Gobby::Folder::signal_document_update_type
Gobby::Folder::document_update_event() const
{
	return m_signal_document_update;
}

void Gobby::Folder::on_switch_page(GtkNotebookPage* page, guint page_num)
{
	m_signal_document_update.emit(
		*static_cast<Document*>(get_nth_page(page_num))
	);
	Gtk::Notebook::on_switch_page(page, page_num);
}

void Gobby::Folder::on_document_update(Document& document)
{
	if(get_current_page() == page_num(document) )
		m_signal_document_update.emit(document);
}

