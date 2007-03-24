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

#include <queue>
#include <memory>

#include <gtkmm/window.h>
#include <gtkmm/paned.h>
#include <gtkmm/frame.h>
#include <gtkmm/messagedialog.h>

#include "features.hpp"
#include "icon.hpp"
#include "config.hpp"
#include "application_state.hpp"
#include "ipc.hpp"
#include "header.hpp"
#include "docwindow.hpp"
#include "buffer_def.hpp"
#include "userlist.hpp"
#include "documentlist.hpp"
#include "hostdialog.hpp"
#include "joindialog.hpp"
#include "finddialog.hpp"
#include "gotodialog.hpp"
#include "folder.hpp"
#include "document_settings.hpp"
#include "chat.hpp"
#include "statusbar.hpp"
#include "dragdrop.hpp"
#ifdef WITH_ZEROCONF
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

	/** @brief Opens a session with the current default settings.
	 *
	 * If initial_dialog is true a dialog to turn the host parameters is
	 * opened, otherwise the default settings are taken.
	 *
	 * If the session opening failed, a dialog appears where the user
	 * might adjust settings or abort.
	 *
	 * This function must not be called when a buffer is already open.
	 */
	bool session_open(bool initial_dialog);

	/** @brief Joins a session with the current default settings.
	 *
	 * If initial_dialog is true a dialog to turn the join parameters is
	 * opened, otherwise the default settings are used.
	 *
	 * If the session join failed, a dialog appears where the user
	 * might adjust settings or abort.
	 *
	 * This function must not be called when a buffer is already open.
	 */
	bool session_join(bool initial_dialog);

	/** Opens a document containing the content of a file mounted on the
	 * local filesystem.
	 */
	void open_local_file(const Glib::ustring& file,
	                     const std::string& encoding);

	/** Saves an existing document to the given path.
	 */
	void save_local_file(DocWindow& doc,
	                     const Glib::ustring& file,
	                     const std::string& encoding);
protected:
	// Gtk::Window overrides
	virtual bool on_delete_event(GdkEventAny* event);
	virtual void on_realize();

	void on_chat_realize();

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

	void on_window_chat();

	// Folder UI handler
	void on_folder_document_add(DocWindow& window);
	void on_folder_document_remove(DocWindow& window);
	void on_folder_document_close_request(DocWindow& window);
	void on_folder_tab_switched(DocWindow& window);

	void on_settings_document_insert(const LocalDocumentInfo& info);

	void on_about();
	void on_quit();

	// Obby signal handlers
	void on_obby_close();

	void on_obby_user_join(const obby::user& user);
	void on_obby_user_part(const obby::user& user);
	void on_obby_user_colour(const obby::user& user);
	void on_obby_user_colour_failed();
	void on_obby_document_insert(DocumentInfo& document);
	void on_obby_document_remove(DocumentInfo& document);

	// IPC signal handlers
	void on_ipc_file(const std::string& file);

	// Helper functions
	void apply_preferences();
	void update_title_bar();
	void close_document(DocWindow& doc);
	void display_error(const Glib::ustring& message,
	                   const Gtk::MessageType type = Gtk::MESSAGE_ERROR);

	bool session_join_impl(const Glib::ustring& host,
	                       unsigned int port,
	                       const Glib::ustring& name,
	                       const Gdk::Color& color);

	bool session_open_impl(unsigned int port,
	                       const Glib::ustring& name,
	                       const Gdk::Color& color,
	                       const Glib::ustring& password,
	                       const Glib::ustring& session);

	// Config
	Config& m_config;
	Glib::RefPtr<Gtk::SourceLanguagesManager> m_lang_manager;
	Preferences m_preferences;
	const IconManager& m_icon_mgr;

	// Paths
	std::string m_last_path;
	std::string m_prev_session;

	std::string m_local_file_path;
	std::string m_local_encoding;

	// GUI
	Gtk::VBox m_mainbox;
	Gtk::VPaned m_mainpaned;

	Gtk::Frame m_frame_chat;
	Gtk::Frame m_frame_text;

	ApplicationState m_application_state;
	DocumentSettings m_document_settings;

	Header m_header;
	UserList m_userlist;
	DocumentList m_documentlist;

	std::auto_ptr<FindDialog> m_finddialog;
	std::auto_ptr<GotoDialog> m_gotodialog;

	Folder m_folder;
	Chat m_chat;
	StatusBar m_statusbar;

	sigc::connection m_conn_chat_realize;

	/** Drag+Drop handler
	 */
	std::auto_ptr<DragDrop> m_dnd;

	/** Local IPC handler
	 */
	std::auto_ptr<Ipc::LocalInstance> m_ipc;
	std::queue<std::string> m_file_queue;

	// Dialogs
	std::auto_ptr<HostDialog> m_host_dlg;
	std::auto_ptr<JoinDialog> m_join_dlg;

	// obby
	std::auto_ptr<LocalBuffer> m_buffer;
#ifdef WITH_ZEROCONF
	std::auto_ptr<obby::zeroconf> m_zeroconf;
#endif
	sigc::connection m_timer_conn;
};

}

#endif // _GOBBY_WINDOW_HPP_
