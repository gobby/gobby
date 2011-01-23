/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2010 Armin Burgmeier <armin@arbur.net>
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

#include "dialogs/open-location-dialog.hpp"

#include "util/file.hpp"
#include "util/i18n.hpp"
#include "util/gtk-compat.hpp"

#include "features.hpp"

Gobby::OpenLocationDialog::OpenLocationDialog(Gtk::Window& parent):
	Gtk::Dialog(_("Open Location"), parent), m_box(false, 6),
	m_label(_("Enter the _location (URI) of the file you would "
	          "like to open:"), GtkCompat::ALIGN_LEFT,
	        Gtk::ALIGN_CENTER, true),
	m_combo(config_filename("recent_uris"), 8)
{
	m_label.set_mnemonic_widget(m_combo);
	m_box.pack_start(m_label, Gtk::PACK_SHRINK);
	m_label.show();

	m_combo.get_entry()->set_activates_default(true);
	m_box.pack_start(m_combo, Gtk::PACK_SHRINK);
	m_combo.show();

	m_combo.get_entry()->signal_changed().connect(
		sigc::mem_fun(*this, &OpenLocationDialog::on_entry_changed));

	m_box.show();

	get_vbox()->set_spacing(6);
	get_vbox()->pack_start(m_box, Gtk::PACK_EXPAND_WIDGET);

	set_resizable(false);
	set_border_width(12);
}

Glib::ustring Gobby::OpenLocationDialog::get_uri() const
{
	return m_combo.get_entry()->get_text();
}

void Gobby::OpenLocationDialog::set_uri(const Glib::ustring& uri)
{
	m_combo.get_entry()->set_text(uri);
}

void Gobby::OpenLocationDialog::on_response(int response_id)
{
	if(response_id == Gtk::RESPONSE_ACCEPT)
		m_combo.commit();

	Gtk::Dialog::on_response(response_id);
}

void Gobby::OpenLocationDialog::on_show()
{
	Gtk::Dialog::on_show();

	// We can't do this in the constructor, because the buttons are added
	// by the caller after the widget has been constructed.
	set_default_response(Gtk::RESPONSE_ACCEPT);
	on_entry_changed();

	m_combo.get_entry()->select_region(
		0, m_combo.get_entry()->get_text().length());
	m_combo.grab_focus();
}

void Gobby::OpenLocationDialog::on_entry_changed()
{
	set_response_sensitive(
		Gtk::RESPONSE_ACCEPT,
		!m_combo.get_entry()->get_text().empty());
}
