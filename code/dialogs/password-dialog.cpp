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

#include "dialogs/password-dialog.hpp"
#include "util/i18n.hpp"

Gobby::PasswordDialog::PasswordDialog(
	const Glib::RefPtr<Gtk::Builder>& builder,
	const Glib::ustring& remote_id, unsigned int retry_counter)
:
	Gtk::Dialog(GTK_DIALOG(gtk_builder_get_object(
		builder->gobj(), "PasswordDialog")))
{
	Gtk::Label* intro_label;
	builder->get_widget("intro-label", intro_label);
	builder->get_widget("password", m_entry);

	if(retry_counter == 0)
	{
		intro_label->set_text(Glib::ustring::compose(
			_("Connection to host \"%1\" requires a password."),
			remote_id));
	}
	else
	{
		intro_label->set_text(Glib::ustring::compose(
			_("Invalid password for host \"%1\". "
			  "Please try again."),
			remote_id));
	}
}

std::unique_ptr<Gobby::PasswordDialog>
Gobby::PasswordDialog::create(Gtk::Window& parent,
                              const Glib::ustring& remote_id,
                              unsigned int retry_counter)
{
	Glib::RefPtr<Gtk::Builder> builder =
		Gtk::Builder::create_from_resource(
			"/de/0x539/gobby/ui/password-dialog.ui");

	std::unique_ptr<PasswordDialog> dialog(
		new PasswordDialog(builder, remote_id, retry_counter));
	dialog->set_transient_for(parent);
	return dialog;

}

Glib::ustring Gobby::PasswordDialog::get_password() const
{
	return m_entry->get_text();
}

void Gobby::PasswordDialog::on_show()
{
	Gtk::Dialog::on_show();

	// We can't do this in the constructor, because the buttons are added
	// by the caller after the widget has been constructed.
	set_default_response(Gtk::RESPONSE_ACCEPT);

	m_entry->select_region(0, m_entry->get_text().length());
	m_entry->grab_focus();
}
