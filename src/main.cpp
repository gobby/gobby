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

#include <iostream>
#include <gtkmm/main.h>
#include <gtkmm/messagedialog.h>
#include "common.hpp"
#include "ipc.hpp"
#include "icon.hpp"
#include "config.hpp"
#include "encoding_selector.hpp"
#include "window.hpp"
#include "features.hpp"

void handle_exception(const Glib::ustring& message)
{
	Gtk::MessageDialog dlg("Unhandled exception: " + message, false,
		Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
	dlg.run();
	std::cerr << "Unhandled exception: " << message << std::endl;
}

std::auto_ptr<Gobby::Ipc::RemoteConnection>
open_files(Gobby::Window& wnd,
           const std::vector<std::string>& files)
{
	std::auto_ptr<Gobby::Ipc::RemoteConnection> conn;

	try
	{
		// Try to connect to other gobby instance
		Gobby::Ipc::RemoteInstance instance;

		conn.reset(
			new Gobby::Ipc::RemoteConnection(instance)
		);
	}
	catch(Gobby::Ipc::Error& e)
	{
		if(e.code() != Gobby::Ipc::Error::NO_REMOTE_INSTANCE)
			throw e;
	}

	if(conn.get() != NULL)
	{
		// Found one, send files. Finish from mainloop as soon as
		// all the files have been sent.
		conn->done_event().connect(sigc::ptr_fun(&Gtk::Main::quit));

		for(std::vector<std::string>::const_iterator iter =
			files.begin();
		    iter != files.end();
		    ++ iter)
		{
			conn->send_file(iter->c_str() );
		}
	}
	else
	{
		// No other gobby found, so open the files locally
		wnd.show();

		// First, open a session with default settings
		if(wnd.session_open(false) )
		{
			// And pass files
			for(std::vector<std::string>::const_iterator iter =
				files.begin();
			    iter != files.end();
			    ++ iter)
			{
				wnd.open_local_file(
					*iter,
					Gobby::EncodingSelector::AUTO_DETECT
				);
			}
		}
	}

	return conn;
}

int main(int argc, char* argv[]) try
{
	setlocale(LC_ALL, "");
	net6::gettext_package gobby_package(GETTEXT_PACKAGE, LOCALE_DIR);
	Gobby::init_gettext(gobby_package);

	// TODO: Options to disable IPC

	Gtk::Main kit(argc, argv);
	net6::main netkit;
	Glib::thread_init();

	// Get files to open
	std::vector<std::string> files(argc - 1);
	for(int i = 1; i < argc; ++ i)
	{
		// Make absolute filenames to understand the files
		// from everywhere when we send them to another process
		files[i - 1] = Glib::build_filename(
			Glib::get_current_dir(),
			argv[i]
		);
	}

	Gobby::IconManager icon_mgr;

	// Set default icon
	std::list<Glib::RefPtr<Gdk::Pixbuf> > icon_list;
	icon_list.push_back(icon_mgr.gobby);
	Gtk::Window::set_default_icon_list(icon_list);

	// Read the configuration
	Gobby::Config config(Glib::get_home_dir() + "/.gobby/config.xml");

	// Create window
	Gobby::Window wnd(icon_mgr, config);

	// Open files passed by command line. Need to keep rem_conn,
	// otherwise the connection would be dropped through
	// Gobby::Ipc::RemoteConnection's destructor.
	std::auto_ptr<Gobby::Ipc::RemoteConnection> rem_conn;
	if(!files.empty() )
		rem_conn = open_files(wnd, files);
	else
		wnd.show();

	// Cannot use just kit.run(wnd) since this would show wnd. If we
	// are just sending some data to theother gobby, we do not want
	// the window to be shown.
	wnd.signal_hide().connect(sigc::ptr_fun(&Gtk::Main::quit) );

	kit.run();
	return 0;
}
catch(Glib::Exception& e)
{
	handle_exception(e.what() );
}
catch(std::exception& e)
{
	handle_exception(e.what() );
}
