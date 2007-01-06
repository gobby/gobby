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

#include "preferences.hpp"
#include "docwindow.hpp"
#include "document.hpp"
#include "buffer_def.hpp"
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

	typedef sigc::signal<void, DocWindow&>
		signal_document_add_type;
	typedef sigc::signal<void, DocWindow&>
		signal_document_remove_type;

	typedef sigc::signal<void, DocWindow&>
		signal_document_close_request_type;
	typedef sigc::signal<void, DocWindow&>
		signal_document_cursor_moved_type;
	typedef sigc::signal<void, DocWindow&>
		signal_document_content_changed_type;
	typedef sigc::signal<void, DocWindow&>
		signal_document_language_changed_type;

	typedef sigc::signal<void, DocWindow&>
		signal_tab_switched_type;

	Folder(Header& header, const Preferences& preferences);

	// Access to the mime map
	const MimeMap& get_mime_map() const;

	Glib::RefPtr<const Gtk::SourceLanguagesManager>
		get_lang_manager() const;

	// Calls from the window
	// TODO: Replace the last 5 of these functions by direct signal
	// connections to obby::local_buffer
	void obby_start(LocalBuffer& buf);
	void obby_end();
	void obby_user_join(const obby::user& user);
	void obby_user_part(const obby::user& user);
	void obby_user_colour(const obby::user& user);
	void obby_document_insert(LocalDocumentInfo& document);
	void obby_document_remove(LocalDocumentInfo& document);

	/** Signal which will be emitted if a document has been added to the
	 * folder.
	 */
	signal_document_add_type
		document_add_event() const;

	/** Signal which will be emitted if a document has been removed from
	 * the folder.
	 */
	signal_document_remove_type
		document_remove_event() const;

	/** Signal which will be emitted if the user wants to close a document
	 * (by clicking on the close button on the tab label).
	 */
	signal_document_close_request_type
		document_close_request_event() const;

	signal_document_cursor_moved_type
		document_cursor_moved_event() const;
	signal_document_content_changed_type
		document_content_changed_event() const;
	signal_document_language_changed_type
		document_language_changed_event() const;
	signal_tab_switched_type
		tab_switched_event() const;
protected:
	// Overrides
	virtual void on_switch_page(GtkNotebookPage* page, guint page_num);

	virtual bool on_key_press_event(GdkEventKey* event);

	// Internals
	void set_tab_colour(DocWindow& win, const Glib::ustring& colour);

	// Signal handlers
	void on_language_changed(const Glib::RefPtr<Gtk::SourceLanguage>& language);

	void on_document_subscribe(const obby::user& user,
	                           LocalDocumentInfo& info);
	void on_document_unsubscribe(const obby::user& user,
	                             LocalDocumentInfo& info);

	// Called by on_document_subscribe/unsubscribe if the (un)subscribing
	// user is the local one.
	void on_self_subscribe(LocalDocumentInfo& info);
	void on_self_unsubscribe(LocalDocumentInfo& info);

	void on_document_modified_changed(DocWindow& window);
	void on_document_close(DocWindow& window);

	void on_document_cursor_moved(DocWindow& window);
	void on_document_content_changed(DocWindow& window);
	void on_document_language_changed(DocWindow& window);

	signal_document_add_type
		m_signal_document_add;
	signal_document_remove_type
		m_signal_document_remove;

	signal_document_close_request_type
		m_signal_document_close_request;
	signal_document_cursor_moved_type
		m_signal_document_cursor_moved;
	signal_document_content_changed_type
		m_signal_document_content_changed;
	signal_document_language_changed_type
		m_signal_document_language_changed;

	signal_tab_switched_type
		m_signal_tab_switched;

	/** Whether to block the handling of the language changed event.
	 */
	bool m_block_language;

	/** Reference to Header.
	 */
	Header& m_header;

	/** Reference to current preferences
	 */
	const Preferences& m_preferences;

	/** Contains a pointer to the current active obby buffer.
	 */
	LocalBuffer* m_buffer;

	/** Connection to the unsubscribe signal.
	 */
	sigc::connection m_conn_unsubscribe;

	MimeMap m_mime_map;
};

}

#endif // _GOBBY_FOLDER_HPP_
