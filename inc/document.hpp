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

#ifndef _GOBBY_DOCUMENT_HPP_
#define _GOBBY_DOCUMENT_HPP_

#include <gtkmm/scrolledwindow.h>
#include <gtkmm/textview.h>
#include <obby/document.hpp>

#include "features.hpp"
#ifdef WITH_GTKSOURCEVIEW
#include "sourceview/sourcelanguagesmanager.hpp"
#include "sourceview/sourceview.hpp"
#endif

namespace Gobby
{

class Document : public Gtk::ScrolledWindow
{
public:
	typedef std::map<Glib::ustring, Glib::ustring> MimeMap;

	Document(obby::document& doc);
	virtual ~Document();

	const obby::document& get_document() const;
	obby::document& get_document();

protected:
	void on_insert(const Gtk::TextBuffer::iterator& begin,
	               const Glib::ustring& text,
		       int bytes);
	void on_erase(const Gtk::TextBuffer::iterator& begin,
	              const Gtk::TextBuffer::iterator& end);

	void on_obby_insert(const obby::insert_record& record);
	void on_obby_delete(const obby::delete_record& record);

	obby::document& m_doc;
#ifdef WITH_GTKSOURCEVIEW
	Gtk::SourceView m_view;
	Glib::RefPtr<Gtk::SourceLanguagesManager> m_lang_manager;
#else
	Gtk::TextView m_view;
#endif
	bool m_editing;

public:
#ifdef WITH_GTKSOURCEVIEW
	static const MimeMap& create_mime_map();
	static const MimeMap& m_mime_map;
#endif

};

}

#endif // _GOBBY_DOCUMENT_HPP_
