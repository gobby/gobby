/* gobby - A GTKmm driven libobby client
 * Copyright (C) 2005 0x539 dev group
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <gtkmm/stock.h>
#include "chat.hpp"

Gobby::Chat::Chat()
 : Gtk::VBox(), m_img_btn(Gtk::Stock::JUMP_TO, Gtk::ICON_SIZE_BUTTON)
{
	m_btn_chat.set_label("Send");
	m_btn_chat.set_image(m_img_btn);

	m_btn_chat.set_size_request(100, -1);
	m_btn_chat.signal_clicked().connect(
		sigc::mem_fun(*this, &Chat::on_chat) );
	m_ent_chat.signal_activate().connect(
		sigc::mem_fun(*this, &Chat::on_chat) );

	m_wnd_chat.add(m_log_chat);
	m_wnd_chat.set_shadow_type(Gtk::SHADOW_IN);
	m_wnd_chat.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

	m_box_chat.pack_start(m_ent_chat, Gtk::PACK_EXPAND_WIDGET);
	m_box_chat.pack_start(m_btn_chat, Gtk::PACK_SHRINK);

	m_box_chat.set_spacing(5);

	pack_start(m_wnd_chat, Gtk::PACK_EXPAND_WIDGET);
	pack_start(m_box_chat, Gtk::PACK_SHRINK);

	set_spacing(5);
	set_sensitive(false);
}

Gobby::Chat::~Chat()
{
}

Gobby::Chat::signal_chat_type Gobby::Chat::chat_event() const
{
	return m_signal_chat;
}

void Gobby::Chat::on_chat()
{
	Glib::ustring message = m_ent_chat.get_text();
	if(message.empty() ) return;
	m_ent_chat.set_text("");

	m_signal_chat.emit(message);
}
