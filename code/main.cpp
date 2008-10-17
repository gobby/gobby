/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008 Armin Burgmeier <armin@arbur.net>
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
#include "util/file.hpp"
#include "util/i18n.hpp"

#include <libinfinity/common/inf-init.h>

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

	const char* _(const char* str)
	{
		return Gobby::_(str);
	}
}

int main(int argc, char* argv[]) try
{
	g_thread_init(NULL);
	Gio::init();

	setlocale(LC_ALL, "");
	bindtextdomain(GETTEXT_PACKAGE, GOBBY_LOCALEDIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");

	bool display_version = false;

	Glib::OptionGroup opt_group_gobby("gobby",
		_("Gobby options"), _("Options related to Gobby"));
	Glib::OptionEntry opt_version;
	opt_version.set_short_name('v');
	opt_version.set_long_name("version");
	opt_version.set_description(
		_("Display version information and exit"));
	opt_group_gobby.add_entry(opt_version, display_version);

	Glib::OptionContext opt_ctx;
	opt_ctx.set_help_enabled(true);
	opt_ctx.set_ignore_unknown_options(false);
	opt_ctx.set_main_group(opt_group_gobby);

	// I would rather like to have Gtk::Main on the stack, but I see
	// no other chance to catch exceptions from the command line option
	// parsing. armin.
	// TODO: Maybe we should parse before initializing GTK+, using
	// Gtk::Main::add_gtk_option_group() with open_default_display set
	// to false.
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

	if(display_version)
	{
		std::cout << "Gobby " << PACKAGE_VERSION << std::endl;
		return EXIT_SUCCESS;
	}

	GError* error = NULL;
	if(!inf_init(&error))
	{
		std::string message = error->message;
		g_error_free(error);
		throw std::runtime_error(message);
	}

	Gobby::IconManager icon_manager;

	// Set default icon
	std::list<Glib::RefPtr<Gdk::Pixbuf> > icon_list;
	icon_list.push_back(icon_manager.gobby);
	Gtk::Window::set_default_icon_list(icon_list);

	// Read the configuration
	Gobby::Config config(
		Gobby::config_filename(GOBBY_CONFIGDIR, "config.xml"));

	// Create window
	Gobby::Window wnd(icon_manager, config);
	wnd.show();

	wnd.signal_hide().connect(sigc::ptr_fun(&Gtk::Main::quit) );
	kit->run();

	//inf_deinit();
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
