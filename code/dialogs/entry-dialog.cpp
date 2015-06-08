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

#include "dialogs/entry-dialog.hpp"

Gobby::EntryDialog::EntryDialog(GtkDialog* cobject,
                                const Glib::RefPtr<Gtk::Builder>& builder):
	Gtk::Dialog(cobject)
{
	builder->get_widget("intro-label", m_intro_label);
	builder->get_widget("entry", m_entry);
}

std::auto_ptr<Gobby::EntryDialog>
Gobby::EntryDialog::create(Gtk::Window& parent,
                           const Glib::ustring& title,
                           const Glib::ustring& intro_text)
{
	Glib::RefPtr<Gtk::Builder> builder =
		Gtk::Builder::create_from_resource(
			"/de/0x539/gobby/ui/entry-dialog.ui");

	EntryDialog* dialog = NULL;
	builder->get_widget_derived("EntryDialog", dialog);

	dialog->set_transient_for(parent);
	dialog->set_title(title);
	dialog->m_intro_label->set_text_with_mnemonic(intro_text);

	return std::auto_ptr<EntryDialog>(dialog);
}

Glib::ustring Gobby::EntryDialog::get_text() const
{
	return m_entry->get_text();
}

void Gobby::EntryDialog::set_text(const Glib::ustring& text)
{
	m_entry->set_text(text);
}

void Gobby::EntryDialog::on_show()
{
	Gtk::Dialog::on_show();

	// We can't do this in the constructor, because the buttons are added
	// by the caller after the widget has been constructed.
	set_default_response(Gtk::RESPONSE_ACCEPT);

	m_entry->select_region(0, m_entry->get_text().length());
	m_entry->grab_focus();
}
