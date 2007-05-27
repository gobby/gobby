/* gobby - A GTKmm driven libobby client
 * Copyright (C) 2005-2006 0x539 dev group
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

#include <glibmm/miscutils.h>
#include <gtkmm/main.h>
#include <gtkmm/aboutdialog.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/stock.h>

#include <obby/format_string.hpp>
#include <obby/client_buffer.hpp>
#include <obby/host_buffer.hpp>

#include "common.hpp"
#include "encoding.hpp"
#include "encoding_selector.hpp"
#include "docwindow.hpp"
#include "passworddialog.hpp"
#include "entrydialog.hpp"
#include "preferencesdialog.hpp"
#include "joinprogressdialog.hpp"
#include "hostprogressdialog.hpp"
#include "window.hpp"
#include "features.hpp"
#include "icon.hpp"
#include "colorsel.hpp"

Gobby::Window::Window(const IconManager& icon_mgr, Config& config):
	Gtk::Window(Gtk::WINDOW_TOPLEVEL), m_config(config),
#ifdef WITH_GTKSOURCEVIEW2
	m_lang_manager(gtk_source_language_manager_new()),
#else
	m_lang_manager(gtk_source_languages_manager_new()),
#endif
	m_preferences(m_config, m_lang_manager), m_icon_mgr(icon_mgr),
	m_application_state(APPLICATION_NONE),
	m_document_settings(*this),
	m_header(m_application_state, m_lang_manager),
	m_folder(m_header, m_preferences),
	m_userlist(
		*this,
		m_header,
		m_folder,
		m_preferences,
		config.get_root()["windows"]
	),
	m_documentlist(
		*this,
		m_document_settings,
		m_header,
		m_folder,
		m_preferences,
		config.get_root()["windows"]
	),
	m_chat(*this, m_preferences),
	m_statusbar(m_header, m_folder)
#ifdef WITH_AVAHI
	,m_glib_poll(avahi_glib_poll_new(NULL, G_PRIORITY_DEFAULT))
#endif
{
	// Header
	m_header.action_app_session_create->signal_activate().connect(
		sigc::mem_fun(*this, &Window::on_session_create) );
	m_header.action_app_session_join->signal_activate().connect(
		sigc::mem_fun(*this, &Window::on_session_join) );
	m_header.action_app_session_save->signal_activate().connect(
		sigc::mem_fun(*this, &Window::on_session_save) );
	m_header.action_app_session_quit->signal_activate().connect(
		sigc::mem_fun(*this, &Window::on_session_quit) );
	m_header.action_app_quit->signal_activate().connect(
		sigc::mem_fun(*this, &Window::on_quit) );

	m_header.action_session_document_create->signal_activate().connect(
		sigc::mem_fun(*this, &Window::on_document_create) );
	m_header.action_session_document_open->signal_activate().connect(
		sigc::mem_fun(*this, &Window::on_document_open) );
	m_header.action_session_document_save->signal_activate().connect(
		sigc::mem_fun(*this, &Window::on_document_save) );
	m_header.action_session_document_save_as->signal_activate().connect(
		sigc::mem_fun(*this, &Window::on_document_save_as) );
	m_header.action_session_document_close->signal_activate().connect(
		sigc::mem_fun(*this, &Window::on_document_close) );

	m_header.action_edit_search->signal_activate().connect(
		sigc::mem_fun(*this, &Window::on_edit_search) );
	m_header.action_edit_search_replace->signal_activate().connect(
		sigc::mem_fun(*this, &Window::on_edit_search_replace) );
	m_header.action_edit_goto_line->signal_activate().connect(
		sigc::mem_fun(*this, &Window::on_edit_goto_line) );
	m_header.action_edit_preferences->signal_activate().connect(
		sigc::mem_fun(*this, &Window::on_edit_preferences) );

	m_header.action_user_set_colour->signal_activate().connect(
		sigc::mem_fun(*this, &Window::on_user_set_colour) );
	m_header.action_user_set_password->signal_activate().connect(
		sigc::mem_fun(*this, &Window::on_user_set_password) );

	m_header.action_edit_document_preferences->signal_activate().connect(
		sigc::mem_fun(*this, &Window::on_view_preferences) );

	m_header.action_window_chat->signal_activate().connect(
		sigc::mem_fun(*this, &Window::on_window_chat) );

	m_header.action_help_about->signal_activate().connect(
		sigc::mem_fun(*this, &Window::on_about) );

	// Folder
	m_folder.document_add_event().connect(
		sigc::mem_fun(*this, &Window::on_folder_document_add) );
	m_folder.document_remove_event().connect(
		sigc::mem_fun(*this, &Window::on_folder_document_remove) );
	m_folder.document_close_request_event().connect(
		sigc::mem_fun(*this, &Window::on_folder_document_close_request) );
	m_folder.tab_switched_event().connect(
		sigc::mem_fun(*this, &Window::on_folder_tab_switched) );

	// Settings
	m_document_settings.document_insert_event().connect(
		sigc::mem_fun(*this, &Window::on_settings_document_insert) );

	m_conn_chat_realize = m_chat.signal_realize().connect(
		sigc::mem_fun(*this, &Window::on_chat_realize) );

	// Build UI
	add_accel_group(m_header.get_accel_group() );

	m_frame_chat.set_shadow_type(Gtk::SHADOW_IN);
	m_frame_text.set_shadow_type(Gtk::SHADOW_IN);

	m_frame_chat.add(m_chat);
	m_frame_text.add(m_folder);

	m_mainpaned.pack1(m_frame_text, true, false);
	m_mainpaned.pack2(m_frame_chat, true, false);

	m_mainbox.pack_start(m_header, Gtk::PACK_SHRINK);
	m_mainbox.pack_start(m_mainpaned, Gtk::PACK_EXPAND_WIDGET);
	m_mainbox.pack_start(m_statusbar, Gtk::PACK_SHRINK);

	add(m_mainbox);

	// Apply initial preferences
	apply_preferences();

	Config::ParentEntry& windows = config.get_root()["windows"];
	bool show_chat = windows["chat"].get_value<bool>(
		"visible",
		true
	);

	m_header.action_window_chat->set_active(show_chat);
	m_application_state.modify(APPLICATION_INITIAL, APPLICATION_NONE);

	show_all_children();
	if(!show_chat) m_frame_chat.hide_all();

	set_title("Gobby");
	set_default_size(640, 480);

#ifdef WITH_ZEROCONF
	// Initialise Zeroconf
	try
	{
#ifdef WITH_AVAHI
		m_zeroconf.reset(new obby::zeroconf_avahi(avahi_glib_poll_get(m_glib_poll)));
#else
		m_zeroconf.reset(new obby::zeroconf);
		// Periodically check for events when not using Avahi Glib Poll
		Glib::signal_timeout().connect(sigc::bind(sigc::mem_fun(*m_zeroconf.get(), &zeroconf_base::select), 0), 1500);
#endif
	}
	catch(std::runtime_error&)
	{
		std::cerr << _("Zeroconf initialisation failed. Probably you "
			"need to run avahi-daemon or mDNSResponder, depending "
			"on the library you use, as root prior to Gobby. "
			"Zeroconf support is deactivated for this session.");
		std::cerr << std::endl;
		m_zeroconf.reset();
	}
#endif

	if(m_preferences.appearance.remember)
	{
		Config::ParentEntry& screen = config.get_root()["screen"];

		// Restore the window's position from the configuration
        	const int x = windows["main"].get_value<int>("x", 0);
	        const int y = windows["main"].get_value<int>("y", 0);
		const int w = windows["main"].get_value<int>("width", 0);
		const int h = windows["main"].get_value<int>("height", 0);

		const int s_w = screen.get_value<int>("width", 0);
		const int s_h = screen.get_value<int>("height", 0);
		bool first_run = (x == 0 && y == 0 && w == 0 && h == 0);

		Glib::RefPtr<Gdk::Screen> scr(get_screen() );
		if( (scr->get_width() >= s_w && scr->get_height() >= s_h) &&
		    (!first_run) )
		{
			move(x, y);
			resize(w, h);
		}
	}
}

Gobby::Window::~Window()
{
	if(m_buffer.get() && m_buffer->is_open() )
		obby_end();

	// Serialise preferences into config
	m_preferences.serialise(m_config);

	Config::ParentEntry& windows = m_config.get_root()["windows"];
	windows["chat"].set_value(
		"visible",
		m_header.action_window_chat->get_active()
	);

	// Save the window's current position
	if(m_preferences.appearance.remember)
	{
		int x, y, w, h;
		get_position(x, y); get_size(w, h);
		Glib::RefPtr<Gdk::Screen> scr(get_screen() );

		windows["main"].set_value("x", x);
		windows["main"].set_value("y", y);
		windows["main"].set_value("width", w);
		windows["main"].set_value("height", h);

		Config::ParentEntry& screen = m_config.get_root()["screen"];
		screen.set_value("width", scr->get_width() );
		screen.set_value("height", scr->get_height() );
	}

	/* Free explictely to make sure that the avahi poll is no longer
	 * referenced when we free it */
#ifdef WITH_ZEROCONF
	m_zeroconf.reset(NULL);
#endif

#ifdef WITH_AVAHI
	avahi_glib_poll_free(m_glib_poll);
#endif
}

bool Gobby::Window::on_delete_event(GdkEventAny* event)
{
	if(m_buffer.get() == NULL) return false;
	if(!m_buffer->is_open() ) return false;

	Gtk::MessageDialog dlg(
		*this,
		_("You are still connected to a session"),
		false,
		Gtk::MESSAGE_WARNING,
		Gtk::BUTTONS_NONE,
		true
	);

	dlg.set_secondary_text(
		_("Do you want to close Gobby nevertheless?")
	);

	dlg.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	dlg.add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_YES)->grab_focus();

	return dlg.run() != Gtk::RESPONSE_YES;
}

void Gobby::Window::on_realize()
{
	Gtk::Window::on_realize();

	// Create new IPC instance
	try
	{
		m_ipc.reset(new Ipc::LocalInstance);
		m_ipc->file_event().connect(
			sigc::mem_fun(*this, &Window::on_ipc_file)
		);
	}
	catch(net6::error& e)
	{
		// Whatever...
		display_error(e.what() );
	}
}

void Gobby::Window::on_chat_realize()
{
	m_mainpaned.set_position(m_mainpaned.get_height() * 3 / 5);
	m_conn_chat_realize.disconnect();
}

void Gobby::Window::obby_start()
{
	// Connect to obby events
	m_buffer->user_join_event().connect(
		sigc::mem_fun(*this, &Window::on_obby_user_join) );
	m_buffer->user_part_event().connect(
		sigc::mem_fun(*this, &Window::on_obby_user_part) );
	m_buffer->user_colour_event().connect(
		sigc::mem_fun(*this, &Window::on_obby_user_colour) );
	m_buffer->user_colour_failed_event().connect(
		sigc::mem_fun(*this, &Window::on_obby_user_colour_failed) );

	m_buffer->document_insert_event().connect(
		sigc::mem_fun(*this, &Window::on_obby_document_insert) );
	m_buffer->document_remove_event().connect(
		sigc::mem_fun(*this, &Window::on_obby_document_remove) );

	// Accept drag and drop of files into the gobby window
	m_dnd.reset(new DragDrop(*this) );

	// Delegate start of obby session
	m_folder.obby_start(*m_buffer);
	m_documentlist.obby_start(*m_buffer);
	m_document_settings.obby_start(*m_buffer);
	m_userlist.obby_start(*m_buffer);
	m_chat.obby_start(*m_buffer);
	m_statusbar.obby_start(*m_buffer);

	// Forward user joins
	const obby::user_table& table = m_buffer->get_user_table();
	for(obby::user_table::iterator iter =
		table.begin(obby::user::flags::NONE, obby::user::flags::NONE);
	    iter != table.end(obby::user::flags::NONE, obby::user::flags::NONE);
	    ++ iter)
	{
		on_obby_user_join(*iter);
	}

	// Send documents to components
	Buffer::document_iterator iter = m_buffer->document_begin();
	for(; iter != m_buffer->document_end(); ++ iter)
		on_obby_document_insert(*iter);

	// Set last page as active one because it is currently shown anyway.
	//if(m_buffer->document_count() > 0)
	//	m_folder.set_current_page(m_buffer->document_count() - 1);

	// Clear location of previous session file, this is a new session
	m_prev_session = "";

	// Current document has changed, update titlebar
	update_title_bar();

	// Show up document list if obby buffer contains documents
	if(m_buffer->document_count() > 0)
	{
		m_documentlist.show();
		m_documentlist.grab_focus();
	}

	ApplicationFlags inc_flags = APPLICATION_SESSION;
	ApplicationFlags exc_flags = APPLICATION_INITIAL | APPLICATION_DOCUMENT;

	if(dynamic_cast<ClientBuffer*>(m_buffer.get()) != NULL)
		exc_flags |= APPLICATION_HOST;
	else
		inc_flags |= APPLICATION_HOST;

	m_application_state.modify(inc_flags, exc_flags);
}

void Gobby::Window::obby_end()
{
	// Nothing to do if no buffer is open
	if(m_buffer.get() == NULL)
	{
		throw std::logic_error(
			"Gobby::Window::obby_end:\n"
			"Buffer not available"
		);
	}

	m_application_state.modify(APPLICATION_NONE, APPLICATION_SESSION);

	if(m_buffer->is_open() )
	{
		// TODO: Virtual close call in obby?
		ClientBuffer* client_buf =
			dynamic_cast<ClientBuffer*>(m_buffer.get());
		HostBuffer* host_buf =
			dynamic_cast<HostBuffer*>(m_buffer.get());

		if(client_buf != NULL) client_buf->disconnect();
		if(host_buf != NULL) host_buf->close();
	}

	// Remove DND handler
	m_dnd.reset(NULL);

	// Tell GUI components that the session ended
	m_folder.obby_end();
	m_document_settings.obby_end();
	m_userlist.obby_end();
	m_documentlist.obby_end();
	m_chat.obby_end();
	m_statusbar.obby_end();

#ifdef WITH_ZEROCONF
	if(m_zeroconf.get() )
		m_zeroconf->unpublish_all();
#endif
}

void Gobby::Window::on_session_create()
{
	session_open(true);
}

void Gobby::Window::on_session_join()
{
	session_join(true);
}

void Gobby::Window::on_session_save()
{
	Gtk::CheckButton m_chk_default_ext(
		_("Use default .obby extension if none is given")
	);

	Gtk::FileChooserDialog dlg(
		*this,
		_("Save obby session"),
		Gtk::FILE_CHOOSER_ACTION_SAVE
	);

	m_chk_default_ext.set_active(true);
	dlg.get_vbox()->pack_start(m_chk_default_ext, Gtk::PACK_SHRINK);
	// This option confuses the overwrite confirmation :/
	//m_chk_default_ext.show();

#ifdef GTKMM_GEQ_28
	dlg.set_do_overwrite_confirmation(true);
#endif

	// Use the location of a previously saved session, if any
	if(!m_prev_session.empty() )
	{
		dlg.set_filename(m_prev_session);
	}
	else
	{
		// Use the last used path for this dialog, if we have any
		if(!m_last_path.empty() )
			dlg.set_current_folder(m_last_path);
	}

	dlg.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	dlg.add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);

	if(dlg.run() == Gtk::RESPONSE_OK)
	{
		// Use current folder as standard folder for other dialogs
		m_last_path = dlg.get_current_folder();
		// Get selected filename
		std::string filename = dlg.get_filename();

		// Append .obby extension if none is given
		/*if(m_chk_default_ext.get_active() )
			if(filename.find('.') == std::string::npos)
				filename += ".obby";*/

		// Save document
		try
		{
			m_buffer->serialise(filename);
			m_prev_session = filename;
		}
		catch(std::exception& e)
		{
			display_error(e.what() );
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

	dlg.set_copyright(
		"Copyright (C) 2005-2007 0x539 dev group <crew@0x539.de>"
	);

	dlg.set_logo(m_icon_mgr.gobby);

	std::deque<Glib::ustring> authors;
	authors.push_back("Developers:");
	authors.push_back("  Armin Burgmeier <armin@0x539.de>");
	authors.push_back("  Philipp Kern <phil@0x539.de>");
	authors.push_back("");
	authors.push_back("Contributors:");
	authors.push_back("  Benjamin Herr <ben@0x539.de>");
	
	std::deque<Glib::ustring> artists;
	artists.push_back("Logo:");
	artists.push_back("  Thomas Glatt <tom@0x539.de>");
	artists.push_back("");
	artists.push_back("Additional artwork:");
	artists.push_back("  Thomas Glatt <tom@0x539.de>");
	artists.push_back("  Benjamin Herr <ben@0x539.de>");

	dlg.set_authors(authors);
	dlg.set_artists(artists);

	dlg.set_translator_credits(
		"French translation:\n"
		"  Peer Janssen <peer@baden-online.de>\n"
		"  Mohammed Adnene Trojette <adn@diwi.org>\n"
		"\n"
		"Spanish translation:\n"
		"  Mario Palomo Torrero <mpalomo@ihman.com>\n"
		"\n"
		"Swedish translation:\n"
		"  Daniel Nylander <po@danielnylander.se>\n"
		"\n"
		"German translation:\n"
		"  Philipp Kern <phil@0x539.de>\n"
		"  Armin Burgmeier <armin@0x539.de>\n"
		"  Thomas Glatt <tom@0x539.de>\n"
		);

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

void Gobby::Window::on_folder_document_add(DocWindow& window)
{
	// Select newly created page if not automatically opened
	if(!m_document_settings.get_automatically_opened(window.get_info() ))
	{
		m_folder.set_current_page(m_folder.page_num(window) );
		window.grab_focus();
	}

	// Unset modifified flag when locally opened
	if(!m_local_file_path.empty())
  {
    gtk_text_buffer_set_modified(
      GTK_TEXT_BUFFER(window.get_document().get_buffer()), FALSE);
  }

	if(m_folder.get_n_pages() == 1)
	{
		// There have not been any documents before
		m_application_state.modify(
			APPLICATION_DOCUMENT,
			APPLICATION_NONE
		);

		m_folder.set_show_tabs(false);
	}
	else
	{
		m_folder.set_show_tabs(true);
	}
}

void Gobby::Window::on_folder_document_remove(DocWindow& window)
{
	// Update title bar if there are no more documents left
	// (folder_tab_switched is not emitted in this case)
	if(m_folder.get_n_pages() == 0)
	{
		update_title_bar();

		m_application_state.modify(
			APPLICATION_NONE,
			APPLICATION_DOCUMENT
		);
	}
	else if(m_folder.get_n_pages() == 1)
	{
		m_folder.set_show_tabs(false);
	}
}

void Gobby::Window::on_folder_document_close_request(DocWindow& window)
{
	close_document(window);
}

void Gobby::Window::on_folder_tab_switched(DocWindow& window)
{
	// Update title bar
	update_title_bar();
}

void Gobby::Window::on_settings_document_insert(LocalDocumentInfo& info)
{
	// Mark automatically opened documents and subscribe to them.
	if(m_preferences.behaviour.auto_open_new_documents
		&& !info.is_subscribed() )
	{
		m_document_settings.set_automatically_opened(info, true);
		info.subscribe();
	}

	// Set the path from which this document was opened,
	// if we opened that file.
	if(info.get_owner() == &m_buffer->get_self() &&
	   !m_local_file_path.empty() )
	{
		// " " is newly created, so we do not need a path
		if(m_local_file_path != " ")
		{
			m_document_settings.set_path(info, m_local_file_path);
		}

		m_document_settings.set_original_encoding(
			info,
			m_local_encoding
		);
	}
	else
	{
		// File was opened remotely, so we do not know anything
		// about the original encoding, so assume it's UTF-8.
		m_document_settings.set_original_encoding(info, "UTF-8");
	}
}

void Gobby::Window::on_document_create()
{
	EntryDialog dlg(*this, _("Create document"), _("Enter document name"));
	dlg.set_check_valid_entry(true);

	if(dlg.run() == Gtk::RESPONSE_OK)
	{
		// " " means a newly created file
		m_local_file_path = " ";
		m_local_encoding = "UTF-8";
		// Create new document
		m_buffer->document_create(dlg.get_text(), "UTF-8", "");

		// Clear local path
		m_local_file_path.clear();
		m_local_encoding.clear();
	}
}

void Gobby::Window::on_document_open()
{
	// Create FileChooser
	EncodingFileChooserDialog dlg(
		*this,
		_("Open new document"),
		Gtk::FILE_CHOOSER_ACTION_OPEN
	);

	dlg.get_selector().set_encoding(EncodingSelector::AUTO_DETECT);

	// Use the last used path for this dialog, if we have any
	if(!m_last_path.empty() )
		dlg.set_current_folder(m_last_path);

	// Create buttons to close it
	dlg.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	dlg.add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);

	// Allow multi selection
	dlg.set_select_multiple(true);

	// Show FileChooser
	if(dlg.run() == Gtk::RESPONSE_OK)
	{
		// Use current folder as standard folder for later dialogs
		m_last_path = dlg.get_current_folder();
		// Open chosen files
		std::list<Glib::ustring> list = dlg.get_filenames();
		for(std::list<Glib::ustring>::iterator iter = list.begin();
		    iter != list.end();
		    ++ iter)
		{
			open_local_file(
				*iter,
				dlg.get_selector().get_encoding()
			);
		}
	}
}

void Gobby::Window::on_document_save()
{
	// Get page
	DocWindow* doc = get_current_document();
	if(doc == NULL)
	{
		throw std::logic_error(
			"Gobby::Window::on_document_save:\n"
			"No document opened"
		);
	}

	// Is there already a path for this document?
	std::string path = m_document_settings.get_path(doc->get_info() );
	if(!path.empty() )
	{
		// Yes, so save the document there
		save_local_file(
			*doc,
			path,
			m_document_settings.get_original_encoding(
				doc->get_info()
			)
		);
	}
	else
	{
		// Open save as dialog otherwise
		on_document_save_as();
	}
}

void Gobby::Window::on_document_save_as()
{
	// Get page
	DocWindow* doc = get_current_document();
	if(doc == NULL)
	{
		throw std::logic_error(
			"Gobby::Window::on_document_save_as:\n"
			"No document opened"
		);
	}

	// Setup dialog
	EncodingFileChooserDialog dlg(
		*this,
		_("Save current document"),
		Gtk::FILE_CHOOSER_ACTION_SAVE
	);

	// TODO: Preselect document's encoding
	dlg.get_selector().set_encoding(
		m_document_settings.get_original_encoding(doc->get_info())
	);

#ifdef GTKMM_GEQ_28
	dlg.set_do_overwrite_confirmation(true);
#endif

	std::string path = m_document_settings.get_path(doc->get_info() );

	// Does the document have already a path?
	if(!path.empty() )
	{
		// Yes, so set it as filename
		dlg.set_filename(path);
	}
	else
	{
		// No, so use the last path a filesel dialog was closed with
		if(!m_last_path.empty() )
			dlg.set_current_folder(m_last_path);

		// Set current title as proposed file name
		dlg.set_current_name(doc->get_info().get_title() );
	}

	// Add buttons to close the dialog
	dlg.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	dlg.add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);

	if(dlg.run() == Gtk::RESPONSE_OK)
	{
		// Use current folder as standard folder for other dialogs
		m_last_path = dlg.get_current_folder();
		// Save document
		save_local_file(
			*doc,
			dlg.get_filename(),
			dlg.get_selector().get_encoding()
		);
	}
}

void Gobby::Window::on_document_close()
{
	// Get current page
	Widget* page = m_folder.get_nth_page(m_folder.get_current_page() );
	// Close it
	close_document(*static_cast<DocWindow*>(page) );
}

void Gobby::Window::on_edit_search()
{
	if(m_finddialog.get() == NULL)
		m_finddialog.reset(new FindDialog(*this));

	m_finddialog->set_search_only(true);
	m_finddialog->present();
}

void Gobby::Window::on_edit_search_replace()
{
	if(m_finddialog.get() == NULL)
		m_finddialog.reset(new FindDialog(*this));

	m_finddialog->set_search_only(false);
	m_finddialog->present();
}

void Gobby::Window::on_edit_goto_line()
{
	if(m_gotodialog.get() == NULL)
		m_gotodialog.reset(new GotoDialog(*this));

	m_gotodialog->present();
}

void Gobby::Window::on_edit_preferences()
{
	PreferencesDialog dlg(*this, m_preferences, m_lang_manager, false);

	// Info label
	Gtk::Label lbl_info(_(
		"Click on \"Apply\" to apply the new settings to documents "
		"that are currently open. \"OK\" will just store the values "
		"to use them with newly created documents."
	) );

	// Show info label and apply button if documents are open
	if(m_buffer.get() && m_buffer->document_count() > 0)
	{
		// TODO: How to get the label to use all available space?
		lbl_info.set_line_wrap(true);
		lbl_info.set_alignment(Gtk::ALIGN_LEFT);

		dlg.get_vbox()->pack_start(lbl_info, Gtk::PACK_SHRINK);
		dlg.add_button(Gtk::Stock::APPLY, Gtk::RESPONSE_APPLY);
		lbl_info.show();
	}

	int result = dlg.run();
	if(result == Gtk::RESPONSE_OK || result == Gtk::RESPONSE_APPLY)
	{
		// Use new preferences
		Preferences prefs;
		dlg.set(prefs);
		m_preferences = prefs;

		// Apply window preferences
		apply_preferences();

		// Apply preferences to open documents.
		if(result == Gtk::RESPONSE_APPLY)
		{
			for(int i = 0; i < m_folder.get_n_pages(); ++ i)
			{
				DocWindow& doc = *static_cast<DocWindow*>(
					m_folder.get_nth_page(i) );
				doc.set_preferences(m_preferences);
			}
		}
	}
}

void Gobby::Window::on_user_set_password()
{
	// Build password dialog with info
	PasswordDialog dlg(*this, _("Set user password") );
	dlg.set_info(_(
		"Set a user password for your user account. When you try to "
		"login next time with this user, you will be prompted for your "
		"password."
	) );

	// Run it
	if(dlg.run() == Gtk::RESPONSE_OK)
	{
		dynamic_cast<ClientBuffer*>(
			m_buffer.get() )->set_password(dlg.get_password() );
	}
}

void Gobby::Window::on_user_set_colour()
{
	// Simple ColorSelectionDialog
	ColorSelectionDialog dlg(m_config.get_root() );
	const obby::user& user = m_buffer->get_self();
	Gdk::Color color;

	color.set_red(user.get_colour().get_red() * 65535 / 255);
	color.set_green(user.get_colour().get_green() * 65535 / 255);
	color.set_blue(user.get_colour().get_blue() * 65535 / 255);
	dlg.get_colorsel()->set_current_color(color);

	// Run it
	if(dlg.run() == Gtk::RESPONSE_OK)
	{
		// Convert GDK color to obby color, set new color
		Gdk::Color color = dlg.get_colorsel()->get_current_color();
		m_buffer->set_colour(
			obby::colour(
				color.get_red() * 255 / 65535,
				color.get_green() * 255 / 65535,
				color.get_blue() * 255 / 65535
			)
		);
	}
}

void Gobby::Window::on_view_preferences()
{
	// Get current page
	DocWindow* doc = get_current_document();
	if(doc == NULL)
	{
		throw std::logic_error(
			"Gobby::Window::on_view_preferences:\n"
			"No window opened"
		);
	}

	// Add preferences dialog
	PreferencesDialog dlg(*this,
		doc->get_preferences(),
		m_lang_manager,
		true
	);

	// Label text
	obby::format_string str(_(
		"These preferences affect only the currently active document "
		"\"%0%\". If you want to change global preferences, use the "
		"preferences menu item in the \"Edit\" menu."
	) );

	// Get title
	str << doc->get_info().get_suffixed_title();

	// Info label
	Gtk::Label lbl_info(str.str() );

	// TODO: How to get the label to use all available space?
	lbl_info.set_line_wrap(true);
	lbl_info.set_alignment(Gtk::ALIGN_LEFT);

	// Add it into the dialog
	dlg.get_vbox()->pack_start(lbl_info, Gtk::PACK_SHRINK);
	dlg.get_vbox()->reorder_child(lbl_info, 0); // Push to top of dialog
	lbl_info.show();

	// Show the dialog
	if(dlg.run() == Gtk::RESPONSE_OK)
	{
		// Apply new preferences to the document
		Preferences prefs;
		dlg.set(prefs);
		doc->set_preferences(prefs);
	}
}

void
Gobby::Window::on_view_language(GtkSourceLanguage* language)
{
	// Set language of current document
	DocWindow* doc = get_current_document();
	if(doc == NULL)
	{
		throw std::logic_error(
			"Gobby::Window::on_view_language:\n"
			"No window opened"
		);
	}

	doc->set_language(language);
}

void Gobby::Window::on_window_chat()
{
	if(m_header.action_window_chat->get_active() )
	{
		m_frame_chat.show_all();
	}
	else
	{
		m_frame_chat.hide_all();
	}
}

void Gobby::Window::on_quit()
{
	if(on_delete_event(NULL) == false)
	{
		// Quit session
		if(m_buffer.get() != NULL && m_buffer->is_open() )
			obby_end();
		// End program
		Gtk::Main::quit();
	}
}

void Gobby::Window::on_obby_close()
{
	display_error(_("Connection lost"));
	on_session_quit();
}

/*void Gobby::Window::on_obby_encrypted()
{
	display_error("Connection now encrypted");
}*/

void Gobby::Window::on_obby_user_join(const obby::user& user)
{
	// Tell user join to components
	m_folder.obby_user_join(user);
	m_userlist.obby_user_join(user);
	m_documentlist.obby_user_join(user);
	m_chat.obby_user_join(user);
	m_statusbar.obby_user_join(user);
}

void Gobby::Window::on_obby_user_part(const obby::user& user)
{
	// Tell user part to components
	m_folder.obby_user_part(user);
	m_userlist.obby_user_part(user);
	m_documentlist.obby_user_part(user);
	m_chat.obby_user_part(user);
	m_statusbar.obby_user_part(user);
}

void Gobby::Window::on_obby_user_colour(const obby::user& user)
{
	m_userlist.obby_user_colour(user);
	m_documentlist.obby_user_colour(user);
	m_folder.obby_user_colour(user);
}

void Gobby::Window::on_obby_user_colour_failed()
{
	display_error(_("Colour change failed: Colour already in use") );
}

void Gobby::Window::on_obby_document_insert(DocumentInfo& document)
{
	LocalDocumentInfo& local_doc =
		dynamic_cast<LocalDocumentInfo&>(document);

	m_folder.obby_document_insert(local_doc);
	m_userlist.obby_document_insert(local_doc);
	m_documentlist.obby_document_insert(local_doc);
	m_chat.obby_document_insert(local_doc);
	m_statusbar.obby_document_insert(local_doc);
}

void Gobby::Window::on_obby_document_remove(DocumentInfo& document)
{
	LocalDocumentInfo& local_doc =
		dynamic_cast<LocalDocumentInfo&>(document);

	m_folder.obby_document_remove(local_doc);
	m_userlist.obby_document_remove(local_doc);
	m_documentlist.obby_document_remove(local_doc);
	m_chat.obby_document_remove(local_doc);
	m_statusbar.obby_document_remove(local_doc);
}

void Gobby::Window::on_ipc_file(const std::string& file)
{
	// Open local file directly if buffer is open
	if(m_buffer.get() != NULL && m_buffer->is_open() )
	{
		open_local_file(file, EncodingSelector::AUTO_DETECT);
		return;
	}

	// Otherwise, push the file back into the file queue.
	bool was_empty = m_file_queue.empty();
	m_file_queue.push(file);

	// If the file queue is empty, open a new session. The queue will
	// be cleared either if the session has finished (either with success
	// or not).

	// TODO: Find a better condition for when the session is currently
	// being opened. Checking whether the file queue is empty is not
	// good because the user might manually open a session while an
	// IPC file request comes in...
	if(was_empty)
	{
		session_open(false);
	}
}

Gobby::DocWindow* Gobby::Window::get_current_document()
{
	if(m_folder.get_n_pages() == 0) return NULL;

	Widget* page = m_folder.get_nth_page(m_folder.get_current_page() );
	return static_cast<DocWindow*>(page);
}

void Gobby::Window::apply_preferences()
{
	m_header.get_toolbar().set_toolbar_style(
		m_preferences.appearance.toolbar_show);
}

void Gobby::Window::update_title_bar()
{
	// No document
	if(m_folder.get_n_pages() == 0)
	{
		set_title("Gobby");
		return;
	}

	// Get currently active document
	const DocWindow& window = *get_current_document();
	// Get title of current document
	const Glib::ustring& file = window.get_info().get_suffixed_title();
	// Get path of current document
	Glib::ustring path = m_document_settings.get_path(window.get_info() );

	// Show path in title, if we know it
	if(!path.empty() )
	{
		// Replace home dir by ~
		Glib::ustring home = Glib::get_home_dir();
		if(path.compare(0, home.length(), home) == 0)
			path.replace(0, home.length(), "~");

		// Set title with file and path
		obby::format_string title_str("%0% (%1%) - Gobby");
		title_str << file << Glib::path_get_dirname(path);
		set_title(title_str.str() );
	}
	else
	{
		// Path not known: Set title with file only
		obby::format_string title_str("%0% - Gobby");
		title_str << file;
		set_title(title_str.str() );
	}
}

namespace
{
	// convert2unix converts a given string from any special line endings
	// (DOS or old-style Macintosh) to Unix line endings. It does no
	// book-keeping about the encountered endings but ensures that no
	// CR characters are left in the string.
	void convert2unix(std::string& str)
	{
		for(std::string::size_type i = 0; i < str.length(); ++ i)
			// Convert DOS CRLF to a single LF
			if(i < str.length() - 1 &&
			   str[i] == '\r' && str[i+1] == '\n')
				str.erase(i, 1);
			// Convert Macintosh CR to LF
			else if(str[i] == '\r')
				str[i] = '\n';
	}
}

bool Gobby::Window::session_join(bool initial_dialog)
{
	if(m_buffer.get() && m_buffer->is_open() )
	{
		throw std::logic_error(
			"Gobby::Window::session_join:\n"
			"Buffer is already open"
		);
	}

	if(m_join_dlg.get() == NULL)
	{
#ifndef WITH_ZEROCONF
		m_join_dlg.reset(
			new JoinDialog(*this, m_config.get_root()["session"])
		);
#else
		m_join_dlg.reset(
			new JoinDialog(
				*this,
				m_config.get_root()["session"],
				m_zeroconf.get()
			)
		);
#endif
	}

	int response = Gtk::RESPONSE_OK;
	if(initial_dialog) response = m_join_dlg->run();

	while(response == Gtk::RESPONSE_OK)
	{
		// Read settings
		Glib::ustring host = m_join_dlg->get_host();
		unsigned int port = m_join_dlg->get_port();
		Glib::ustring name = m_join_dlg->get_name();
		Gdk::Color color = m_join_dlg->get_color();

		if(session_join_impl(host, port, name, color) )
			break;
		else
			response = m_join_dlg->run();
	}

	m_join_dlg->hide();
	return (m_buffer.get() && m_buffer->is_open() );
}

bool Gobby::Window::session_open(bool initial_dialog)
{
	if(m_buffer.get() && m_buffer->is_open() )
	{
		throw std::logic_error(
			"Gobby::Window::session_open:\n"
			"Buffer is already open"
		);
	}

	if(m_host_dlg.get() == NULL)
	{
		m_host_dlg.reset(
			new HostDialog(*this, m_config.get_root()["session"])
		);
	}

	int response = Gtk::RESPONSE_OK;
	if(initial_dialog) response = m_host_dlg->run();

	while(response == Gtk::RESPONSE_OK)
	{
		// Read setting
		unsigned int port = m_host_dlg->get_port();
		Glib::ustring name = m_host_dlg->get_name();
		Gdk::Color color = m_host_dlg->get_color();
		Glib::ustring password = m_host_dlg->get_password();
		Glib::ustring session = m_host_dlg->get_session();

		if(session_open_impl(port, name, color, password, session) )
			break;
		else
			response = m_host_dlg->run();
	}

	// Process file queue (files that have been queued for opening
	// after session creation).
	m_host_dlg->hide();
	while(!m_file_queue.empty() )
	{
		std::string str = m_file_queue.front();
		m_file_queue.pop();

		if(m_buffer.get() && m_buffer->is_open() )
			open_local_file(str, EncodingSelector::AUTO_DETECT);
	}

	return (m_buffer.get() && m_buffer->is_open() );
}

bool Gobby::Window::session_join_impl(const Glib::ustring& host,
                                      unsigned int port,
                                      const Glib::ustring& name,
                                      const Gdk::Color& color)
{
	JoinProgressDialog prgdlg(
		*this,
		m_config.get_root()["session"],
		host,
		port,
		name,
		color
	);

	if(prgdlg.run() == Gtk::RESPONSE_OK)
	{
		prgdlg.hide();

		// Get buffer
		std::auto_ptr<ClientBuffer> buffer = prgdlg.get_buffer();

		buffer->set_enable_keepalives(true);

		buffer->close_event().connect(
			sigc::mem_fun(*this, &Window::on_obby_close) );

		obby::format_string str(_("Connected to %0%:%1%") );
		str << host << port;
		m_statusbar.update_connection(str.str() );

		// Start session
		m_buffer = buffer;
		obby_start();

		// Session is open, no need to reshow join dialog
		return true;
	}
	else
	{
		return false;
	}
}

bool Gobby::Window::session_open_impl(unsigned int port,
                                      const Glib::ustring& name,
                                      const Gdk::Color& color,
                                      const Glib::ustring& password,
                                      const Glib::ustring& session)
{
	// Set up host with hostprogressdialog
	HostProgressDialog prgdlg(*this, m_config, port, name, color, session);

	if(prgdlg.run() == Gtk::RESPONSE_OK)
	{
		prgdlg.hide();

		// Get buffer
		std::auto_ptr<HostBuffer> buffer =
			prgdlg.get_buffer();

		// Set password
		buffer->set_global_password(password);
		buffer->set_enable_keepalives(true);
#ifdef WITH_ZEROCONF
		// Publish the newly created session via Zeroconf
		// if Howl is not deactivated
		if(m_zeroconf.get() )
			m_zeroconf->publish(name, port);
#endif

		obby::format_string str(_("Serving on port %0%") );
		str << port;
		m_statusbar.update_connection(str.str() );

		m_buffer = buffer;

		// Start session
		obby_start();
		// Remember session file
		m_prev_session = Glib::filename_from_utf8(session);
		// Session is open, no need to reshow host dialog
		return true;
	}
	else
	{
		// Session opening did not succeed
		return false;
	}
}

void Gobby::Window::open_local_file(const Glib::ustring& file,
                                    const std::string& encoding)
{
	try
	{
		// Set local file path for the document_insert callback
		m_local_file_path = file;
		m_local_encoding = encoding;

		std::string utf8_content;
		if(encoding == EncodingSelector::AUTO_DETECT)
		{
			std::string detected_encoding;

			utf8_content = Encoding::convert_to_utf8(
					Glib::file_get_contents(file),
					detected_encoding
			);

			m_local_encoding = detected_encoding;
		}
		else
		{
			utf8_content = Glib::convert(
				Glib::file_get_contents(file),
				"UTF-8",
				encoding
			);
		}

		convert2unix(utf8_content);

		m_buffer->document_create(
			Glib::path_get_basename(file), "UTF-8", utf8_content
		);

		// Clear local path
		m_local_file_path.clear();
		m_local_encoding.clear();
	}
	catch(Glib::Exception& e)
	{
		// Show errors while opening the file (e.g. if it doesn't exist)
		display_error(e.what() );
	}
}

void Gobby::Window::save_local_file(DocWindow& doc,
                                    const Glib::ustring& file,
                                    const std::string& encoding)
{
	try
	{
		Glib::RefPtr<Glib::IOChannel> channel = Glib::IOChannel::create_from_file(file, "w");

		// Save content into file
		std::string conv_content = doc.get_content().raw();
		if(encoding != "UTF-8")
		{
			conv_content = Glib::convert(
				conv_content,
				encoding,
				"UTF-8"
			);
		}

		channel->write(conv_content);
		channel->close();

		m_document_settings.set_path(doc.get_info(), file);

		m_document_settings.set_original_encoding(
			doc.get_info(),
			encoding
		);

		// Update title bar according to new path
		update_title_bar();
		// Unset modifified flag
		gtk_text_buffer_set_modified(
      GTK_TEXT_BUFFER(doc.get_document().get_buffer()), FALSE);
	}
	catch(Glib::Error& e)
	{
		display_error(e.what() );
	}
	catch(std::exception& e)
	{
		display_error(e.what() );
	}
}

void Gobby::Window::close_document(DocWindow& window)
{
	// Check for the document being modified
	if(window.get_modified() )
	{
		// Setup confirmation strings
		obby::format_string primary_str(
			_("Save changes to document \"%0%\" before closing?")
		);

		// TODO: Tell that resubscription is not possible when the
		// session is closed (unless you are host).

		std::string secondary_str;
		if(m_buffer.get() != NULL && m_buffer->is_open() )
		{
			secondary_str = _(
				"If you don't save, changes will be "
				"discarded, but may still be retrieved if "
				"you re-subscribe to the document as long "
				"as the session remains open."
			);
		}
		else
		{
			secondary_str = _(
				"If you don't save, changes will be "
				"discarded."
			);
		}

		primary_str << window.get_info().get_suffixed_title();

		// Setup dialog
		Gtk::MessageDialog dlg(*this, primary_str.str(), false,
			Gtk::MESSAGE_WARNING, Gtk::BUTTONS_NONE, true);

		// Set secondary text
		dlg.set_secondary_text(secondary_str);

		// Add button to allow the user to save the dialog
		dlg.add_button(_("Close without saving"), Gtk::RESPONSE_REJECT);
		dlg.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
		dlg.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_ACCEPT)->
			grab_focus();

		// Show the dialog
		int result = dlg.run();

		switch(result)
		{
		case Gtk::RESPONSE_REJECT:
			/* Close the document */
			break;
		case Gtk::RESPONSE_ACCEPT:
			/* Save the document before closing it */
			m_folder.set_current_page(m_folder.page_num(window) );
			// TODO: Do not close the document if the user cancells
			// the save dialog.
			on_document_save();
			break;
		case Gtk::RESPONSE_CANCEL:
		case Gtk::RESPONSE_DELETE_EVENT:
			/* Do not close the document */
			return;
			break;
		default:
			throw std::logic_error("Gobby::Window::close_document");
			break;
		}
	}

	window.get_info().unsubscribe();
}

void Gobby::Window::display_error(const Glib::ustring& message,
                                  const Gtk::MessageType type)
{
	Gtk::MessageDialog dlg(*this, message, false, type,
	                       Gtk::BUTTONS_OK, true);
	dlg.run();
}
