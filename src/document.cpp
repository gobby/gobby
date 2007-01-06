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

#include <obby/client_document.hpp>
#ifdef WITH_GTKSOURCEVIEW
#include "sourceview/sourcelanguagesmanager.hpp"
#endif
#include "document.hpp"
#include "folder.hpp"

#ifdef WITH_GTKSOURCEVIEW
const Gobby::Document::MimeMap& Gobby::Document::m_mime_map =
	Gobby::Document::create_mime_map();
#endif

Gobby::Document::Document(obby::document& doc, const Folder& folder)
 : Gtk::ScrolledWindow(), m_doc(doc), m_folder(folder), m_editing(true)
#ifdef WITH_GTKSOURCEVIEW
   ,m_lang_manager(Gtk::SourceLanguagesManager::create() )
#endif
{
#ifdef WITH_GTKSOURCEVIEW
	m_view.set_show_line_numbers(true);
	Glib::RefPtr<Gtk::SourceBuffer> buf = m_view.get_buffer();
#else
	Glib::RefPtr<Gtk::TextBuffer> buf = m_view.get_buffer();
#endif

	// Set monospaced font
	Pango::FontDescription desc;
	desc.set_family("monospace");
	m_view.modify_font(desc);


#ifdef WITH_GTKSOURCEVIEW
	// Set source language by file extension
	Glib::ustring title = doc.get_title();
	Glib::ustring::size_type pos = title.rfind('.');
	if(pos != Glib::ustring::npos)
	{
		Glib::ustring extension = title.substr(pos + 1);
		MimeMap::const_iterator iter = m_mime_map.find(extension);
		if(iter != m_mime_map.end() )
		{
			Glib::ustring mime = iter->second;
			Glib::RefPtr<Gtk::SourceLanguage> language = 
				m_lang_manager->get_language_from_mime_type(
					mime
				);
			if(language)
			{
				buf->set_language(language);
			}
			else
			{
				g_warning("Could not find syntax file for file "
				          "extension %s (mime-type %s)",
				          extension.c_str(), mime.c_str() );
			}
		}
		else
		{
			g_warning("Could not detect file type of file "
			          "extension '%s'", extension.c_str() );
		}
	}

	buf->set_highlight(true);
#endif

	// Textbuffer signal handlers
	buf->signal_insert().connect(
		sigc::mem_fun(*this, &Document::on_insert_before), false);
	buf->signal_erase().connect(
		sigc::mem_fun(*this, &Document::on_erase_before), false);
	buf->signal_insert().connect(
		sigc::mem_fun(*this, &Document::on_insert_after), true);
	buf->signal_erase().connect(
		sigc::mem_fun(*this, &Document::on_erase_after), true);
	buf->signal_mark_set().connect(
		sigc::mem_fun(*this, &Document::on_cursor_changed) );

	// Obby signal handlers
	doc.insert_event().before().connect(
		sigc::mem_fun(*this, &Document::on_obby_insert) );
	doc.delete_event().before().connect(
		sigc::mem_fun(*this, &Document::on_obby_delete) );

	// Set initial text
	buf->set_text(doc.get_whole_buffer() );
	m_editing = false;
	
	set_shadow_type(Gtk::SHADOW_IN);
	set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	add(m_view);
}

Gobby::Document::~Document()
{
}

const obby::document& Gobby::Document::get_document() const
{
	return m_doc;
}

obby::document& Gobby::Document::get_document()
{
	return m_doc;
}

Gobby::Document::signal_update_type Gobby::Document::update_event() const
{
	return m_signal_update;
}
	
void Gobby::Document::get_cursor_position(unsigned int& row,
                                          unsigned int& col)
{
	// Get insert mark
	Glib::RefPtr<Gtk::TextBuffer::Mark> mark =
		m_view.get_buffer()->get_mark("insert");

	// Get corresponding iterator
	// Gtk::TextBuffer::Mark::get_iter is not const. Why not? It prevents
	// this function from being const.
	const Gtk::TextBuffer::iterator iter = mark->get_iter();

	// Read line and column
	row = iter.get_line();
	col = iter.get_line_offset();
}

unsigned int Gobby::Document::get_unsynced_changes_count() const
{
	obby::client_document* doc = 
		dynamic_cast<obby::client_document*>(&m_doc);

	// Changes in Server/Host documents are always synced
	if(doc == NULL)
		return 0;

	return doc->get_unsynced_changes_count();
}

#ifdef WITH_GTKSOURCEVIEW
Glib::RefPtr<Gtk::SourceLanguage> Gobby::Document::get_language() const
{
	return m_view.get_buffer()->get_language();
}
#endif

void Gobby::Document::on_insert_before(const Gtk::TextBuffer::iterator& begin,
                                       const Glib::ustring& text,
                                       int bytes)
{
	if(m_editing) return;
	m_editing = true;
	
	m_doc.insert(
		m_doc.coord_to_position(
			begin.get_line(),
			begin.get_line_index()
		),
		text
	);

	m_editing = false;
}

void Gobby::Document::on_erase_before(const Gtk::TextBuffer::iterator& begin,
                                      const Gtk::TextBuffer::iterator& end)
{
	if(m_editing) return;
	m_editing = true;

	m_doc.erase(
		m_doc.coord_to_position(
			begin.get_line(),
			begin.get_line_index()
		),
		m_doc.coord_to_position(
			end.get_line(),
			end.get_line_index()
		)
	);

	m_editing = false;
}

void Gobby::Document::on_obby_insert(const obby::insert_record& record)
{
	if(m_editing) return;
	m_editing = true;

	Glib::RefPtr<Gtk::TextBuffer> buffer = m_view.get_buffer();

	unsigned int row, col;
	m_doc.position_to_coord(record.get_position(), row, col);
	buffer->insert(
		buffer->get_iter_at_line_index(row, col),
		record.get_text()
	);

	m_editing = false;
}

void Gobby::Document::on_obby_delete(const obby::delete_record& record)
{
	if(m_editing) return;
	m_editing = true;

	Glib::RefPtr<Gtk::TextBuffer> buffer = m_view.get_buffer();

	unsigned int brow, bcol, erow, ecol;
	m_doc.position_to_coord(record.get_begin(), brow, bcol);
	m_doc.position_to_coord(record.get_end(), erow, ecol);

	buffer->erase(
		buffer->get_iter_at_line_index(brow, bcol),
		buffer->get_iter_at_line_index(erow, ecol)
	);
	m_editing = false;
}

void Gobby::Document::on_insert_after(const Gtk::TextBuffer::iterator& begin,
                                      const Glib::ustring& text,
                                      int bytes)
{
	// Document changed: Update statusbar
	m_signal_update.emit();
}

void Gobby::Document::on_erase_after(const Gtk::TextBuffer::iterator& begin,
                                     const Gtk::TextBuffer::iterator& end)
{
	// Document changed: Update statusbar
	m_signal_update.emit();
}

void Gobby::Document::on_cursor_changed(
	const Gtk::TextBuffer::iterator& location,
	const Glib::RefPtr<Gtk::TextBuffer::Mark>& mark
)
{
	// Insert mark changed position: Update status bar
	if(mark->get_name() == "insert")
		m_signal_update.emit();
}

#ifdef WITH_GTKSOURCEVIEW
const Gobby::Document::MimeMap& Gobby::Document::create_mime_map()
{
	static MimeMap map;

	// Translates file extension to mime type
	map["ada"] = "text/x-ada";
	map["c"] = "text/x-c";
	map["h"] = "text/x-c++";
	map["hh"] = "text/x-c++";
	map["cpp"] = "text/x-c++";
	map["hpp"] = "text/x-c++";
	map["cc"] = "text/x-c++";
	map["css"] = "text/css";
	map["cs"] = "text/x-csharp";
	map["diff"] = "text/x-diff";
	map["f"] = "text/x-fortran";
	map["f77"] = "text/x-fortran";
	map["hs"] = "text/x-haskell";
	map["htm"] = "text/html";
	map["html"] = "text/html";
	map["xhtml"] = "text/html";
	// Wi geth IDL?
	map["java"] = "text/x-java";
	map["js"] = "text/x-javascript";
	map["tex"] = "text/x-tex";
	map["latex"] = "text/x-tex";
	map["lua"] = "text/x-lua";
	// Wi geth MSIL?
	map["dpr"] = "text/x-pascal";
	map["pas"] = "text/x-pascal";
	map["pl"] = "text/x-perl";
	map["pm"] = "text/x-perl";
	map["php"] = "text/x-php";
	map["php3"] = "text/x-php";
	map["php4"] = "text/x-php";
	map["php5"] = "text/x-php";
	map["po"] = "text/x-gettext-translation";
	map["py"] = "text/x-python";
	map["rb"] = "text/x-ruby";
	map["sql"] = "text/x-sql";
	// Wi geth texinfo?
	// Wi geth vb.NET?
	// Wi geth verilog?
	map["xml"] = "text/xml";

	return map;
}
#endif
