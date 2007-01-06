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

#include <obby/local_document_info.hpp>

#include "preferences.hpp"
#include "features.hpp"
#include "sourceview/sourcelanguagesmanager.hpp"
#include "sourceview/sourceview.hpp"

namespace Gobby
{

class Folder;

class Document : public Gtk::SourceView
{
public:
	typedef sigc::signal<void> signal_cursor_moved_type;
	typedef sigc::signal<void> signal_content_changed_type;
	typedef sigc::signal<void> signal_language_changed_type;

	Document(obby::local_document_info& doc, const Folder& folder,
	         const Preferences& preferences);
	virtual ~Document();

	const obby::local_document_info& get_document() const;
	obby::local_document_info& get_document();

	/** Writes the current cursor position into row and col.
	 */
	void get_cursor_position(unsigned int& row, unsigned int& col);

	/** Returns the amount of unsynced operations in this document.
	 */
	unsigned int get_unsynced_changes_count() const;

	/** Returns the current document revision.
	 */
	unsigned int get_revision() const;

	/** Returns the title of the file.
	 */
	const Glib::ustring& get_title() const;

	/** Returns the path to the file.
	 */
	const Glib::ustring& get_path() const;

	/** Changes the path to the file.
	 */
	void set_path(const Glib::ustring& new_path);

	/** Returns whether the local user is subscribed to this document.
	 */
	bool is_subscribed() const;

	/** Returns the currently selected Gtk::SourceLanguage.
	 */
	Glib::RefPtr<Gtk::SourceLanguage> get_language() const;

	/** Sets a new Language to use.
	 */
	void set_language(const Glib::RefPtr<Gtk::SourceLanguage>& language);

	/** Returns the currently preferences.
	 */
	const Preferences& get_preferences() const;

	/** Sets new preferences to use.
	 */
	void set_preferences(const Preferences& preferences);

	/** Returns the document content. Equivalent to
	 * get_document().get_whole_buffer(), but it may be used even if the
	 * obby buffer does not exist anymore (in which case get_document()
	 * returns an invalid reference!)
	 */
	Glib::ustring get_content();

v v v v v v v
*************
v v v v v v v
*************
	/** Returns whether the document is displayed with the words wrapped
	 * to the window's width.
	 */
	bool get_word_wrapping() const;

	/** Sets whether the words should be wrapped to the window's width.
	 */
	void set_word_wrapping(bool wrap);

	/** Returns whether line numbers are currently shown for this document.
	 */
	bool get_show_line_numbers() const;

	/** Sets whether to show line numbers for this document.
	 */
	void set_show_line_numbers(bool show);

^ ^ ^ ^ ^ ^ ^
^ ^ ^ ^ ^ ^ ^
	/** Signal which will be emitted if the cursor's position changed.
	 */
	signal_cursor_moved_type cursor_moved_event() const;

	/** Signal which will be emitted if the document's content was changed.
	 */
	signal_content_changed_type content_changed_event() const;

	/** Signal which will be emitted if the document's language has changed.
	 */
	signal_language_changed_type language_changed_event() const;

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

	void on_obby_user_subscribe(const obby::user& user);
	void on_obby_user_unsubscribe(const obby::user& user);

	/** These are called by the on_obby_user_subscribe and
	 * on_obby_user_unsubscribe if the user who (un)subscribed is
	 * the local one.
	 */
	void on_obby_self_subscribe();
	void on_obby_self_unsubscribe();

	/** GUI callbacks.
	 */
	void on_gui_subscribe();

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

	/** Sets intro text for the document, if the user is not subscribed.
	 */
	void set_intro_text();

	/** Applies the currently used preferences to the view.
	 */
	void apply_preferences();

	obby::local_document_info& m_doc;
	const Folder& m_folder;

	/** Whether we are subscribed to this document.
	 */
	bool m_subscribed;
	
	/** Preferences for this document.
	 */
	Preferences m_preferences;

	/** Variable to prevent event handlers from endless recursion. After
	 * an obby insert or textbuffer insert has occured, this variable is
	 * set to true, after the event has been handled, to false. If an
	 * obby event arrives and we insert the newly written text into the
	 * buffer, a textbuffer-insert event would occur. But if m_editing is
	 * true, the event is ignored.
	 */
	bool m_editing;

	/** Button to subscribe to the document.
	 */
	Gtk::Button m_btn_subscribe;

	/** Document title (even available when connection has been lost).
	 */
	Glib::ustring m_title;

	/** Path to the file on the local disc.
	 */
	Glib::ustring m_path;

	signal_cursor_moved_type m_signal_cursor_moved;
	signal_content_changed_type m_signal_content_changed;
	signal_language_changed_type m_signal_language_changed;
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
