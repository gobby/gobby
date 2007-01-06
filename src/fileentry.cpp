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
#include "fileentry.hpp"

Gobby::FileEntry::FileEntry(const Glib::ustring& title):
	Gtk::HBox(), m_dialog(title), m_btn_browse(Gtk::Stock::OPEN)
{
	init();
}

Gobby::FileEntry::FileEntry(Gtk::Window& parent, const Glib::ustring& title):
	Gtk::HBox(), m_dialog(parent, title), m_btn_browse(Gtk::Stock::OPEN)
{
	init();
}

void Gobby::FileEntry::init()
{
	m_dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	m_dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);

	m_btn_browse.signal_clicked().connect(
		sigc::mem_fun(*this, &FileEntry::on_browse) );

	pack_start(m_ent_file, Gtk::PACK_EXPAND_WIDGET);
	pack_start(m_btn_browse, Gtk::PACK_SHRINK);

	set_spacing(5);
}

Glib::ustring Gobby::FileEntry::get_text() const
{
	return m_ent_file.get_text();
}

void Gobby::FileEntry::set_text(const Glib::ustring& title)
{
	m_ent_file.set_text(title);
}

Gtk::FileChooser& Gobby::FileEntry::get_file_chooser()
{
	return m_dialog;
}

const Gtk::FileChooser& Gobby::FileEntry::get_file_chooser() const
{
	return m_dialog;
}

void Gobby::FileEntry::on_browse()
{
	if(m_dialog.run() == Gtk::RESPONSE_OK)
		m_ent_file.set_text(m_dialog.get_filename() );

	m_dialog.hide();
}
