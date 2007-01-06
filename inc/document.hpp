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

#include <obby/document.hpp>

#include "features.hpp"
#ifdef WITH_GTKSOURCEVIEW
#include "sourceview/sourcelanguagesmanager.hpp"
#include "sourceview/sourceview.hpp"
#else
#include <gtkmm/textview.h>
#endif

namespace Gobby
{

class Folder;

#ifdef WITH_GTKSOURCEVIEW
class Document : public Gtk::SourceView
#else
class Document : public Gtk::TextView
#endif
{
public:
	typedef sigc::signal<void> signal_cursor_moved_type;
	typedef sigc::signal<void> signal_content_changed_type;
#ifdef WITH_GTKSOURCEVIEW
	typedef sigc::signal<void> signal_language_changed_type;
#endif

	Document(obby::document& doc, const Folder& folder);
	virtual ~Document();

	const obby::document& get_document() const;
	obby::document& get_document();

	/** Writes the current cursor position into row and col.
	 */
	void get_cursor_position(unsigned int& row, unsigned int& col);

	/** Returns the amount of unsynced operations in this document.
	 */
	unsigned int get_unsynced_changes_count() const;

	/** Returns the current document revision.
	 */
	unsigned int get_revision() const;

#ifdef WITH_GTKSOURCEVIEW
	/** Returns the currently selected Gtk::SourceLanguage.
	 */
	Glib::RefPtr<Gtk::SourceLanguage> get_language() const;

	/** Sets a new Language to use.
	 */
	void set_language(const Glib::RefPtr<Gtk::SourceLanguage>& language);
#endif

	/** Returns the document content. Equivalent to
	 * get_document().get_whole_buffer(), but it may be used even if the
	 * obby buffer does not exist anymore (in which case get_document()
	 * returns an invalid reference!)
	 */
	Glib::ustring get_content();

#ifdef WITH_GTKSOURCEVIEW
	/** Returns whether line numbers are currently shown for this document.
	 */
	bool get_show_line_numbers() const;

	/** Sets whether to show line numbers for this document.
	 */
	void set_show_line_numbers(bool show);
#endif

	/** Signal which will be emitted if the cursor's position changed.
	 */
	signal_cursor_moved_type cursor_moved_event() const;

	/** Signal which will be emitted if the document's content was changed.
	 */
	signal_content_changed_type content_changed_event() const;

#ifdef WITH_GTKSOURCEVIEW
	/** Signal which will be emitted if the document's language has changed.
	 */
	signal_language_changed_type language_changed_event() const;
#endif

	/** Calls from the folder.
	 */
	void obby_user_join(obby::user& user);
	void obby_user_part(obby::user& user);

protected:
	/** Obby signal handlers.
	 */
	void on_obby_insert(const obby::insert_record& record);
	void on_obby_delete(const obby::delete_record& record);
	void on_obby_change_before();
	void on_obby_change_after();

	/** TextBuffer signal handlers.
	 */
	void on_insert_before(const Gtk::TextBuffer::iterator& begin,
	                      const Glib::ustring& text,
		              int bytes);
	void on_erase_before(const Gtk::TextBuffer::iterator& begin,
	                     const Gtk::TextBuffer::iterator& end);

	void on_insert_after(const Gtk::TextBuffer::iterator& end,
	                     const Glib::ustring& text,
			     int bytes);
	void on_erase_after(const Gtk::TextBuffer::iterator& begin,
	                    const Gtk::TextBuffer::iterator& end);
	void on_apply_tag_after(const Glib::RefPtr<Gtk::TextTag>& tag,
	                        const Gtk::TextBuffer::iterator& begin,
	                        const Gtk::TextBuffer::iterator& end);

	/** Signal handler for the mark_set event to detect cursor movements.
	 */
	void on_mark_set(const Gtk::TextBuffer::iterator& location,
	                 const Glib::RefPtr<Gtk::TextBuffer::Mark>& mark);

	/** Marks the given part of the text as written by <em>user</em>.
	 */
	void update_user_colour(const Gtk::TextBuffer::iterator& begin,
	                        const Gtk::TextBuffer::iterator& end,
				const obby::user* user);

	obby::document& m_doc;
	const Folder& m_folder;

	/** Variable to prevent event handlers from endless recursion. After
	 * an obby insert or textbuffer insert has occured, this variable is
	 * set to true, after the event has been handled, to false. If an
	 * obby event arrives and we insert the newly written text into the
	 * buffer, a textbuffer-insert event would occur. But if m_editing is
	 * true, the event is ignored.
	 */
	bool m_editing;

	signal_cursor_moved_type m_signal_cursor_moved;
	signal_content_changed_type m_signal_content_changed;
#ifdef WITH_GTKSOURCEVIEW
	signal_language_changed_type m_signal_language_changed;
#endif
private:
	/** Handler for update_user_colour(): It removes the given tag in
	 * the given range if it is a gobby-user-tag.
	 */
	void on_remove_user_colour(Glib::RefPtr<Gtk::TextBuffer::Tag> tag,
	                           const Gtk::TextBuffer::iterator& begin,
	                           const Gtk::TextBuffer::iterator& end);

};

}

#endif // _GOBBY_DOCUMENT_HPP_
