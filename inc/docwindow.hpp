/* gobby - A GTKmm driven libobby client
 * Copyright (C) 2005, 2006 0x539 dev group
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
#include <gtksourceview/gtksourceview.h>

#include "features.hpp"
#include "preferences.hpp"
#include "document.hpp"
#include "buffer_def.hpp"

namespace Gobby
{

/** @brief A DocWindow displays a Document.
 */
class DocWindow: public Gtk::ScrolledWindow
{
public:
	typedef sigc::signal<void> signal_cursor_moved_type;
	typedef sigc::signal<void> signal_content_changed_type;
	typedef sigc::signal<void> signal_language_changed_type;

	/** @brief Creates a new DocWindow displaying the given document.
	 *
	 * The preferences are initially applied to the DocWindow.
	 */
	DocWindow(LocalDocumentInfo& info, const Preferences& preferences);

	/** @brief Returns the current cursor position in <em>row</em>
	 * and <em>col</em>.
	 */
	void get_cursor_position(unsigned int& row, unsigned int& col);

	/** @brief Selects the given range of text and scrolls to it to
	 * be visible.
	 */
	void set_selection(const Gtk::TextIter& begin,
	                   const Gtk::TextIter& end);

	/** @brief Makes the source view insensitive, but lets the view
	 * be scrollable.
	 */
	void disable();

	/** @brief Returns the currently selected text.
	 */
	Glib::ustring get_selected_text() const;

	/** @brief Returns the title of the document. Equivalent to
	 * get_document().get_title(). DEPRECATED.
	 */
	const Glib::ustring& get_title() const; // TODO: Remove this as soon as the obby buffers stay available after session has been closed

	/** @brief Returns whether the document has been modified since it
	 * has been saved to disk.
	 *
	 * Equivalent to get_document().get_buffer()->get_modified().
	 * DEPRECATED.
	 */
	bool get_modified() const; // TODO: Remove this in favor of get_document().get_buffer()->get_modified()

	/** @brief Gives the focus to the underlaying sourceview instead of
	 * the scrolled window containing it.
	 */
	void grab_focus();

	/** @brief Returns the current GtkSourceLanguage the document is
	 * highlighted with.
	 */
	GtkSourceLanguage* get_language() const;

	/** @brief Changes the language of the document.
	 */
	void set_language(GtkSourceLanguage* language);

	/** @brief Returns the preferences set for this document.
	 */
	const Preferences& get_preferences() const;

	/** @brief Changes the preferences for this document.
	 */
	void set_preferences(const Preferences& preferences);

	/** @brief Returns the whole document content.
	 *
	 * Equivalent to get_document().get_buffer()->get_text(). DEPRECATED
	 */
	Glib::ustring get_content() const; // // TODO: Remove this as soon as the obby buffers stay available after session has been close

	/** @brief Signal that is emitted when the cursor has been moved.
	 */
	signal_cursor_moved_type cursor_moved_event() const;

	/** @brief Signal that is emitted when the document's content has
	 * changed.
	 *
	 * TODO: Move this signal to Gobby::Document.
	 */
	signal_content_changed_type content_changed_event() const;

	/** @brief Signal that is emitted when the language of the document
	 * has changed.
	 */
	signal_language_changed_type language_changed_event() const;

	/** @brief Provides access to the underlaying document info.
	 */
	const LocalDocumentInfo& get_info() const;

	/** @brief Provides access to the underlaying document info.
	 */
	LocalDocumentInfo& get_info();

	/** @brief Provides access to the underlaying document. Equivalent
	 * to get_info().get_content().
	 */
	const Document& get_document() const;

protected:
	/** @brief Callback to watch cursor movement.
	 */
	void on_mark_set(const Gtk::TextIter& location,
	                 const Glib::RefPtr<Gtk::TextMark>& mark);

	/** @brief Callback when the buffer content changed.
	 */
	void on_changed();

	/** @brief Callback when text has to be inserted.
	 */
	void on_local_insert(obby::position pos,
	                     const std::string& text);

	/** @brief Callback when text has to be erased.
	 */
	void on_local_erase(obby::position pos,
	                    obby::position len);

	void on_remote_insert_before(obby::position pos,
	                             const std::string& text);

	void on_remote_erase_before(obby::position pos,
	                            obby::position len);

	void on_remote_insert_after(obby::position pos,
	                            const std::string& text);

	void on_remote_erase_after(obby::position pos,
	                           obby::position len);

	/** @brief Helper function that applies the preferences to the buffer.
	 */
	void apply_preferences();

	void store_scroll();
	void restore_scroll();

	GtkSourceView* m_view;
	LocalDocumentInfo& m_info;
	const Document& m_doc;

	Preferences m_preferences;
	//bool m_editing;
	Glib::ustring m_title; // TODO: Remove this as soon as the obby buffers stay available after session has been closed

	signal_cursor_moved_type m_signal_cursor_moved;
	signal_content_changed_type m_signal_content_changed;
	signal_language_changed_type m_signal_language_changed;

	double m_scrolly;
	bool m_scroll_restore;
};

}

#endif // _GOBBY_DOCWINDOW_HPP_
