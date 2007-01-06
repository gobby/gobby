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

#include <gdk/gdkkeysyms.h>
#include "defaultdialog.hpp"

Gobby::DefaultDialog::DefaultDialog(const Glib::ustring& title,
                                    Gtk::Window& parent,
                                    bool modal,
                                    bool use_separator)
 : Gtk::Dialog(title, parent, modal, use_separator)
{
	add_events(Gdk::KEY_PRESS_MASK);
}

Gobby::DefaultDialog::DefaultDialog(const Glib::ustring& title,
                                    bool modal,
                                    bool use_separator)
 : Gtk::Dialog(title, modal, use_separator)
{
	add_events(Gdk::KEY_PRESS_MASK);
}

Gobby::DefaultDialog::DefaultDialog()
 : Gtk::Dialog()
{
	add_events(Gdk::KEY_PRESS_MASK);
}

bool Gobby::DefaultDialog::on_key_press_event(GdkEventKey* event)
{
	switch(event->keyval)
	{
	case GDK_Return:
		response(Gtk::RESPONSE_OK);
		return true;
	case GDK_Escape:
		response(Gtk::RESPONSE_CANCEL);
		return true;
	default:
		return Dialog::on_key_press_event(event);
	}
}
