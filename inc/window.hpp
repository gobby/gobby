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
#include <obby/error.hpp>
#include <obby/local_buffer.hpp>
#include "config.hpp"
#include "header.hpp"
#include "folder.hpp"
#include "userlist.hpp"
#include "chat.hpp"
#include "statusbar.hpp"
#include "features.hpp"
#ifdef WITH_HOWL
#include <obby/zeroconf.hpp>
#endif

namespace Gobby
{

class Window : public Gtk::Window
{
public:
	Window();
	~Window();

protected:
	// Gtk::Window overrides
	virtual void on_realize();

	// UI handler
	void on_session_create();
	void on_session_join();
	void on_session_quit();

	void on_document_create();
	void on_document_open();
	void on_document_save();
	void on_document_close();

	void on_user_set_password();

	void on_document_word_wrap();
#ifdef WITH_GTKSOURCEVIEW
	void on_document_line_numbers();
	void on_document_language(
		const Glib::RefPtr<Gtk::SourceLanguage>& lang
	);
#endif

	void on_about();
	void on_quit();

	void on_chat(const Glib::ustring& message);

	// Obby signal handlers
	void on_obby_login_failed(obby::login::error error);
	bool on_obby_global_password(std::string& password);
	bool on_obby_user_password(std::string& password);
	void on_obby_close();
	void on_obby_sync();

	void on_obby_user_join(obby::user& user);
	void on_obby_user_part(obby::user& user);
	void on_obby_document_insert(obby::document_info& document);
	void on_obby_document_remove(obby::document_info& document);

	void on_obby_server_chat(const Glib::ustring& message);
	void on_obby_chat(obby::user& user, const Glib::ustring& message);

	// Helper functions
	void display_error(const Glib::ustring& message);

	// Config
	Config m_config;

	// GUI
	Gtk::VBox m_mainbox;

	Gtk::VPaned m_mainpaned;
	Gtk::HPaned m_subpaned;

	Gtk::Frame m_frame_chat;
	Gtk::Frame m_frame_list;
	Gtk::Frame m_frame_text;

	Folder m_folder;
	UserList m_userlist;
	Chat m_chat;

	Header m_header;
	StatusBar m_statusbar;

	// obby
	obby::local_buffer* m_buffer;
#ifdef WITH_HOWL
	obby::zeroconf* m_zeroconf;
#endif
	sigc::connection m_timer_conn;
	bool m_running; // m_running is set if the obby connection has been
	                // established successfully.
};

}

#endif // _GOBBY_WINDOW_HPP_
