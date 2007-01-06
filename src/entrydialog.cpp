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

#include "entrydialog.hpp"

Gobby::EntryDialog::EntryDialog(Gtk::Window& parent, const Glib::ustring& title, const Glib::ustring& label) 
 : Gtk::Dialog(title, parent, true, true),
   m_label(label),
   m_box(false, 5)
{
	m_box.pack_start(m_label);
	m_box.pack_start(m_entry);

	get_vbox()->set_spacing(5);
	get_vbox()->pack_start(m_box);

	add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);

	show_all();
	set_border_width(10);
	set_resizable(false);
}

Gobby::EntryDialog::~EntryDialog()
{
}

Glib::ustring Gobby::EntryDialog::get_text() const
{
	return m_entry.get_text();
}

void Gobby::EntryDialog::set_text(const Glib::ustring& text)
{
	m_entry.set_text(text);
}
