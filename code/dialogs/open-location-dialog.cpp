/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2015 Armin Burgmeier <armin@arbur.net>
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

#include "features.hpp"

Gobby::OpenLocationDialog::OpenLocationDialog(
	GtkDialog* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
:
	Gtk::Dialog(cobject),
	m_combo(builder, "location-combo", config_filename("recent_uris"), 8)
{
	m_combo.get_entry()->set_activates_default(true);

	m_combo.get_entry()->signal_changed().connect(
		sigc::mem_fun(*this, &OpenLocationDialog::on_entry_changed));
}

std::unique_ptr<Gobby::OpenLocationDialog>
Gobby::OpenLocationDialog::create(Gtk::Window& parent)
{
	Glib::RefPtr<Gtk::Builder> builder =
		Gtk::Builder::create_from_resource(
			"/de/0x539/gobby/ui/open-location-dialog.ui");

	OpenLocationDialog* dialog;
	builder->get_widget_derived("OpenLocationDialog", dialog);
	dialog->set_transient_for(parent);
	return std::unique_ptr<OpenLocationDialog>(dialog);
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
