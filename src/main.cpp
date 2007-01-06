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
#include "icon.hpp"
#include "config.hpp"
#include "window.hpp"
#include "features.hpp"

void handle_exception(const Glib::ustring& message)
{
	Gtk::MessageDialog dlg("Unhandled exception: " + message, false,
		Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
	dlg.run();
	std::cerr << "Unhandled exception: " << message << std::endl;
}

int main(int argc, char* argv[]) try
{
	setlocale(LC_ALL, "");
	net6::gettext_package gobby_package(GETTEXT_PACKAGE, LOCALE_DIR);
	Gobby::init_gettext(gobby_package);

	Gtk::Main kit(argc, argv);
	Glib::thread_init();

	Gobby::IconManager icon_mgr;

	// Set default icon
	std::list<Glib::RefPtr<Gdk::Pixbuf> > icon_list;
	icon_list.push_back(icon_mgr.gobby);
	Gtk::Window::set_default_icon_list(icon_list);

	// Read the configuration
	Gobby::Config config(Glib::get_home_dir() + "/.gobby/config.xml");

	Gobby::Window wnd(icon_mgr, config);
	wnd.show_all();
	kit.run(wnd);
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

