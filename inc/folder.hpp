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
#include <gtkmm/box.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>
#include <gtkmm/button.h>
#include <gtkmm/notebook.h>
#include <obby/user.hpp>
#include <obby/document.hpp>
#include <obby/local_buffer.hpp>

#include "preferences.hpp"
#include "docwindow.hpp"
#include "document.hpp"
#include "mimemap.hpp"
#include "header.hpp"

namespace Gobby
{

/** Thing containing multiple documents.
 */
class Folder : public Gtk::Notebook
{
public:
	class TabLabel : public Gtk::HBox
	{
	public:
		typedef Glib::SignalProxy0<void> close_signal_type;

		TabLabel(const Glib::ustring& label);
		~TabLabel();

		Glib::ustring get_label() const;

		void set_close_sensitive(bool sensitive);

		void set_modified(bool modified);
		void set_label(const Glib::ustring& label);
		void set_use_markup(bool setting);

		close_signal_type close_event();
	protected:
		Gtk::Image m_image;
		Gtk::Label m_label;
		Gtk::Label m_modified;
		Gtk::Button m_button;
		Gtk::HBox m_box;
	};

	typedef sigc::signal<void, Document&>
		signal_document_close_type;
	typedef sigc::signal<void, Document&>
		signal_document_cursor_moved_type;
	typedef sigc::signal<void, Document&>
		signal_document_content_changed_type;
	typedef sigc::signal<void, Document&>
		signal_document_language_changed_type;
	typedef sigc::signal<void, Document&>
		signal_tab_switched_type;

	Folder(Header& header, const Preferences& preferences);

	// Access to the mime map
	const MimeMap& get_mime_map() const;

	Glib::RefPtr<const Gtk::SourceLanguagesManager>
		get_lang_manager() const;

	// Calls from the window
	// TODO: Replace the last 5 of these functions by direct signal
	// connections to obby::local_buffer and emit two own signals
	// local_document_insert or something with the Gobby::Document that
	// has been inserted.
	void obby_start(obby::local_buffer& buf);
	void obby_end();
	void obby_user_join(const obby::user& user);
	void obby_user_part(const obby::user& user);
	void obby_user_colour(const obby::user& user);
	void obby_document_insert(obby::local_document_info& document);
	void obby_document_remove(obby::local_document_info& document);

	signal_document_close_type
		document_close_event() const;
	signal_document_cursor_moved_type
		document_cursor_moved_event() const;
	signal_document_content_changed_type
		document_content_changed_event() const;
	signal_document_language_changed_type
		document_language_changed_event() const;
	signal_tab_switched_type
		tab_switched_event() const;
protected:
	/** Parameter type for the internal enable_document_items function.
	 */
	enum DocumentItems {
		/** Disable all document items.
		 */
		DOCUMENT_ITEMS_DISABLE_ALL = 0,

		/** Enable only document items which are useful even if the
		 * user is not subscribed to the document, disable others.
		 */
		DOCUMENT_ITEMS_ENABLE_NOSUBSCRIBE = 1,

		/** Enable all document items.
		 */
		DOCUMENT_ITEMS_ENABLE_ALL = 2
	};

	// Overrides
	virtual void on_switch_page(GtkNotebookPage* page, guint page_num);

	// Internals
	void set_tab_colour(DocWindow& win, const Glib::ustring& colour);
	void enable_document_items(DocumentItems which);

	// Signal handlers
	void on_language_changed(const Glib::RefPtr<Gtk::SourceLanguage>& language);

	void on_document_subscribe(const obby::user& user, DocWindow& window);
	void on_document_unsubscribe(const obby::user& user, DocWindow& window);

	void on_document_modified_changed(DocWindow& window);
	void on_document_close(Document& document);

	void on_document_cursor_moved(Document& document);
	void on_document_content_changed(DocWindow& window);
	void on_document_language_changed(Document& document);

	signal_document_close_type
		m_signal_document_close;
	signal_document_cursor_moved_type
		m_signal_document_cursor_moved;
	signal_document_content_changed_type
		m_signal_document_content_changed;
	signal_document_language_changed_type
		m_signal_document_language_changed;
	signal_tab_switched_type
		m_signal_tab_switched;

	bool m_block_language;

	/** Reference to Header.
	 */
	Header& m_header;

	/** Reference to current preferences
	 */
	const Preferences& m_preferences;

	/** Contains a pointer to the current active obby buffer.
	 */
	obby::local_buffer* m_buffer;

	MimeMap m_mime_map;
};

}

#endif // _GOBBY_FOLDER_HPP_
