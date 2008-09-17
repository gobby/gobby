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

#include "features.hpp"
#include "window.hpp"

#include "core/iconmanager.hpp"
#include "util/config.hpp"

#include <gtkmm/main.h>
#include <gtkmm/messagedialog.h>
#include <giomm/init.h>
#include <glibmm/optionentry.h>
#include <glibmm/optiongroup.h>
#include <glibmm/optioncontext.h>

#include <libintl.h> // bindtextdomain
#include <iostream>

namespace
{
	void handle_exception(const Glib::ustring& message)
	{
		Gtk::MessageDialog dlg("Unhandled exception", false,
			Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
		dlg.set_secondary_text(message);
		dlg.run();

		std::cerr << "Unhandled exception: " << message << std::endl;
	}
}

int main(int argc, char* argv[]) try
{
	g_thread_init(NULL);
	gnutls_global_init();
	Gio::init();

	setlocale(LC_ALL, "");
	bindtextdomain(GETTEXT_PACKAGE, GOBBY_LOCALEDIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");

	bool new_instance = false;
	Glib::ustring join;

	Glib::OptionContext opt_ctx;
	opt_ctx.set_help_enabled(true);
	opt_ctx.set_ignore_unknown_options(false);

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

	Gobby::IconManager icon_manager;

	// Set default icon
	std::list<Glib::RefPtr<Gdk::Pixbuf> > icon_list;
	icon_list.push_back(icon_manager.gobby);
	Gtk::Window::set_default_icon_list(icon_list);

	// Read the configuration
	Gobby::Config config(Glib::get_home_dir() + "/.gobby/config.xml");

	// Create window
	Gobby::Window wnd(icon_manager, config);
	wnd.show();

	// Cannot use just kit.run(wnd) since this would show wnd. If we
	// are just sending some data to theother gobby, we do not want
	// the window to be shown.
	wnd.signal_hide().connect(sigc::ptr_fun(&Gtk::Main::quit) );
	kit->run();

	//gnutls_global_deinit();
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
