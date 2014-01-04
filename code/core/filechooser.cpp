/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2014 Armin Burgmeier <armin@arbur.net>
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
 * Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include "core/filechooser.hpp"

#include <glibmm/miscutils.h>
#include <glibmm/convert.h>
#include <gtkmm/stock.h>

Gobby::FileChooser::Dialog::Dialog(Gobby::FileChooser& chooser,
                                   Gtk::Window& parent,
                                   const Glib::ustring& title,
                                   Gtk::FileChooserAction action):
	Gtk::FileChooserDialog(parent, title, action),
	m_chooser(chooser)
{
	// Set defaults depending on action
	switch(action)
	{
	case Gtk::FILE_CHOOSER_ACTION_SAVE:
		add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
		add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_ACCEPT);
		set_do_overwrite_confirmation(true);
		break;
	case Gtk::FILE_CHOOSER_ACTION_OPEN:
		add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
		add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_ACCEPT);
		break;
	default:
		g_assert_not_reached();
		break;
	}

	set_local_only(false);
	set_current_folder_uri(m_chooser.get_current_folder_uri());
}

Gobby::FileChooser::Dialog::~Dialog()
{
	m_chooser.set_current_folder_uri(get_current_folder_uri());
}

Gobby::FileChooser::FileChooser():
	m_current_folder_uri(Glib::filename_to_uri(Glib::get_current_dir()))
{
}

const std::string& Gobby::FileChooser::get_current_folder_uri() const
{
	return m_current_folder_uri;
}

void Gobby::FileChooser::set_current_folder_uri(const std::string& uri)
{
	m_current_folder_uri = uri;
}

