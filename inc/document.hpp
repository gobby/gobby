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

class Folder;

class Document : public Gtk::ScrolledWindow
{
public:
	typedef sigc::signal<void> signal_update_type;

	Document(obby::document& doc, const Folder& folder);
	virtual ~Document();

	const obby::document& get_document() const;
	obby::document& get_document();

	// Statusbar information
	void get_cursor_position(unsigned int& row, unsigned int& col);
	unsigned int get_unsynced_changes_count() const;
#ifdef WITH_GTKSOURCEVIEW
	Glib::RefPtr<Gtk::SourceLanguage> get_language() const;
#endif

	/** Signal which will be emitted if the document gets updated in a way
	 * that is interesting for the status bar.
	 */
	signal_update_type update_event() const;

protected:
	void on_insert_before(const Gtk::TextBuffer::iterator& begin,
	                      const Glib::ustring& text,
		              int bytes);
	void on_erase_before(const Gtk::TextBuffer::iterator& begin,
	                     const Gtk::TextBuffer::iterator& end);

	void on_obby_insert(const obby::insert_record& record);
	void on_obby_delete(const obby::delete_record& record);

	void on_insert_after(const Gtk::TextBuffer::iterator& begin,
	                     const Glib::ustring& text,
			     int bytes);
	void on_erase_after(const Gtk::TextBuffer::iterator& begin,
	                    const Gtk::TextBuffer::iterator& end);

	void on_cursor_changed(const Gtk::TextBuffer::iterator& location,
	                       const Glib::RefPtr<Gtk::TextBuffer::Mark>& mark);

	obby::document& m_doc;
	const Folder& m_folder;

#ifdef WITH_GTKSOURCEVIEW
	Gtk::SourceView m_view;
	Glib::RefPtr<Gtk::SourceLanguagesManager> m_lang_manager;
#else
	Gtk::TextView m_view;
#endif
	/** Variable to prevent event functions from endless recursion. After
	 * an obby insert or textbuffer insert has occured, this variable is
	 * set to true, after the event has been handled, to false. If an
	 * obby event arrives and we insert the newly written text into the
	 * buffer, a textbuffer-insert event would occur. But if m_editing is
	 * true, the event is ignored.
	 */
	bool m_editing;
	
	signal_update_type m_signal_update;
};

}

#endif // _GOBBY_DOCUMENT_HPP_
