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

#include <gtkmm/main.h>
#include <gtkmm/messagedialog.h>
#include <libobby/client_buffer.hpp>
#include <libobby/host_buffer.hpp>
#include "createdialog.hpp"
#include "joindialog.hpp"
#include "window.hpp"

Gobby::Window::Window()
 : Gtk::Window(Gtk::WINDOW_TOPLEVEL), 
   m_config(Glib::get_home_dir() + "/.gobby/config.xml"), m_buffer(NULL)
{
	m_header.session_create_event().connect(
		sigc::mem_fun(*this, &Window::on_session_create) );
	m_header.session_join_event().connect(
		sigc::mem_fun(*this, &Window::on_session_join) );
	m_header.session_quit_event().connect(
		sigc::mem_fun(*this, &Window::on_session_quit) );
	m_header.quit_event().connect(
		sigc::mem_fun(*this, &Window::on_quit) );

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
	delete m_buffer;
}

void Gobby::Window::on_session_create() try
{
	CreateDialog dlg(*this, m_config);
	if(dlg.run() == Gtk::RESPONSE_OK)
	{
		m_port = dlg.get_port();
		m_name = dlg.get_name();
		m_color = dlg.get_color();

		unsigned int red = m_color.get_red() * 255 / 65535;
		unsigned int green = m_color.get_green() * 255 / 65535;
		unsigned int blue = m_color.get_blue() * 255 / 65535;

		// Delete existing buffer, create new one
		delete m_buffer;
		m_buffer = new obby::host_buffer(m_port, m_name,
		                                 red, green, blue);
		
		m_buffer->user_join_event().connect(
			sigc::mem_fun(*this, &Window::on_obby_user_join) );
		m_buffer->user_part_event().connect(
			sigc::mem_fun(*this, &Window::on_obby_user_part) );
		m_buffer->insert_document_event().connect(
			sigc::mem_fun(*this, &Window::on_obby_document_insert));
		m_buffer->remove_document_event().connect(
			sigc::mem_fun(*this, &Window::on_obby_document_remove));
	}
	else
	{
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
		m_host = dlg.get_host();
		m_port = dlg.get_port();
		m_name = dlg.get_name();
		m_color = dlg.get_color();

		unsigned int red = m_color.get_red() * 255 / 65535;
		unsigned int green = m_color.get_green() * 255 / 65535;
		unsigned int blue = m_color.get_blue() * 255 / 65535;

		// TODO: Keep existing connection if host and port did not
		// change
		delete m_buffer;
		m_buffer = new obby::client_buffer(m_host, m_port);

		obby::client_buffer* client_buffer;
		client_buffer = static_cast<obby::client_buffer*>(m_buffer);

		client_buffer->login_failed_event().connect(
			sigc::mem_fun(*this, &Window::on_obby_login_failed) );
		client_buffer->close_event().connect(
			sigc::mem_fun(*this, &Window::on_obby_close) );
		
		m_buffer->user_join_event().connect(
			sigc::mem_fun(*this, &Window::on_obby_user_join) );
		m_buffer->user_part_event().connect(
			sigc::mem_fun(*this, &Window::on_obby_user_part) );
		m_buffer->insert_document_event().connect(
			sigc::mem_fun(*this, &Window::on_obby_document_insert));
		m_buffer->remove_document_event().connect(
			sigc::mem_fun(*this, &Window::on_obby_document_remove));

		client_buffer->login(m_name, red, green, blue);
	}
	else
	{
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
	delete m_buffer;
	m_buffer = NULL;
}

void Gobby::Window::on_quit()
{
	Gtk::Main::quit();
}

void Gobby::Window::on_obby_login_failed(const std::string& reason)
{
	display_error(reason);
	on_session_join();
}

void Gobby::Window::on_obby_close()
{
}

void Gobby::Window::on_obby_user_join(obby::user& user)
{
}

void Gobby::Window::on_obby_user_part(obby::user& user)
{
}

void Gobby::Window::on_obby_document_insert(obby::document& document)
{
}

void Gobby::Window::on_obby_document_remove(obby::document& document)
{
}

void Gobby::Window::display_error(const Glib::ustring& message)
{
	Gtk::MessageDialog dlg(*this, message, false, Gtk::MESSAGE_ERROR,
	                       Gtk::BUTTONS_OK, true);
	dlg.run();
}

