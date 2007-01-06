/* gobby - A GTKmm driven libobby client
 * Copyright (C) 2005 0x539 dev group
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef _GOBBY_WINDOW_HPP_
#define _GOBBY_WINDOW_HPP_

#include <gtkmm/window.h>
#include <gtkmm/paned.h>
#include <gtkmm/frame.h>
#include <libobby/buffer.hpp>
#include "config.hpp"
#include "header.hpp"
#include "folder.hpp"
#include "userlist.hpp"
#include "chat.hpp"

namespace Gobby
{

class Window : public Gtk::Window
{
public:
	Window();
	~Window();

protected:
	void on_session_create();
	void on_session_join();
	void on_session_quit();
	void on_quit();

	void on_chat(const Glib::ustring& message);

	// Obby signal handlers
	void on_obby_login_failed(const std::string& reason);
	void on_obby_close();
	void on_obby_sync();

	void on_obby_user_join(obby::user& user);
	void on_obby_user_part(obby::user& user);
	void on_obby_document_insert(obby::document& document);
	void on_obby_document_remove(obby::document& document);

	void on_obby_server_chat(const Glib::ustring& message);
	void on_obby_chat(obby::user& user, const Glib::ustring& message);

	bool on_timer();

	// Helper functions
	void display_error(const Glib::ustring& message);

	// Config
	Config m_config;

	// GUI
	Gtk::VBox m_mainbox;
	Header m_header;

	Gtk::VPaned m_mainpaned;
	Gtk::HPaned m_subpaned;

	Gtk::Frame m_frame_chat;
	Gtk::Frame m_frame_list;
	Gtk::Frame m_frame_text;

	Folder m_folder;
	UserList m_userlist;
	Chat m_chat;

	// obby
	obby::buffer* m_buffer;
	sigc::connection m_timer_conn;
	bool m_running; // m_running is set if the obby connection has been
	                // established successfully.
	bool m_login_failed; // Variable used to indicate a login failed event
	                     // See corresponding comments in window.cpp
};

}

#endif // _GOBBY_WINDOW_HPP_
