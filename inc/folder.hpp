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

#include "sourceview/sourcelanguage.hpp"
#include "sourceview/sourcelanguagesmanager.hpp"
#include "mimemap.hpp"

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

		void set_modified(bool modified = true);
		void set_label(const Glib::ustring& label);
		void set_use_markup(bool setting = true);

		close_signal_type close_event();
	protected:
		Gtk::Image m_image;
		Gtk::Label m_label;
		Gtk::Label m_modified;
		Gtk::Button m_button;
		Gtk::HBox m_box;
	};

	typedef sigc::signal<void, Document&> signal_document_close_type;
	typedef sigc::signal<void, Document&> signal_document_cursor_moved_type;
	typedef sigc::signal<void, Document&>
		signal_document_content_changed_type;
	typedef sigc::signal<void, Document&>
		signal_document_language_changed_type;
	typedef sigc::signal<void, Document&> signal_tab_switched_type;

	Folder(const Preferences& preferences);
	~Folder();

	// Access to the mime map
	const MimeMap& get_mime_map() const;

	Glib::RefPtr<Gtk::SourceLanguagesManager> get_lang_manager() const;

	// Calls from the window
	void obby_start(obby::local_buffer& buf);
	void obby_end();
	void obby_user_join(obby::user& user);
	void obby_user_part(obby::user& user);
	void obby_user_colour(obby::user& user);
	void obby_document_insert(obby::local_document_info& document);
	void obby_document_remove(obby::local_document_info& document);

	signal_document_close_type document_close_event() const;
	signal_document_cursor_moved_type document_cursor_moved_event() const;
	signal_document_content_changed_type
		document_content_changed_event() const;
	signal_document_language_changed_type
		document_language_changed_event() const;
	signal_tab_switched_type tab_switched_event() const;

protected:
	// Overrides
	virtual void on_switch_page(GtkNotebookPage* page, guint page_num);

	void set_tab_colour(DocWindow& win, const Glib::ustring& colour);

	// Signal handlers
	void on_document_modified_changed(DocWindow& window);
	void on_document_close(Document& document);

	void on_document_subscribe(const obby::user& user, DocWindow& window);
	void on_document_unsubscribe(const obby::user& user, DocWindow& window);

	void on_document_cursor_moved(Document& document);
	void on_document_content_changed(DocWindow& window);
	void on_document_language_changed(Document& document);

	signal_document_close_type m_signal_document_close;
	signal_document_cursor_moved_type m_signal_document_cursor_moved;
	signal_document_content_changed_type m_signal_document_content_changed;
	signal_document_language_changed_type
		m_signal_document_language_changed;
	signal_tab_switched_type m_signal_tab_switched;

	/** Reference to current preferences
	 */
	const Preferences& m_preferences;

	/** Signals whether the obby session is running.
	 */
	bool m_running;

	Glib::RefPtr<Gtk::SourceLanguagesManager> m_lang_manager;
	MimeMap m_mime_map;
};

}

#endif // _GOBBY_FOLDER_HPP_

