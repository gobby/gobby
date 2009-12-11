/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008, 2009 Armin Burgmeier <armin@arbur.net>
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

#include "dialogs/password-dialog.hpp"
#include "util/i18n.hpp"

#include <gtkmm/stock.h>

Gobby::PasswordDialog::PasswordDialog(Gtk::Window& parent,
                                const Glib::ustring& remote_id):
	Gtk::Dialog(_("Password required"), parent), m_box(false, 6),
	m_rightbox(false, 16),
	m_promptbox(false, 8),
	m_image(Gtk::Stock::DIALOG_AUTHENTICATION, Gtk::ICON_SIZE_DIALOG),
	m_intro_label(Glib::ustring::compose(
		_("Connection to host %1 requires a password."), remote_id)),
	m_prompt_label(_("_Password:"),
		Gtk::ALIGN_RIGHT, Gtk::ALIGN_CENTER, true)
{
	m_prompt_label.set_mnemonic_widget(m_entry);
	m_promptbox.pack_start(m_prompt_label, Gtk::PACK_SHRINK);
	m_entry.set_visibility(false);
	m_entry.set_activates_default(true);
	m_promptbox.pack_start(m_entry, Gtk::PACK_EXPAND_WIDGET);
	m_rightbox.pack_start(m_intro_label, Gtk::PACK_SHRINK);
	m_rightbox.pack_start(m_promptbox, Gtk::PACK_SHRINK);
	m_box.pack_start(m_image, Gtk::PACK_SHRINK);
	m_box.pack_start(m_rightbox, Gtk::PACK_EXPAND_WIDGET);

	m_box.show_all();

	get_vbox()->set_spacing(6);
	get_vbox()->pack_start(m_box, Gtk::PACK_EXPAND_WIDGET);

	set_resizable(false);
	set_border_width(12);
}

Glib::ustring Gobby::PasswordDialog::get_password() const
{
	return m_entry.get_text();
}

void Gobby::PasswordDialog::on_show()
{
	Gtk::Dialog::on_show();

	// We can't do this in the constructor, because the buttons are added
	// by the caller after the widget has been constructed.
	set_default_response(Gtk::RESPONSE_ACCEPT);

	m_entry.select_region(0, m_entry.get_text().length());
	m_entry.grab_focus();
}
