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
#include <glibmm/optionentry.h>
#include <glibmm/optiongroup.h>
#include <glibmm/optioncontext.h>
#include "common.hpp"
#include "ipc.hpp"
#include "icon.hpp"
#include "config.hpp"
#include "encoding_selector.hpp"
#include "window.hpp"
#include "features.hpp"

#include "sourceview/private/sourcelanguage_p.hpp"
#include "sourceview/private/sourcelanguagesmanager_p.hpp"
#include "sourceview/private/sourcebuffer_p.hpp"
#include "sourceview/private/sourceview_p.hpp"

#ifdef WITH_GNOME
# include <libgnomevfs/gnome-vfs-init.h>
#endif

void handle_exception(const Glib::ustring& message)
{
	Gtk::MessageDialog dlg("Unhandled exception: " + message, false,
		Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
	dlg.run();
	std::cerr << "Unhandled exception: " << message << std::endl;
}

std::auto_ptr<Gobby::Ipc::RemoteConnection>
open_files(Gobby::Window& wnd,
           const std::vector<std::string>& files,
           bool disable_ipc,
           bool do_join)
{
	std::auto_ptr<Gobby::Ipc::RemoteConnection> conn;

	// Neither do NPC when we want to join a session
	if(!disable_ipc && !do_join)
	{
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

		bool result = do_join ?
			wnd.session_join(false) :
			wnd.session_open(false);

		// First, open a session with default settings
		if(result)
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


	bool new_instance = true;
	Glib::ustring join;

	Glib::OptionGroup opt_group_gobby("gobby", "Gobby options", "Options related directly to gobby");
	Glib::OptionEntry opt_new_instance;
	opt_new_instance.set_short_name('n');
	opt_new_instance.set_long_name("new-instance");
	opt_new_instance.set_description(
		"Do not try to contact already running gobby instance"
	);

	Glib::OptionEntry opt_join;
	opt_join.set_short_name('j');
	opt_join.set_long_name("join");
	opt_join.set_arg_description("HOST:PORT");
	opt_join.set_description("Join a session on the given host");

	opt_group_gobby.add_entry(opt_new_instance, new_instance);
	opt_group_gobby.add_entry(opt_join, join);

	Glib::OptionContext opt_ctx("[file1] [file2] [...]");
	opt_ctx.set_help_enabled(true);
	opt_ctx.set_ignore_unknown_options(false);

	opt_ctx.set_main_group(opt_group_gobby);

	// I would rather like to have Gtk::Main on the stack, but I see
	// no other chance to catch exceptions from the command line option
	// parsing. armin.
	std::auto_ptr<Gtk::Main> kit;

	try
	{
		kit.reset(new Gtk::Main(argc, argv, opt_ctx));
	}
	catch(Glib::Exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	net6::main netkit;

	if(!Glib::thread_supported())
		Glib::thread_init();

#ifdef WITH_GNOME
	gnome_vfs_init();
#endif

	Glib::wrap_register(gtk_source_language_get_type(), &Gtk::SourceLanguage_Class::wrap_new);
	Glib::wrap_register(gtk_source_languages_manager_get_type(), &Gtk::SourceLanguagesManager_Class::wrap_new);
	Glib::wrap_register(gtk_source_buffer_get_type(), &Gtk::SourceBuffer_Class::wrap_new);
	Glib::wrap_register(gtk_source_view_get_type(), &Gtk::SourceView_Class::wrap_new);

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

	// Set join parameters if we want to join a session
	if(!join.empty())
	{
		Glib::ustring::size_type pos = join.rfind(':');
		if(pos == std::string::npos) pos = join.length();

		Gobby::Config::ParentEntry& entry =
			config.get_root()["session"];

		entry.set_value("join_host", join.substr(0, pos));
		if(pos < join.length())
		{
			entry.set_value(
				"join_port",
				std::strtoul(
					join.substr(pos + 1).c_str(),
					NULL,
					10
				)
			);
		}
	}

	// Create window
	Gobby::Window wnd(icon_mgr, config);

	// Open files passed by command line. Need to keep rem_conn,
	// otherwise the connection would be dropped through
	// Gobby::Ipc::RemoteConnection's destructor.
	std::auto_ptr<Gobby::Ipc::RemoteConnection> rem_conn;
	if(!files.empty() )
		rem_conn = open_files(wnd, files, new_instance, !join.empty());
	else
	{
		wnd.show();

		if(!join.empty())
			wnd.session_join(false);
	}

	// Cannot use just kit.run(wnd) since this would show wnd. If we
	// are just sending some data to theother gobby, we do not want
	// the window to be shown.
	wnd.signal_hide().connect(sigc::ptr_fun(&Gtk::Main::quit) );

	kit->run();

#ifdef WITH_GNOME
	//gnome_vfs_shutdown(); // Prints error messages.
	                        // I don't know where they come from...
#endif

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
