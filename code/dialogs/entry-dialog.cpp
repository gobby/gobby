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

#include "dialogs/entry-dialog.hpp"

Gobby::EntryDialog::EntryDialog(Gtk::Window& parent,
                                const Glib::ustring& title,
                                const Glib::ustring& intro_text):
	Gtk::Dialog(title, parent), m_box(false, 6),
	m_intro_label(intro_text, Gtk::ALIGN_RIGHT, Gtk::ALIGN_CENTER, true)
{
	m_intro_label.set_mnemonic_widget(m_entry);
	m_box.pack_start(m_intro_label, Gtk::PACK_EXPAND_WIDGET);
	m_intro_label.show();

	m_entry.set_activates_default(true);
	m_box.pack_start(m_entry, Gtk::PACK_EXPAND_WIDGET);
	m_entry.show();

	m_box.show();

	get_vbox()->set_spacing(6);
	get_vbox()->pack_start(m_box, Gtk::PACK_EXPAND_WIDGET);

	set_resizable(false);
	set_border_width(12);
}

Glib::ustring Gobby::EntryDialog::get_text() const
{
	return m_entry.get_text();
}

void Gobby::EntryDialog::set_text(const Glib::ustring& text)
{
	m_entry.set_text(text);
}

void Gobby::EntryDialog::on_show()
{
	Gtk::Dialog::on_show();

	// We can't do this in the constructor, because the buttons are added
	// by the caller after the widget has been constructed.
	set_default_response(Gtk::RESPONSE_ACCEPT);

	m_entry.select_region(0, m_entry.get_text().length());
	m_entry.grab_focus();
}
