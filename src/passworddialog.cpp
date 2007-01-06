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

#include <gtkmm/stock.h>
#include <gtkmm/messagedialog.h>
#include "common.hpp"
#include "passworddialog.hpp"

Gobby::PasswordDialog::PasswordDialog(Gtk::Window& parent,
                                      const Glib::ustring& title):
	Gtk::Dialog(title, parent, true, true), m_table(3, 3),
	m_icon(Gtk::Stock::DIALOG_AUTHENTICATION, Gtk::ICON_SIZE_DIALOG),
	m_lbl_password("Password:", Gtk::ALIGN_RIGHT),
	m_lbl_conf_password("Confirm password:", Gtk::ALIGN_RIGHT)
{
	m_ent_password.set_visibility(false);
	m_ent_conf_password.set_visibility(false);
	m_info.set_line_wrap(true);

	m_ent_password.set_activates_default(true);
	m_ent_conf_password.set_activates_default(true);

	m_table.set_spacings(5);
	m_table.attach(m_icon, 0, 1, 0, 3, Gtk::SHRINK, Gtk::SHRINK);
	m_table.attach(m_info, 1, 3, 0, 1, Gtk::SHRINK, Gtk::SHRINK);

	m_table.attach(m_lbl_password, 1, 2, 1, 2,
		Gtk::SHRINK | Gtk::FILL, Gtk::SHRINK);
	m_table.attach(m_lbl_conf_password, 1, 2, 2, 3,
		Gtk::SHRINK | Gtk::FILL, Gtk::SHRINK);
	m_table.attach(m_ent_password, 2, 3, 1, 2,
		Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK);
	m_table.attach(m_ent_conf_password, 2, 3, 2, 3,
		Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK);

	get_vbox()->add(m_table);

	add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
	set_default_response(Gtk::RESPONSE_OK);

	show_all();

	// No info at startup
	m_info.hide();

	m_ent_password.signal_changed().connect(
		sigc::mem_fun(*this, &PasswordDialog::on_password_changed)
	);

	m_ent_conf_password.signal_changed().connect(
		sigc::mem_fun(*this, &PasswordDialog::on_password_changed)
	);

	set_response_sensitive(Gtk::RESPONSE_OK, false);
	
	set_resizable(false);
	set_border_width(10);
}

void Gobby::PasswordDialog::set_info(const Glib::ustring& info)
{
	m_info.set_text(info);

	// Show/hide info widget
	if(info.empty() )
		m_info.hide();
	else
		m_info.show();
}

Glib::ustring Gobby::PasswordDialog::get_password() const
{
	return m_ent_password.get_text();
}

void Gobby::PasswordDialog::on_password_changed()
{
	if(m_ent_password.get_text().empty() )
		set_response_sensitive(Gtk::RESPONSE_OK, false);
	else if(m_ent_password.get_text() != m_ent_conf_password.get_text() )
		set_response_sensitive(Gtk::RESPONSE_OK, false);
	else
		set_response_sensitive(Gtk::RESPONSE_OK, true);
}
