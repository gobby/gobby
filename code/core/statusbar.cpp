/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008, 2009 Armin Burgmeier <armin@arbur.net>
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

#include "core/statusbar.hpp"
#include "util/i18n.hpp"

#include <gtkmm/frame.h>
#include <gtkmm/stock.h>

namespace
{
	Gtk::StockID
	message_type_to_stock_id(Gobby::StatusBar::MessageType type)
	{
		switch(type)
		{
		case Gobby::StatusBar::INFO:
			return Gtk::Stock::DIALOG_INFO;
		case Gobby::StatusBar::ERROR:
			return Gtk::Stock::DIALOG_ERROR;
		default:	
			g_assert_not_reached();
		}
	}
}

class Gobby::StatusBar::Message
{
public:
	Message(Gtk::Widget* widget):
		m_widget(widget) {}
	~Message() { m_conn.disconnect(); }

	void set_timeout_connection(sigc::connection timeout_conn)
	{
		m_conn = timeout_conn;
	}

	Gtk::Widget* widget() const { return m_widget; }
	
	Glib::ustring m_simple_desc;
	Glib::ustring m_detail_desc;
protected:
	Gtk::Widget* m_widget;
	sigc::connection m_conn;
};

Gobby::StatusBar::StatusBar(Folder& folder,
                            const Preferences& preferences):
	Gtk::HBox(false, 2), m_folder(folder), m_preferences(preferences),
	m_current_view(NULL), m_position_context_id(0)
{
	pack_end(m_bar_position, Gtk::PACK_SHRINK);
	m_bar_position.set_size_request(200, -1);
	m_bar_position.show();

	m_folder.signal_document_removed().connect(
		sigc::mem_fun(*this, &StatusBar::on_document_removed));
	m_folder.signal_document_changed().connect(
		sigc::mem_fun(*this, &StatusBar::on_document_changed));
	m_preferences.appearance.show_statusbar.signal_changed().connect(
		sigc::mem_fun(*this, &StatusBar::on_view_changed));

	// Initial update
	on_document_changed(m_folder.get_current_document());
	on_view_changed();
}

Gobby::StatusBar::~StatusBar()
{
	on_document_changed(NULL);
}

Gobby::StatusBar::MessageHandle
Gobby::StatusBar::add_message(MessageType type,
                              const Glib::ustring& message,
                              const Glib::ustring& dialog_message)
{
	if (m_list.size() >= 12)
		Gobby::StatusBar::remove_message(m_list.begin());

	Gtk::HBox* bar = Gtk::manage(new Gtk::HBox);

	Gtk::Image* image = Gtk::manage(new Gtk::Image(
		message_type_to_stock_id(type), Gtk::ICON_SIZE_MENU));
	bar->pack_start(*image, Gtk::PACK_SHRINK);
	image->show();

	Gtk::Label* label = Gtk::manage(new Gtk::Label(message,
	                                               Gtk::ALIGN_LEFT));
	label->set_ellipsize(Pango::ELLIPSIZE_END);
	bar->pack_start(*label, Gtk::PACK_EXPAND_WIDGET);
	label->show();

	GtkShadowType shadow_type;
	gtk_widget_style_get(GTK_WIDGET(m_bar_position.gobj()),
	                     "shadow-type", &shadow_type, NULL);
	Gtk::Frame* frame = Gtk::manage(new Gtk::Frame);
	Gtk::EventBox *eventbox = Gtk::manage(new Gtk::EventBox);

	frame->set_shadow_type(static_cast<Gtk::ShadowType>(shadow_type));
	frame->add(*eventbox);
	eventbox->add(*bar);
	bar->show();

	pack_start(*frame, Gtk::PACK_EXPAND_WIDGET);
	reorder_child(*frame, 0);

	m_list.push_back(new Message(frame));
	Gobby::StatusBar::MessageHandle iter(--m_list.end());
	(*iter)->m_simple_desc = message;
	(*iter)->m_detail_desc = dialog_message;

	eventbox->signal_button_press_event().connect(
		sigc::bind_return(
			sigc::bind(
				sigc::mem_fun(
					*this,
					&StatusBar::on_message_clicked),
				iter),
			false));

	eventbox->show();
	frame->show();

	return iter;
}

Gobby::StatusBar::MessageHandle
Gobby::StatusBar::add_info_message(const Glib::ustring& message)
{
	return Gobby::StatusBar::add_message(INFO, message, "");
}

Gobby::StatusBar::MessageHandle
Gobby::StatusBar::add_error_message(const Glib::ustring& brief_desc,
                                    const Glib::ustring& detailed_desc)
{
	return Gobby::StatusBar::add_message(ERROR, brief_desc, detailed_desc);
}

void Gobby::StatusBar::remove_message(const MessageHandle& handle)
{
	remove(*(*handle)->widget());
	delete *handle;
	m_list.erase(handle);
}

Gobby::StatusBar::MessageHandle Gobby::StatusBar::invalid_handle()
{
	return m_list.end();
}

void Gobby::StatusBar::on_message_clicked(GdkEventButton* button,
                                          const MessageHandle& handler)
{
	const Glib::ustring& simple = (*handler)->m_simple_desc;
	const Glib::ustring& detail = (*handler)->m_detail_desc;

	if(!detail.empty())
	{
		if (button->button == 1)
		{
			Gtk::MessageDialog *dialog = new Gtk::MessageDialog(
				simple,
				false,
				Gtk::MESSAGE_ERROR,
				Gtk::BUTTONS_CLOSE,
				false
			);

			dialog->set_secondary_text(detail, false);
			dialog->run();

			delete dialog;
		}

		remove_message(handler);
	}
}

void Gobby::StatusBar::on_document_removed(SessionView& view)
{
	if(m_current_view == &view)
	{
		GtkTextBuffer* buffer = GTK_TEXT_BUFFER(
			m_current_view->get_text_buffer());

		g_signal_handler_disconnect(buffer, m_mark_set_handler);
		g_signal_handler_disconnect(buffer, m_changed_handler);

		m_current_view = NULL;
	}
}

void Gobby::StatusBar::on_document_changed(SessionView* view)
{
	if(m_current_view)
	{
		GtkTextBuffer* buffer = GTK_TEXT_BUFFER(
			m_current_view->get_text_buffer());

		g_signal_handler_disconnect(buffer, m_mark_set_handler);
		g_signal_handler_disconnect(buffer, m_changed_handler);
	}

	m_current_view = dynamic_cast<TextSessionView*>(view);

	if(m_current_view)
	{
		GtkTextBuffer* buffer = GTK_TEXT_BUFFER(
			m_current_view->get_text_buffer());

		m_mark_set_handler = g_signal_connect_after(
			G_OBJECT(buffer), "mark-set",
			G_CALLBACK(on_mark_set_static), this);

		m_changed_handler = g_signal_connect_after(
			G_OBJECT(buffer), "changed",
			G_CALLBACK(on_changed_static), this);

		// Initial update
		update_pos_display();
	}
}

void Gobby::StatusBar::on_view_changed()
{
	if(m_preferences.appearance.show_statusbar) show();
	else hide();
}

void Gobby::StatusBar::on_mark_set(GtkTextMark* mark)
{
	GtkTextBuffer* buffer = GTK_TEXT_BUFFER(
		m_current_view->get_text_buffer());

	if(mark == gtk_text_buffer_get_insert(buffer))
		update_pos_display();
}

void Gobby::StatusBar::on_changed()
{
	update_pos_display();
}

void Gobby::StatusBar::update_pos_display()
{
	if(m_position_context_id)
		m_bar_position.remove_message(m_position_context_id);

	if(m_current_view != NULL)
	{
		GtkTextBuffer* buffer = GTK_TEXT_BUFFER(
			m_current_view->get_text_buffer());
		GtkTextIter iter;

		// TODO: Use TextSessionView::get_cursor_position()?
		gtk_text_buffer_get_iter_at_mark(
			buffer, &iter, gtk_text_buffer_get_insert(buffer));

		guint offset = gtk_text_iter_get_line_offset(&iter);

		unsigned int column = 0;
		const unsigned int tab_width = m_preferences.editor.tab_width;
		for(gtk_text_iter_set_line_offset(&iter, 0);
		    gtk_text_iter_get_line_offset(&iter) < offset;
		    gtk_text_iter_forward_char(&iter))
		{
			if(gtk_text_iter_get_char(&iter) == '\t')
				column += (tab_width - column % tab_width);
			else
				++ column;
		}

		m_position_context_id = m_bar_position.push(
			Glib::ustring::compose(
				_("Ln %1, Col %2"),
				gtk_text_iter_get_line(&iter) + 1,
				column + 1));
	}
}
