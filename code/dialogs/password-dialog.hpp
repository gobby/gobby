/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2011 Armin Burgmeier <armin@arbur.net>
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

#ifndef _GOBBY_PASSWORDDIALOG_HPP_
#define _GOBBY_PASSWORDDIALOG_HPP_

#include <gtkmm/dialog.h>
#include <gtkmm/label.h>
#include <gtkmm/entry.h>
#include <gtkmm/box.h>
#include <gtkmm/image.h>

namespace Gobby
{

class PasswordDialog: public Gtk::Dialog
{
public:
	PasswordDialog(Gtk::Window& parent,
	               const Glib::ustring& remote_id,
	               unsigned int retry_counter);

	Glib::ustring get_password() const;

protected:
	virtual void on_show();

	Gtk::HBox m_box;
	Gtk::VBox m_rightbox;
	Gtk::HBox m_promptbox;
        Gtk::Image m_image;
	Gtk::Label m_intro_label;
	Gtk::Label m_prompt_label;
	Gtk::Entry m_entry;
};

}

#endif // _GOBBY_PASSWORDDIALOG_HPP_

