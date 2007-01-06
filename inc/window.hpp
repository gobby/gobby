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
#include <obby/local_buffer.hpp>
#include "config.hpp"
#include "header.hpp"
#include "folder.hpp"
#include "userlist.hpp"
#include "chat.hpp"
#include "statusbar.hpp"

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

	void on_about();
	void on_quit();

	void on_chat(const Glib::ustring& message);
	void on_document_update(Document& document);

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

	// Helper functions
	void display_error(const Glib::ustring& message);

	// Config
	Config m_config;

	// GUI
	Gtk::VBox m_mainbox;
	Header m_header;
	StatusBar m_statusbar;

	Gtk::VPaned m_mainpaned;
	Gtk::HPaned m_subpaned;

	Gtk::Frame m_frame_chat;
	Gtk::Frame m_frame_list;
	Gtk::Frame m_frame_text;

	Folder m_folder;
	UserList m_userlist;
	Chat m_chat;

	// obby
	obby::local_buffer* m_buffer;
	sigc::connection m_timer_conn;
	bool m_running; // m_running is set if the obby connection has been
	                // established successfully.
};

}

#endif // _GOBBY_WINDOW_HPP_
