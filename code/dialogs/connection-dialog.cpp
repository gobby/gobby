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

#include "dialogs/connection-dialog.hpp"
#include "util/i18n.hpp"

// TODO: Merge this with entrydialog and passworddialog to a slightly more
// generic entry dialog.

Gobby::ConnectionDialog::ConnectionDialog(
	GtkDialog* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
:
	Gtk::Dialog(cobject)
{
	builder->get_widget("entry", m_entry);
}

std::auto_ptr<Gobby::ConnectionDialog>
Gobby::ConnectionDialog::create(Gtk::Window& parent)
{
	Glib::RefPtr<Gtk::Builder> builder =
		Gtk::Builder::create_from_resource(
			"/de/0x539/gobby/ui/connection-dialog.ui");

	ConnectionDialog* dialog;
	builder->get_widget_derived("ConnectionDialog", dialog);
	dialog->set_transient_for(parent);
	return std::auto_ptr<ConnectionDialog>(dialog);
}

Glib::ustring Gobby::ConnectionDialog::get_host_name() const
{
	return m_entry->get_text();
}

void Gobby::ConnectionDialog::on_show()
{
	Gtk::Dialog::on_show();

	// We can't do this in the constructor, because the buttons are added
	// by the caller after the widget has been constructed.
	set_default_response(Gtk::RESPONSE_ACCEPT);

	m_entry->select_region(0, m_entry->get_text().length());
	m_entry->grab_focus();
}
