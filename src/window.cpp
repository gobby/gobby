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

#include <stdexcept>
#include <fstream>
#include <ostream>

#include <gtkmm/main.h>
#include <gtkmm/aboutdialog.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/stock.h>

#include <obby/client_buffer.hpp>
#include <obby/host_buffer.hpp>

#include "common.hpp"
#include "buffer_wrapper.hpp"
#include "document.hpp"
#include "hostdialog.hpp"
#include "joindialog.hpp"
#include "entrydialog.hpp"
#include "window.hpp"
#include "features.hpp"
#include "icon.hpp"

Gobby::Window::Window()
 : Gtk::Window(Gtk::WINDOW_TOPLEVEL), 
   m_config(Glib::get_home_dir() + "/.gobby/config.xml"), m_buffer(NULL),
   m_running(false), m_header(m_folder), m_statusbar(m_folder)
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
	m_header.document_save_event().connect(
		sigc::mem_fun(*this, &Window::on_document_save) );
	m_header.document_close_event().connect(
		sigc::mem_fun(*this, &Window::on_document_close) );

#ifdef WITH_GTKSOURCEVIEW
	m_header.document_line_numbers_event().connect(
		sigc::mem_fun(*this, &Window::on_document_line_numbers) );
#endif

	m_header.about_event().connect(
		sigc::mem_fun(*this, &Window::on_about) );
	m_header.quit_event().connect(
		sigc::mem_fun(*this, &Window::on_quit) );

	add_accel_group(m_header.get_accel_group() );

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
	m_mainbox.pack_start(m_statusbar, Gtk::PACK_SHRINK);

	add(m_mainbox);

	set_title("Gobby");
	set_default_size(640, 480);
}

Gobby::Window::~Window()
{
	on_session_quit();
}

void Gobby::Window::on_realize()
{
	Gtk::Window::on_realize();

	// Initialize paned sizes. This cannot be done in the constructor
	// because the widget's sizes are not known.
	int submin = m_subpaned.property_min_position();
	int submax = m_subpaned.property_max_position();
	int mainmin = m_mainpaned.property_min_position();
	int mainmax = m_mainpaned.property_max_position();

	m_subpaned.set_position(submin + (submax - submin) * 4 / 5);
	m_mainpaned.set_position(mainmin + (mainmax - mainmin) * 3 / 4);
}

void Gobby::Window::on_session_create() try
{
	HostDialog dlg(*this, m_config);
	if(dlg.run() == Gtk::RESPONSE_OK)
	{
		unsigned int port = dlg.get_port();
		Glib::ustring name = dlg.get_name();
		Gdk::Color color = dlg.get_color();	

		unsigned int red = color.get_red() * 255 / 65535;
		unsigned int green = color.get_green() * 255 / 65535;
		unsigned int blue = color.get_blue() * 255 / 65535;

		// Create new buffer
		obby::host_buffer* buffer = new HostBuffer(
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

		// Running
		m_running = true;
		
		// Delegate start of obby session
		m_header.obby_start();
		m_folder.obby_start();
		m_userlist.obby_start();
		m_chat.obby_start();
		m_statusbar.obby_start();

		// Let the local user join
		on_obby_user_join(buffer->get_self() );
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
		Glib::ustring host = dlg.get_host();
		unsigned int port = dlg.get_port();
		Glib::ustring name = dlg.get_name();
		Gdk::Color color = dlg.get_color();

		unsigned int red = color.get_red() * 255 / 65535;
		unsigned int green = color.get_green() * 255 / 65535;
		unsigned int blue = color.get_blue() * 255 / 65535;

		// TODO: Keep existing connection if host and port did not
		// change
		obby::client_buffer* buffer = new ClientBuffer(host, port);

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

		buffer->login(name, red, green, blue);
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
	if(m_buffer)
	{
		if(m_running)
		{
			m_header.obby_end();
			m_folder.obby_end();
			m_userlist.obby_end();
			m_chat.obby_end();
			m_statusbar.obby_end();

			m_running = false;
		}

		delete m_buffer;
		m_buffer = NULL;
	}
}

void Gobby::Window::on_about()
{
	Gtk::AboutDialog dlg;
	dlg.set_name("Gobby");
	dlg.set_version(PACKAGE_VERSION);
	dlg.set_comments(_("A collaborative text editor"));
	dlg.set_copyright("Copyright (C) 2005 0x539 dev group <crew@0x539.de>");
#ifndef WITH_GNOME
	dlg.set_logo(Gdk::Pixbuf::create_from_inline(512 * 128, Icon::gobby) );
#else
	dlg.set_logo(Gdk::Pixbuf::create_from_file(
		"/usr/share/pixmaps/gobby.png") );
#endif

	std::deque<Glib::ustring> authors;
	authors.push_back("Armin Burgmeier <armin@0x539.de>");
	authors.push_back("Benjamin Herr <ben@0x539.de>");
	authors.push_back("Philipp Kern <phil@0x539.de>");
	
	std::deque<Glib::ustring> artists;
	artists.push_back("Thomas Glatt <tom@0x539.de>");

	dlg.set_authors(authors);
	dlg.set_artists(artists);

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
	EntryDialog dlg(*this, _("Create document"), "Enter document name");
	if(dlg.run() == Gtk::RESPONSE_OK)
	{
		m_buffer->create_document(dlg.get_text() );
	}
}

void Gobby::Window::on_document_open()
{
	Gtk::FileChooserDialog dlg(*this, _("Open new document"));
	dlg.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	dlg.add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);

	if(dlg.run() == Gtk::RESPONSE_OK)
	{
		m_buffer->create_document(
			Glib::path_get_basename(dlg.get_filename()),
			Glib::file_get_contents(dlg.get_filename()) );
	}
}

void Gobby::Window::on_document_save()
{
	Widget* page = m_folder.get_nth_page(m_folder.get_current_page() );
	Document& doc = *static_cast<Document*>(page);

	Gtk::FileChooserDialog dlg(*this, _("Save current document"),
			Gtk::FILE_CHOOSER_ACTION_SAVE);
	dlg.set_current_name(m_folder.get_tab_label_text(doc) );
	dlg.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	dlg.add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);

	if(dlg.run() == Gtk::RESPONSE_OK)
	{
		std::ofstream stream(dlg.get_filename().c_str() );

		if(stream)
		{
			stream << doc.get_content() << std::endl;
		}
		else
		{
			display_error(
				"Could not open file " +
				dlg.get_filename() +
				" for writing"
			);
		}
	}
}

void Gobby::Window::on_document_close()
{
	if(m_buffer)
	{
		// Get current page
		Widget* page = m_folder.get_nth_page(
			m_folder.get_current_page()
		);

		// Send remove document request
		m_buffer->remove_document(
			static_cast<Document*>(page)->get_document()
		);
	}
	else
	{
		// Buffer does not exist: Maybe the connection has been lost
		// or something: Just remove the document from the folder.
		Gtk::Widget* doc = m_folder.get_nth_page(
			m_folder.get_current_page()
		);

		m_folder.remove_page(m_folder.get_current_page() );
		delete doc;

		// If there are no more documents disable
		// save and close buttons in header
		if(!m_folder.get_n_pages() )
			m_header.disable_document_actions();
	}
}

#ifdef WITH_GTKSOURCEVIEW
void Gobby::Window::on_document_line_numbers()
{
	// Get current page
	Widget* page = m_folder.get_nth_page(m_folder.get_current_page() );
	Document* doc = static_cast<Document*>(page);

	// Toggle line number flag
	doc->set_show_line_numbers(!doc->get_show_line_numbers() );
}
#endif

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

/*void Gobby::Window::on_document_update(Document& document)
{
	// Update statusbar
	m_statusbar.update(document);
}*/

void Gobby::Window::on_obby_login_failed(const std::string& reason)
{
	display_error(reason);
	on_session_join();
}

void Gobby::Window::on_obby_close()
{
	display_error(_("Connection lost"));
	on_session_quit();
}

void Gobby::Window::on_obby_sync()
{
	// Send documents to components
	obby::buffer::document_iterator iter = m_buffer->document_begin();
	for(; iter != m_buffer->document_end(); ++ iter)
		on_obby_document_insert(*iter);

	// Set last page as active one because it is currently shown anyway.
	m_folder.set_current_page(m_buffer->document_count() - 1);
}

void Gobby::Window::on_obby_chat(obby::user& user, const Glib::ustring& message)
{
	m_chat.obby_message(user, message);
}

void Gobby::Window::on_obby_server_chat(const Glib::ustring& message)
{
	m_chat.obby_server_message(message);
}

void Gobby::Window::on_obby_user_join(obby::user& user)
{
	// Send obby start to GUI components if this is a join command for the
	// local user. The host does not emit such a signal (well it does, but
	// in its constructor - no signal handler could have been connected),
	// so it is only done for the client upon successful login
	if(&m_buffer->get_self() == &user)
	{
		m_header.obby_start();
		m_folder.obby_start();
		m_userlist.obby_start();
		m_chat.obby_start();
		m_statusbar.obby_start();

		m_running = true;
	}

	// Tell user join to components
	m_header.obby_user_join(user);
	m_folder.obby_user_join(user);
	m_userlist.obby_user_join(user);
	m_chat.obby_user_join(user);
	m_statusbar.obby_user_join(user);
}

void Gobby::Window::on_obby_user_part(obby::user& user)
{
	// Tell user part to components
	m_header.obby_user_part(user);
	m_folder.obby_user_part(user);
	m_userlist.obby_user_part(user);
	m_chat.obby_user_part(user);
	m_statusbar.obby_user_part(user);
}

void Gobby::Window::on_obby_document_insert(obby::document& document)
{
	m_header.obby_document_insert(document);
	m_folder.obby_document_insert(document);
	m_userlist.obby_document_insert(document);
	m_chat.obby_document_insert(document);
	m_statusbar.obby_document_insert(document);
}

void Gobby::Window::on_obby_document_remove(obby::document& document)
{
	m_header.obby_document_remove(document);
	m_folder.obby_document_remove(document);
	m_userlist.obby_document_remove(document);
	m_chat.obby_document_remove(document);
	m_statusbar.obby_document_remove(document);
}

void Gobby::Window::display_error(const Glib::ustring& message)
{
	Gtk::MessageDialog dlg(*this, message, false, Gtk::MESSAGE_ERROR,
	                       Gtk::BUTTONS_OK, true);
	dlg.run();
}

