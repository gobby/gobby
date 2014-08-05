/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2014 Armin Burgmeier <armin@arbur.net>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
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
