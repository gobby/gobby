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

#include "dialogs/goto-dialog.hpp"
#include "util/i18n.hpp"

#include <gtkmm/messagedialog.h>
#include <gtkmm/textbuffer.h>

Gobby::GotoDialog::GotoDialog(GtkDialog* cobject,
                              const Glib::RefPtr<Gtk::Builder>& builder):
	Gtk::Dialog(cobject), m_folder(NULL), m_current_view(NULL)
{
	builder->get_widget("spin-button", m_entry_line);
	m_entry_line->set_increments(1, 10);

	add_button(_("_Close"), Gtk::RESPONSE_CLOSE);
	add_button(_("Go To _Line"), Gtk::RESPONSE_ACCEPT);
	set_default_response(Gtk::RESPONSE_ACCEPT);
}

Gobby::GotoDialog::~GotoDialog()
{
	on_document_changed(NULL);
}

std::unique_ptr<Gobby::GotoDialog>
Gobby::GotoDialog::create(Gtk::Window& parent, const Folder& folder)
{
	Glib::RefPtr<Gtk::Builder> builder =
		Gtk::Builder::create_from_resource(
			"/de/0x539/gobby/ui/goto-dialog.ui");

	GotoDialog* dialog_ptr;
	builder->get_widget_derived("GotoDialog", dialog_ptr);
	std::unique_ptr<GotoDialog> dialog(dialog_ptr);

	dialog->set_transient_for(parent);
	dialog->m_folder = &folder;

	folder.signal_document_changed().connect(
		sigc::mem_fun(*dialog, &GotoDialog::on_document_changed));

	// For initial sensitivity:
	dialog->on_document_changed(folder.get_current_document());
	return dialog;
}

void Gobby::GotoDialog::on_show()
{
	Gtk::Dialog::on_show();
	m_entry_line->grab_focus();

	if(m_current_view != NULL)
	{
		GtkTextBuffer* buffer = GTK_TEXT_BUFFER(
			m_current_view->get_text_buffer());
		GtkTextIter cursor;
		gtk_text_buffer_get_iter_at_mark(
			buffer, &cursor, gtk_text_buffer_get_insert(buffer));

		m_entry_line->set_value(gtk_text_iter_get_line(&cursor) + 1);
		m_entry_line->select_region(
			0, m_entry_line->get_text().length());
	}
}

void Gobby::GotoDialog::on_response(int id)
{
	if(id == Gtk::RESPONSE_ACCEPT)
	{
		g_assert(m_current_view != NULL);

		int value = m_entry_line->get_value_as_int();
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
	m_entry_line->set_sensitive(m_current_view != NULL);

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

	m_entry_line->set_range(1, gtk_text_buffer_get_line_count(buffer));
}
