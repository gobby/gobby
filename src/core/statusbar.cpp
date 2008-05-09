/* gobby - A GTKmm driven libobby client
 * Copyright (C) 2005, 2006 0x539 dev group
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
protected:
	Gtk::Widget* m_widget;
	sigc::connection m_conn;
};

Gobby::StatusBar::StatusBar(const Folder& folder):
	Gtk::HBox(false, 2)
{
	pack_end(m_bar_position, Gtk::PACK_SHRINK);
	m_bar_position.set_size_request(200, -1);
	m_bar_position.show();
}

Gobby::StatusBar::MessageHandle
Gobby::StatusBar::add_message(MessageType type,
                              const Glib::ustring& message,
                              unsigned int timeout)
{
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
	frame->set_shadow_type(static_cast<Gtk::ShadowType>(shadow_type));
	frame->add(*bar);
	bar->show();

	pack_start(*frame, Gtk::PACK_EXPAND_WIDGET);
	reorder_child(*frame, 0);
	frame->show();

	m_list.push_back(new Message(frame));
	MessageHandle iter(m_list.end());
	--iter;

	if(timeout)
	{
		sigc::slot<bool> slot(sigc::bind_return(
			sigc::bind(
				sigc::mem_fun(
					*this,
					&StatusBar::remove_message),
				iter),
			false));
		
		sigc::connection timeout_conn =
			Glib::signal_timeout().connect_seconds(slot, timeout);

		(*iter)->set_timeout_connection(timeout_conn);
	}

	return iter;
}

void Gobby::StatusBar::remove_message(const MessageHandle& handle)
{
	remove(*(*handle)->widget());
	delete *handle;
	m_list.erase(handle);
}
