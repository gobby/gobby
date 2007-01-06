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

#ifndef _GOBBY_WINDOW_HPP_
#define _GOBBY_WINDOW_HPP_

#include <gtkmm/window.h>
#include <gtkmm/paned.h>
#include <gtkmm/frame.h>
#include <gtkmm/messagedialog.h>
#include "features.hpp"
#include "icon.hpp"
#include "config.hpp"
#include "header.hpp"
#include "docwindow.hpp"
#include "buffer_def.hpp"
#include "userlist.hpp"
#include "documentlist.hpp"
#include "finddialog.hpp"
#include "gotodialog.hpp"
#include "folder.hpp"
#include "chat.hpp"
#include "statusbar.hpp"
#include "dragdrop.hpp"
#ifdef WITH_HOWL
#include <obby/zeroconf.hpp>
#endif

namespace Gobby
{

class Window : public Gtk::Window
{
public:
	Window(const IconManager& icon_mgr, Config& config);
	~Window();

	/** Offers a pointer to the currently visible document. If the user
	 * is not subscribed to a document, NULL is returned.
	 */
	DocWindow* get_current_document();

	/** Opens a document containing the content of a file mounted on the
	 * local filesystem.
	 */
	void open_local_file(const Glib::ustring& file);

	/** Saves an existing document to the given path.
	 */
	void save_local_file(DocWindow& doc, const Glib::ustring& file);
protected:
	// Gtk::Window overrides
	virtual void on_realize();
	virtual bool on_delete_event(GdkEventAny* event);

	// Start/End obby session
	void obby_start();
	void obby_end();

	// Header UI handler
	void on_session_create();
	void on_session_join();
	void on_session_save();
	void on_session_quit();

	void on_document_create();
	void on_document_open();
	void on_document_save();
	void on_document_save_as();
	void on_document_close();

	void on_edit_search();
	void on_edit_search_replace();
	void on_edit_goto_line();
	void on_edit_preferences();

	void on_user_set_password();
	void on_user_set_colour();

	void on_view_preferences();
	void on_view_language(const Glib::RefPtr<Gtk::SourceLanguage>& lang);

	// Folder UI handler
	void on_folder_document_add(DocWindow& window);
	void on_folder_document_remove(DocWindow& window);
	void on_folder_document_close_request(DocWindow& window);
	void on_folder_tab_switched(DocWindow& window);

	void on_about();
	void on_quit();

	// Obby signal handlers
	void on_obby_close();
	void on_obby_encrypted();

	void on_obby_user_join(const obby::user& user);
	void on_obby_user_part(const obby::user& user);
	void on_obby_user_colour(const obby::user& user);
	void on_obby_user_colour_failed();
	void on_obby_document_insert(DocumentInfo& document);
	void on_obby_document_remove(DocumentInfo& document);

	// Helper functions
	void apply_preferences();
	void update_title_bar();
	void close_document(DocWindow& doc);
	void display_error(const Glib::ustring& message,
	                   const Gtk::MessageType type = Gtk::MESSAGE_ERROR);

	// Config
	Config& m_config;
	Preferences m_preferences;
	const IconManager& m_icon_mgr;

	// Paths
	std::string m_last_path;
	std::string m_local_file_path;
	std::string m_prev_session;

	// GUI
	Gtk::VBox m_mainbox;
	Gtk::VPaned m_mainpaned;

	Gtk::Frame m_frame_chat;
	Gtk::Frame m_frame_text;

	Header m_header;
	UserList m_userlist;
	DocumentList m_documentlist;

	FindDialog m_finddialog;
	GotoDialog m_gotodialog;

	Folder m_folder;
	Chat m_chat;
	StatusBar m_statusbar;

	/** Drag+Drop handler
	 */
	std::auto_ptr<DragDrop> m_dnd;

	// obby
	std::auto_ptr<LocalBuffer> m_buffer;
#ifdef WITH_HOWL
	std::auto_ptr<obby::zeroconf> m_zeroconf;
#endif
	sigc::connection m_timer_conn;
};

}

#endif // _GOBBY_WINDOW_HPP_
