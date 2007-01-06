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
#include "window.hpp"

int main(int argc, char* argv[]) try
{
	Gtk::Main kit(argc, argv);
	Gobby::Window wnd;
	wnd.show_all();
	kit.run(wnd);
	return 0;
}
catch(Glib::Exception& e)
{
	// TODO: Gtk::MessageDialog
	std::cerr << e.what() << std::endl;
}
catch(std::exception& e)
{
	std::cerr << e.what() << std::endl;
}
