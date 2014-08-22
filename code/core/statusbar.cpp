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

#include "core/statusbar.hpp"
#include "util/gtk-compat.hpp"
#include "util/i18n.hpp"

#include <glibmm/main.h>
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

	void dispose_dialog(Gtk::MessageDialog* dialog)
	{
		delete dialog;
	}
}

class Gobby::StatusBar::Message
{
public:
	Message(Gtk::Widget* widget,
	        const Glib::ustring& simple,
	        const Glib::ustring& detail,
		sigc::connection timeout_conn = sigc::connection()):
		m_widget(widget), m_timeout_conn(timeout_conn),
		m_simple_desc(simple), m_detail_desc(detail)
	{
	}

	~Message()
	{
		m_timeout_conn.disconnect();
	}

	void show_dialog() const
	{
		Gtk::MessageDialog* dialog = new Gtk::MessageDialog(
			m_simple_desc,
			false,
			Gtk::MESSAGE_ERROR,
			Gtk::BUTTONS_NONE,
			false);

		dialog->add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CLOSE);

		dialog->set_secondary_text(m_detail_desc, true);
		dialog->signal_response().connect(
			sigc::hide(
				sigc::bind(
					sigc::ptr_fun(dispose_dialog),
					dialog)));
		dialog->show();
	}

	bool is_error() { return !m_detail_desc.empty(); }

	const Glib::ustring& get_simple_text() const { return m_simple_desc; }
	const Glib::ustring& get_detail_text() const { return m_detail_desc; }

	Gtk::Widget* widget() const { return m_widget; }

protected:
	Gtk::Widget* m_widget;
	sigc::connection m_timeout_conn;
	Glib::ustring m_simple_desc;
	Glib::ustring m_detail_desc;
};

Gobby::StatusBar::StatusBar(Gtk::Window& window,
                            const Folder& folder,
                            const Preferences& preferences):
	Gtk::HBox(false, 2), m_folder(folder), m_preferences(preferences),
	m_visible_messages(0), m_current_view(NULL), m_position_context_id(0)
{
	pack_end(m_bar_position, Gtk::PACK_SHRINK);
	m_bar_position.set_size_request(200, -1);
	m_bar_position.show();

	window.signal_window_state_event().connect(
		sigc::mem_fun(*this, &StatusBar::on_window_state_event));

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
Gobby::StatusBar::add_message(Gobby::StatusBar::MessageType type,
                              const Glib::ustring& message,
                              const Glib::ustring& dialog_message,
                              unsigned int timeout)
{
	if(m_visible_messages >= 12)
	{
		for(MessageHandle iter = m_list.begin();
		    iter != m_list.end();
		    ++iter)
		{
			if(*iter)
			{
				if((*iter)->is_error())
					remove_message(iter);
				else
					// only hide message because whoever
					// installed it is expecting to be
					// able to call remove_message on it
					hide_message(iter);
				break;
			}
		}
	}

	Gtk::HBox* bar = Gtk::manage(new Gtk::HBox);

	Gtk::Image* image = Gtk::manage(new Gtk::Image(
		message_type_to_stock_id(type), Gtk::ICON_SIZE_MENU));
	bar->pack_start(*image, Gtk::PACK_SHRINK);
	image->show();

	Gtk::Label* label = Gtk::manage(
		new Gtk::Label(message, GtkCompat::ALIGN_LEFT));
	label->set_ellipsize(Pango::ELLIPSIZE_END);
	bar->pack_start(*label, Gtk::PACK_EXPAND_WIDGET);
	label->show();

	GtkShadowType shadow_type;
	gtk_widget_style_get(GTK_WIDGET(m_bar_position.gobj()),
	                     "shadow-type", &shadow_type, NULL);
	Gtk::Frame* frame = Gtk::manage(new Gtk::Frame);

	m_list.push_back(0);
	Gobby::StatusBar::MessageHandle iter(--m_list.end());
	sigc::connection timeout_conn;
	if(timeout)
	{
		timeout_conn = Glib::signal_timeout().connect_seconds(
			sigc::bind(
				sigc::bind_return(
					sigc::mem_fun(
						*this,
						&StatusBar::remove_message),
					false),
				iter),
			timeout);
	}
	*iter = new Message(frame, message, dialog_message, timeout_conn);
	++m_visible_messages;

	if(dialog_message.empty())
	{
		frame->add(*bar);
	}
	else
	{
		Gtk::EventBox *eventbox = Gtk::manage(new Gtk::EventBox);
		frame->add(*eventbox);
		eventbox->add(*bar);
		eventbox->signal_button_press_event().connect(
			sigc::bind_return(sigc::bind(
				sigc::mem_fun(
					*this,
					&StatusBar::on_message_clicked),
				iter), false));

		eventbox->show();
	}

	frame->set_shadow_type(static_cast<Gtk::ShadowType>(shadow_type));
	bar->show();

	pack_start(*frame, Gtk::PACK_EXPAND_WIDGET);
	reorder_child(*frame, 0);

	frame->show();

	return iter;
}

Gobby::StatusBar::MessageHandle
Gobby::StatusBar::add_info_message(const Glib::ustring& message,
                                   unsigned int timeout)
{
	MessageHandle handle =
		Gobby::StatusBar::add_message(INFO, message, "", timeout);
	// Caller is not allowed to hold on to handles to messages that we are
	// going to delete anyway.
	if(timeout)
		return invalid_handle();
	else
		return handle;
}

void
Gobby::StatusBar::add_error_message(const Glib::ustring& brief_desc,
                                    const Glib::ustring& detailed_desc,
                                    unsigned int timeout)
{
	MessageHandle next;
	for(MessageHandle iter = m_list.begin(); iter != m_list.end(); iter = next)
	{
		next = iter;
		++next;

		if(*iter && (*iter)->is_error())
		{
			if( (*iter)->get_simple_text() == brief_desc)
			{
				remove_message(iter);
			}
		}
	}

	Gobby::StatusBar::add_message(ERROR,
	                              brief_desc,
	                              detailed_desc,
	                              timeout);
}

void Gobby::StatusBar::remove_message(const MessageHandle& handle)
{
	hide_message(handle);
	m_list.erase(handle);
}

void Gobby::StatusBar::hide_message(const MessageHandle& handle)
{
	if(*handle != 0)
	{
		g_assert(m_visible_messages > 0);
		--m_visible_messages;
		remove(*(*handle)->widget());
		delete *handle;
		*handle = 0;
	}
}

Gobby::StatusBar::MessageHandle Gobby::StatusBar::invalid_handle()
{
	return m_list.end();
}

void Gobby::StatusBar::on_message_clicked(GdkEventButton* button,
                                          const MessageHandle& handle)
{
	if(button->button == 1)
		(*handle)->show_dialog();

	remove_message(handle);
}

void Gobby::StatusBar::on_document_removed(SessionView& view)
{
	if(m_current_view == &view)
	{
		GtkTextBuffer* buffer = GTK_TEXT_BUFFER(
			m_current_view->get_text_buffer());

		g_signal_handler_disconnect(buffer, m_mark_set_handler);
		g_signal_handler_disconnect(buffer, m_changed_handler);
		g_signal_handler_disconnect(m_current_view->get_text_view(),
		                            m_toverwrite_handler);

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
		g_signal_handler_disconnect(m_current_view->get_text_view(),
		                            m_toverwrite_handler);
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

		m_toverwrite_handler = g_signal_connect_after(
			G_OBJECT(m_current_view->get_text_view()),
			"notify::overwrite",
			G_CALLBACK(on_toggled_overwrite_static), this);
	}

	// Initial update
	update_pos_display();
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

void Gobby::StatusBar::on_toggled_overwrite()
{
	update_pos_display();
}

void Gobby::StatusBar::on_changed()
{
	update_pos_display();
}

bool Gobby::StatusBar::on_window_state_event(GdkEventWindowState* event)
{
	// In GTK+ 3 the resize grip is handled by the Window,
	// not the status bar
#ifndef USE_GTKMM3
	if (event->new_window_state & GDK_WINDOW_STATE_MAXIMIZED)
		m_bar_position.set_has_resize_grip(false);
	else
		m_bar_position.set_has_resize_grip(true);
#endif

	return true;
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

		gint offset = gtk_text_iter_get_line_offset(&iter);

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

		// TODO: We might want to have a separate widget for the
		// OVR/INS display.
		m_position_context_id = m_bar_position.push(
			Glib::ustring::compose(
				_("Ln %1, Col %2\t%3"),
				gtk_text_iter_get_line(&iter) + 1,
				column + 1,
				gtk_text_view_get_overwrite(GTK_TEXT_VIEW(m_current_view->get_text_view())) ? _("OVR") : _("INS")
			)
		);
	}
}
