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

#include <obby/format_string.hpp>
#include <obby/client_buffer.hpp>
#include <obby/host_buffer.hpp>

#include "common.hpp"
#include "docwindow.hpp"
#include "joindialog.hpp"
#include "hostdialog.hpp"
#include "joinprogressdialog.hpp"
#include "hostprogressdialog.hpp"
#include "entrydialog.hpp"
#include "window.hpp"
#include "features.hpp"
#include "icon.hpp"

Gobby::Window::Window()
 : Gtk::Window(Gtk::WINDOW_TOPLEVEL), 
   m_config(Glib::get_home_dir() + "/.gobby/config.xml"), m_buffer(NULL),
#ifdef WITH_HOWL
   m_zeroconf(NULL),
#endif
   m_header(m_folder), m_statusbar(m_folder)
{
	// Header
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

	m_header.user_set_password_event().connect(
		sigc::mem_fun(*this, &Window::on_user_set_password) );

	m_header.document_word_wrap_event().connect(
		sigc::mem_fun(*this, &Window::on_document_word_wrap) );
#ifdef WITH_GTKSOURCEVIEW
	m_header.document_line_numbers_event().connect(
		sigc::mem_fun(*this, &Window::on_document_line_numbers) );
	m_header.document_language_event().connect(
		sigc::mem_fun(*this, &Window::on_document_language) );
#endif

	m_header.about_event().connect(
		sigc::mem_fun(*this, &Window::on_about) );
	m_header.quit_event().connect(
		sigc::mem_fun(*this, &Window::on_quit) );

	// Folder
	m_folder.document_close_event().connect(
		sigc::mem_fun(*this, &Window::on_folder_document_close) );
	m_folder.tab_switched_event().connect(
		sigc::mem_fun(*this, &Window::on_folder_tab_switched) );

	// Build UI
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
	obby_end();
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

void Gobby::Window::obby_start()
{
	// Connect to obby events
	m_buffer->user_join_event().connect(
		sigc::mem_fun(*this, &Window::on_obby_user_join) );
	m_buffer->user_part_event().connect(
		sigc::mem_fun(*this, &Window::on_obby_user_part) );
	m_buffer->document_insert_event().connect(
		sigc::mem_fun(*this, &Window::on_obby_document_insert));
	m_buffer->document_remove_event().connect(
		sigc::mem_fun(*this, &Window::on_obby_document_remove));

	m_buffer->message_event().connect(
		sigc::mem_fun(*this, &Window::on_obby_chat) );
	m_buffer->server_message_event().connect(
		sigc::mem_fun(*this, &Window::on_obby_server_chat) );

	// Delegate start of obby session
	m_header.obby_start(*m_buffer);
	m_folder.obby_start(*m_buffer);
	m_userlist.obby_start(*m_buffer);
	m_chat.obby_start(*m_buffer);
	m_statusbar.obby_start(*m_buffer);

	// Forward user joins for users that are connected 
	const obby::user_table& user_table = m_buffer->get_user_table();
	for(obby::user_table::user_iterator<obby::user::CONNECTED> iter =
		user_table.user_begin<obby::user::CONNECTED>();
	    iter != user_table.user_end<obby::user::CONNECTED>();
	    ++ iter)
	{
		on_obby_user_join(*iter);
	}

	// Send documents to components
	obby::buffer::document_iterator iter = m_buffer->document_begin();
	for(; iter != m_buffer->document_end(); ++ iter)
		on_obby_document_insert(*iter);

	// Set last page as active one because it is currently shown anyway.
	if(m_buffer->document_count() > 0)
		m_folder.set_current_page(m_buffer->document_count() - 1);
}

void Gobby::Window::obby_end()
{
	// Nothing to do if no buffer is open
	if(!m_buffer.get() ) return;

	// Tell GUI components that the session ended
	m_header.obby_end();
	m_folder.obby_end();
	m_userlist.obby_end();
	m_chat.obby_end();
	m_statusbar.obby_end();

	// Delete buffer and zeroconf
	m_buffer.reset();
#ifdef WITH_HOWL
	delete m_zeroconf;
	m_zeroconf = NULL;
#endif
}

void Gobby::Window::on_session_create()
{
	// Show up host dialog
	HostDialog dlg(*this, m_config);
	if(dlg.run() == Gtk::RESPONSE_OK)
	{
		dlg.hide();

		// Read setting
		unsigned int port = dlg.get_port();
		Glib::ustring name = dlg.get_name();
		Gdk::Color color = dlg.get_color();	
		Glib::ustring password = dlg.get_password();

		// Set up host with hostprogressdialog
		HostProgressDialog prgdlg(*this, m_config, port, name, color);
		if(prgdlg.run() == Gtk::RESPONSE_OK)
		{
			prgdlg.hide();

			// Get buffer
			std::auto_ptr<obby::host_buffer> buffer =
				prgdlg.get_buffer();

			// Set password
			buffer->set_global_password(password);
#ifdef WITH_HOWL
			// Publish the newly created session via ZeroConf
			m_zeroconf = new obby::zeroconf();
			m_zeroconf->publish(name, port);
#endif

			// Start session
			m_buffer = buffer;
			obby_start();
		}
	}
}

void Gobby::Window::on_session_join()
{
	JoinDialog dlg(*this, m_config);
	if(dlg.run() == Gtk::RESPONSE_OK)
	{
		dlg.hide();

		// Read settings
		Glib::ustring host = dlg.get_host();
		unsigned int port = dlg.get_port();
		Glib::ustring name = dlg.get_name();
		Gdk::Color color = dlg.get_color();

		JoinProgressDialog prgdlg(
			*this, m_config, host, port, name, color);
		if(prgdlg.run() == Gtk::RESPONSE_OK)
		{
			prgdlg.hide();

			// Get buffer
			std::auto_ptr<obby::client_buffer> buffer =
				prgdlg.get_buffer();

			buffer->close_event().connect(
				sigc::mem_fun(*this, &Window::on_obby_close) );

			// Start session
			m_buffer = buffer;
			obby_start();
		}
	}
}

void Gobby::Window::on_session_quit()
{
	obby_end();
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

void Gobby::Window::on_folder_document_close(Document& document)
{
	for(int i = 0; i < m_folder.get_n_pages(); ++ i)
	{
		Gtk::Widget* doc = m_folder.get_nth_page(i);
		if(&static_cast<DocWindow*>(doc)->get_document() == &document)
		{
			close_document(*static_cast<DocWindow*>(doc));
			break;
		}
	}
}

void Gobby::Window::on_folder_tab_switched(Document& document)
{
	const Glib::ustring& file = document.get_document().get_title();
	Glib::ustring path = document.get_path();
		
	if(!path.empty() )
	{
		// Replace home dir by ~
		Glib::ustring home = Glib::get_home_dir();
		if(path.compare(0, home.length(), home) == 0)
			path.replace(0, home.length(), "~");

		// Set title with file and path
		obby::format_string title_str("%0 (%1) - Gobby");
		title_str << file << path;
		set_title(title_str.str() );
	}
	else
	{
		// Path not known: Set title with file only
		obby::format_string title_str("%0 - Gobby");
		title_str << file;
		set_title(title_str.str() );
	}
}

void Gobby::Window::on_document_create()
{
	EntryDialog dlg(*this, _("Create document"), _("Enter document name"));
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
		// TODO: Set path in newly generated document
		m_buffer->create_document(
			Glib::path_get_basename(dlg.get_filename()),
			Glib::file_get_contents(dlg.get_filename()) );
	}
}

void Gobby::Window::on_document_save()
{
	// Get page
	Widget* page = m_folder.get_nth_page(m_folder.get_current_page() );
	DocWindow& doc = *static_cast<DocWindow*>(page);

	// Show dialog
	Gtk::FileChooserDialog dlg(*this, _("Save current document"),
			Gtk::FILE_CHOOSER_ACTION_SAVE);
	dlg.set_current_name(m_folder.get_tab_label_text(doc) );
	dlg.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	dlg.add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);

	if(dlg.run() == Gtk::RESPONSE_OK)
	{
		// Build ofstream
		std::ofstream stream(dlg.get_filename().c_str() );

		if(stream)
		{
			// Save content into file
			stream << doc.get_document().get_content() << std::endl;
			doc.get_document().set_path(dlg.get_filename() );
			on_folder_tab_switched(doc.get_document() );
		}
		else
		{
			// File could not be opened
			obby::format_string str("Could not open file "
			                        "%0 for writing");
			str << dlg.get_filename();
			display_error(str.str() );
		}
	}
}

void Gobby::Window::on_document_close()
{
	Widget* page = m_folder.get_nth_page(m_folder.get_current_page() );
	close_document(*static_cast<DocWindow*>(page) );
}

void Gobby::Window::on_user_set_password()
{
	// TODO: Password dialog with second entry field to confirm password
	EntryDialog dlg(
		*this,
		_("Set user password"),
		_("Enter new password")
	);

	dlg.get_entry().set_visibility(false);

	if(dlg.run() == Gtk::RESPONSE_OK)
	{
		dynamic_cast<obby::client_buffer*>(
			m_buffer.get() )->set_password(dlg.get_text() );
	}
}

void Gobby::Window::on_document_word_wrap()
{
	// Get current page
	DocWindow& doc_wnd = *static_cast<DocWindow*>(
		m_folder.get_nth_page(m_folder.get_current_page() )
	);

	// Toggle word wrapping flag
	doc_wnd.get_document().set_word_wrapping(
		!doc_wnd.get_document().get_word_wrapping()
	);
}

#ifdef WITH_GTKSOURCEVIEW
void Gobby::Window::on_document_line_numbers()
{
	// Get current page
	DocWindow& doc_wnd = *static_cast<DocWindow*>(
		m_folder.get_nth_page(m_folder.get_current_page() )
	);

	// Toggle line number flag
	doc_wnd.get_document().set_show_line_numbers(
		!doc_wnd.get_document().get_show_line_numbers()
	);
}

void Gobby::Window::on_document_language(
	const Glib::RefPtr<Gtk::SourceLanguage>& lang
)
{
	// Get current page
	DocWindow& doc = *static_cast<DocWindow*>(
		m_folder.get_nth_page(m_folder.get_current_page() )
	);

	// Set given language
	doc.get_document().set_language(lang);
}
#endif

void Gobby::Window::on_quit()
{
	on_session_quit();
	Gtk::Main::quit();
}

void Gobby::Window::on_chat(const Glib::ustring& message)
{
	m_buffer->send_message(message);
}

void Gobby::Window::on_obby_close()
{
	display_error(_("Connection lost"));
	on_session_quit();
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

void Gobby::Window::on_obby_document_insert(obby::document_info& document)
{
	obby::local_document_info& local_doc =
		dynamic_cast<obby::local_document_info&>(document);

	m_header.obby_document_insert(local_doc);
	m_folder.obby_document_insert(local_doc);
	m_userlist.obby_document_insert(local_doc);
	m_chat.obby_document_insert(local_doc);
	m_statusbar.obby_document_insert(local_doc);
}

void Gobby::Window::on_obby_document_remove(obby::document_info& document)
{
	obby::local_document_info& local_doc =
		dynamic_cast<obby::local_document_info&>(document);

	m_header.obby_document_remove(local_doc);
	m_folder.obby_document_remove(local_doc);
	m_userlist.obby_document_remove(local_doc);
	m_chat.obby_document_remove(local_doc);
	m_statusbar.obby_document_remove(local_doc);

	// Reset title if last document has been closed
	if(m_buffer->document_count() == 1)
		set_title("Gobby");
}

void Gobby::Window::display_error(const Glib::ustring& message)
{
	Gtk::MessageDialog dlg(*this, message, false, Gtk::MESSAGE_ERROR,
	                       Gtk::BUTTONS_OK, true);
	dlg.run();
}

void Gobby::Window::close_document(DocWindow& document)
{
	if(m_buffer.get() != NULL)
	{
		// Send remove document request
		m_buffer->remove_document(
			document.get_document().get_document()
		);
	}
	else
	{
		// Buffer does not exist: Maybe the connection has been lost
		// or something: Just remove the document from the folder.
		m_folder.remove_page(document);

		// If there are no more documents, disable
		// save and close buttons in header
		if(!m_folder.get_n_pages() )
			m_header.disable_document_actions();
	}
}

