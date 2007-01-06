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

#include <stdexcept>
#include <gtkmm/main.h>
#include <gtkmm/aboutdialog.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/filechooserdialog.h>
#include <libobby/client_buffer.hpp>
#include <libobby/host_buffer.hpp>
#include "createdialog.hpp"
#include "joindialog.hpp"
#include "entrydialog.hpp"
#include "window.hpp"
#include "features.hpp"

Gobby::Window::Window()
 : Gtk::Window(Gtk::WINDOW_TOPLEVEL), 
   m_config(Glib::get_home_dir() + "/.gobby/config.xml"), m_buffer(NULL),
   m_running(false), m_login_failed(false)
{
	m_header.session_create_event().connect(
		sigc::mem_fun(*this, &Window::on_session_create) );
	m_header.session_join_event().connect(
		sigc::mem_fun(*this, &Window::on_session_join) );
	m_header.session_quit_event().connect(
		sigc::mem_fun(*this, &Window::on_session_quit) );

	m_header.document_create_event().connect(
		sigc::mem_fun(*this, &Window::on_document_create) );
	m_header.document_open_event().connect(
		sigc::mem_fun(*this, &Window::on_document_open) );
	m_header.document_close_event().connect(
		sigc::mem_fun(*this, &Window::on_document_close) );

	m_header.about_event().connect(
		sigc::mem_fun(*this, &Window::on_about) );
	m_header.quit_event().connect(
		sigc::mem_fun(*this, &Window::on_quit) );

	m_chat.chat_event().connect(
		sigc::mem_fun(*this, &Window::on_chat) );

	m_frame_chat.set_shadow_type(Gtk::SHADOW_IN);
	m_frame_list.set_shadow_type(Gtk::SHADOW_IN);
	m_frame_text.set_shadow_type(Gtk::SHADOW_IN);

	m_frame_chat.add(m_chat);
	m_frame_list.add(m_userlist);
	m_frame_text.add(m_folder);

	m_subpaned.pack1(m_frame_text, true, false);
	m_subpaned.pack2(m_frame_list, true, false);

	m_mainpaned.set_border_width(10);
	m_mainpaned.pack1(m_subpaned, true, false);
	m_mainpaned.pack2(m_frame_chat, true, false);

	m_mainbox.pack_start(m_header, Gtk::PACK_SHRINK);
	m_mainbox.pack_start(m_mainpaned, Gtk::PACK_EXPAND_WIDGET);

	add(m_mainbox);

	set_title("Gobby");
	set_default_size(640, 480);
}

Gobby::Window::~Window()
{
	on_session_quit();
}

void Gobby::Window::on_session_create() try
{
	CreateDialog dlg(*this, m_config);
	if(dlg.run() == Gtk::RESPONSE_OK)
	{
		unsigned int port = dlg.get_port();
		Glib::ustring name = dlg.get_name();
		Gdk::Color color = dlg.get_color();	

		unsigned int red = color.get_red() * 255 / 65535;
		unsigned int green = color.get_green() * 255 / 65535;
		unsigned int blue = color.get_blue() * 255 / 65535;

		// Create new buffer
		obby::host_buffer* buffer = new obby::host_buffer(
			port, name, red, green, blue);

		// Delete existing buffer, take new one
		delete m_buffer;
		m_buffer = buffer;
		
		m_buffer->user_join_event().connect(
			sigc::mem_fun(*this, &Window::on_obby_user_join) );
		m_buffer->user_part_event().connect(
			sigc::mem_fun(*this, &Window::on_obby_user_part) );
		m_buffer->insert_document_event().connect(
			sigc::mem_fun(*this, &Window::on_obby_document_insert));
		m_buffer->remove_document_event().connect(
			sigc::mem_fun(*this, &Window::on_obby_document_remove));

		m_buffer->message_event().connect(
			sigc::mem_fun(*this, &Window::on_obby_chat) );
		m_buffer->server_message_event().connect(
			sigc::mem_fun(*this, &Window::on_obby_server_chat) );


		if(!m_timer_conn.connected() )
			m_timer_conn = Glib::signal_timeout().connect(
				sigc::mem_fun(*this, &Window::on_timer), 400);

		// Running
		m_running = true;
		
		// Delegate start of obby session
		m_header.obby_start();
		m_folder.obby_start();
		m_userlist.obby_start();
		m_chat.obby_start();

		// Let the local user join
		on_obby_user_join(buffer->get_self() );
	}
	else
	{
		if(m_timer_conn.connected() )
			m_timer_conn.disconnect();

		// Delete existing buffer, if any
		delete m_buffer;
		m_buffer = NULL;
	}
}
catch(Glib::Exception& e)
{
	display_error(e.what() );
	on_session_create();
}
catch(std::exception& e)
{
	display_error(e.what() );
	on_session_create();
}

void Gobby::Window::on_session_join() try
{
	JoinDialog dlg(*this, m_config);
	if(dlg.run() == Gtk::RESPONSE_OK)
	{
		Glib::ustring host = dlg.get_host();
		unsigned int port = dlg.get_port();
		Glib::ustring name = dlg.get_name();
		Gdk::Color color = dlg.get_color();

		unsigned int red = color.get_red() * 255 / 65535;
		unsigned int green = color.get_green() * 255 / 65535;
		unsigned int blue = color.get_blue() * 255 / 65535;

		// TODO: Keep existing connection if host and port did not
		// change
		obby::client_buffer* buffer = new obby::client_buffer(
			host, port);
		delete m_buffer;
		m_buffer = buffer;

		buffer->login_failed_event().connect(
			sigc::mem_fun(*this, &Window::on_obby_login_failed) );
		buffer->close_event().connect(
			sigc::mem_fun(*this, &Window::on_obby_close) );
		buffer->sync_event().connect(
			sigc::mem_fun(*this, &Window::on_obby_sync) );
		
		m_buffer->user_join_event().connect(
			sigc::mem_fun(*this, &Window::on_obby_user_join) );
		m_buffer->user_part_event().connect(
			sigc::mem_fun(*this, &Window::on_obby_user_part) );
		m_buffer->insert_document_event().connect(
			sigc::mem_fun(*this, &Window::on_obby_document_insert));
		m_buffer->remove_document_event().connect(
			sigc::mem_fun(*this, &Window::on_obby_document_remove));

		m_buffer->message_event().connect(
			sigc::mem_fun(*this, &Window::on_obby_chat) );
		m_buffer->server_message_event().connect(
			sigc::mem_fun(*this, &Window::on_obby_server_chat) );

		if(!m_timer_conn.connected() )
			m_timer_conn = Glib::signal_timeout().connect(
				sigc::mem_fun(*this, &Window::on_timer), 400);

		buffer->login(name, red, green, blue);
	}
	else
	{
		if(m_timer_conn.connected() )
			m_timer_conn.disconnect();

		delete m_buffer;
		m_buffer = NULL;
	}
}
catch(Glib::Exception& e)
{
	display_error(e.what());
	on_session_join();
}
catch(std::exception& e)
{
	display_error(e.what());
	on_session_join();
}

void Gobby::Window::on_session_quit()
{
	if(m_buffer)
	{
		if(m_running)
		{
			m_header.obby_end();
			m_folder.obby_end();
			m_userlist.obby_end();
			m_chat.obby_end();

			m_running = false;
		}

		if(m_timer_conn.connected() )
			m_timer_conn.disconnect();

		delete m_buffer;
		m_buffer = NULL;
	}
}

void Gobby::Window::on_about()
{
	Gtk::AboutDialog dlg;
	dlg.set_name("Gobby");
	dlg.set_version(PACKAGE_VERSION);
	dlg.set_comments("A collaborative text editor");
	dlg.set_copyright("Copyright (C) 2005 0x539 dev group <crew@0x539.de>");

	std::deque<Glib::ustring> authors;
	authors.push_back("Armin Burgmeier <armin@0x539.de>");
	authors.push_back("Benjamin Herr <ben@0x539.de>");
	authors.push_back("Philipp Kern <phil@0x539.de>");
	dlg.set_authors(authors);

	dlg.set_license(
		"This program is free software; you can redistribute it\n"
		"and/or modify it under the terms of the GNU General Public\n"
		"License as published by the Free Software Foundation; either\n"
		"version 2 of the License, or (at your option) any later\n"
		"version.\n"
		"\n"
		"This program is distributed in the hope that it will be\n"
		"useful, but WITHOUT ANY WARRANTY; without even the implied\n"
		"warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR\n"
		"PURPOSE.  See the GNU General Public License for more details."
	);
	dlg.run();
}

void Gobby::Window::on_document_create()
{
	EntryDialog dlg(*this, "Create document", "Enter document name");
	if(dlg.run() == Gtk::RESPONSE_OK)
	{
		m_buffer->create_document(dlg.get_text() );
	}
}

void Gobby::Window::on_document_open()
{
	Gtk::FileChooserDialog dlg(*this, "Open new document");
	if(dlg.run() == Gtk::RESPONSE_OK)
	{
		m_buffer->create_document(dlg.get_filename() );
	}
}

void Gobby::Window::on_document_close()
{
}

void Gobby::Window::on_quit()
{
	on_session_quit();
	Gtk::Main::quit();
}

void Gobby::Window::on_chat(const Glib::ustring& message) {
	if (m_running)
		m_buffer->send_message(message);
	else
		throw std::runtime_error("tried to send chat message while not connected");
}

void Gobby::Window::on_obby_login_failed(const std::string& reason)
{
	// Remove timer connection, we do not need any timer calls while
	// displaying dialogs
	m_timer_conn.disconnect();
	display_error(reason);

	// We can not call on_session_join right here. In the callstack, there
	// is a call to the timer (which emitted the login_failed signal through
	// obby::buffer::select). on_session_join destroys this buffer, but it
	// still remains in the callstack. The program will crash if it regains
	// control.
	// TODO: A good solution would be to disable the name and port field
	// in the dialog now, because we are already connected to a server. Only
	// the login fields (name & color) will be editable. That ensures that
	// we need not to create a new buffer but just resend a login request.
	m_login_failed = true;
}

void Gobby::Window::on_obby_close()
{
	display_error("Connection lost");
	on_session_quit();
}

void Gobby::Window::on_obby_sync()
{
	// Send documents to components
	obby::buffer::document_iterator iter = m_buffer->document_begin();
	for(iter; iter != m_buffer->document_end(); ++ iter)
		on_obby_document_insert(*iter);
}

void Gobby::Window::on_obby_chat(obby::user& user, const Glib::ustring& message)
{
	m_chat.obby_message(user, message);
}

void Gobby::Window::on_obby_server_chat(const Glib::ustring& message)
{
	m_chat.obby_server_message(message);
}

bool Gobby::Window::on_timer()
{
	// TODO: Connection lost segfaultet hier..?
//	if(!m_buffer) return true;

	for(int i = 0; i < 15; ++ i)
	{
		m_buffer->select(0);

		// See comment in Window::on_obby_login_failed
		if(m_login_failed)
		{
			on_session_join();
			m_login_failed = false;
			return true;
		}
	}

	return true;
}

void Gobby::Window::on_obby_user_join(obby::user& user)
{
	// TODO: Something like is_client to prevent dynamic_cast?
	obby::client_buffer* buf = dynamic_cast<obby::client_buffer*>(m_buffer);
	if(buf)
	{
		if(&buf->get_self() == &user)
		{
			// Login was sucessful, let the fun begin		
			m_header.obby_start();
			m_folder.obby_start();
			m_userlist.obby_start();
			m_chat.obby_start();

			m_running = true;
		}
	}

	// Tell user join to components
	m_header.obby_user_join(user);
	m_folder.obby_user_join(user);
	m_userlist.obby_user_join(user);
	m_chat.obby_user_join(user);
}

void Gobby::Window::on_obby_user_part(obby::user& user)
{
	// Tell user part to components
	m_header.obby_user_part(user);
	m_folder.obby_user_part(user);
	m_userlist.obby_user_part(user);
	m_chat.obby_user_part(user);
}

void Gobby::Window::on_obby_document_insert(obby::document& document)
{
	m_header.obby_document_insert(document);
	m_folder.obby_document_insert(document);
	m_userlist.obby_document_insert(document);
	m_chat.obby_document_insert(document);
}

void Gobby::Window::on_obby_document_remove(obby::document& document)
{
	m_header.obby_document_remove(document);
	m_folder.obby_document_remove(document);
	m_userlist.obby_document_remove(document);
	m_chat.obby_document_remove(document);
}

void Gobby::Window::display_error(const Glib::ustring& message)
{
	Gtk::MessageDialog dlg(*this, message, false, Gtk::MESSAGE_ERROR,
	                       Gtk::BUTTONS_OK, true);
	dlg.run();
}

