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

#include "dialogs/goto-dialog.hpp"
#include "util/i18n.hpp"

#include <gtkmm/messagedialog.h>
#include <gtkmm/textbuffer.h>
#include <gtkmm/stock.h>

Gobby::GotoDialog::GotoDialog(Gtk::Window& parent, const Folder& folder):
	Gtk::Dialog(_("Go to line"), parent),
	m_folder(folder),
	m_table(1, 2),
	m_label_line(_("Line _number:"), Gtk::ALIGN_START,
	             Gtk::ALIGN_CENTER, true),
	m_current_view(NULL)
{
	m_label_line.set_mnemonic_widget(m_entry_line);
	m_label_line.show();

	m_entry_line.set_increments(1, 10);
	m_entry_line.set_activates_default(true);
	m_entry_line.show();

	m_table.attach(m_label_line, 0, 1, 0, 1, Gtk::FILL, Gtk::FILL);
	m_table.attach(m_entry_line, 1, 2, 0, 1,
	               Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK);
	m_table.set_spacings(12);
	m_table.show();

	get_vbox()->pack_start(m_table, Gtk::PACK_EXPAND_WIDGET);

	add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CLOSE);
	Gtk::Button* button =
		add_button(_("Go To _Line"), Gtk::RESPONSE_ACCEPT);

	button->set_image(*Gtk::manage(new Gtk::Image(
		Gtk::Stock::JUMP_TO, Gtk::ICON_SIZE_BUTTON)));

	m_folder.signal_document_changed().connect(
		sigc::mem_fun(*this, &GotoDialog::on_document_changed));

	set_default_response(Gtk::RESPONSE_ACCEPT);
	set_border_width(12);
	set_resizable(false);

	// For initial sensitivity:
	on_document_changed(m_folder.get_current_document());
}

Gobby::GotoDialog::~GotoDialog()
{
	on_document_changed(NULL);
}

void Gobby::GotoDialog::on_show()
{
	Gtk::Dialog::on_show();
	m_entry_line.grab_focus();

	if(m_current_view != NULL)
	{
		GtkTextBuffer* buffer = GTK_TEXT_BUFFER(
			m_current_view->get_text_buffer());
		GtkTextIter cursor;
		gtk_text_buffer_get_iter_at_mark(
			buffer, &cursor, gtk_text_buffer_get_insert(buffer));

		m_entry_line.set_value(gtk_text_iter_get_line(&cursor) + 1);
		m_entry_line.select_region(
			0, m_entry_line.get_text().length());
	}
}

void Gobby::GotoDialog::on_response(int id)
{
	if(id == Gtk::RESPONSE_ACCEPT)
	{
		g_assert(m_current_view != NULL);

		int value = m_entry_line.get_value_as_int();
		GtkTextBuffer* buffer = GTK_TEXT_BUFFER(
			m_current_view->get_text_buffer());
		GtkTextIter begin;
		gtk_text_buffer_get_iter_at_line(buffer, &begin, value - 1);
		m_current_view->set_selection(&begin, &begin);
	}
	else if(id == Gtk::RESPONSE_CLOSE)
	{
		hide();
	}

	Gtk::Dialog::on_response(id);
}

void Gobby::GotoDialog::on_document_changed(SessionView* view)
{
	if(m_current_view != NULL)
	{
		GtkTextBuffer* buffer = GTK_TEXT_BUFFER(
			m_current_view->get_text_buffer());
		g_signal_handler_disconnect(buffer, m_changed_handler);
	}

	m_current_view = dynamic_cast<TextSessionView*>(view);
	set_response_sensitive(Gtk::RESPONSE_ACCEPT, m_current_view != NULL);
	m_entry_line.set_sensitive(m_current_view != NULL);

	if(m_current_view != NULL)
	{
		GtkTextBuffer* buffer = GTK_TEXT_BUFFER(
			m_current_view->get_text_buffer());

		m_changed_handler = g_signal_connect_after(
			G_OBJECT(buffer), "changed",
			G_CALLBACK(on_changed_static), this);

		on_changed();
	}
}

void Gobby::GotoDialog::on_changed()
{
	g_assert(m_current_view != NULL);
	GtkTextBuffer* buffer = GTK_TEXT_BUFFER(
		m_current_view->get_text_buffer());

	m_entry_line.set_range(1, gtk_text_buffer_get_line_count(buffer));
}
