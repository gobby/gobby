/* gobby - A GTKmm driven libobby client
 * Copyright (C) 2005 0x539 dev group
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

#include <gtkmm/stock.h>
#include "common.hpp"
#include "chat.hpp"

Gobby::Chat::Chat()
 : Gtk::VBox(), m_img_btn(Gtk::Stock::JUMP_TO, Gtk::ICON_SIZE_BUTTON)
{
	m_btn_chat.set_label(_("Send"));
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

void Gobby::Chat::obby_start()
{
	m_log_chat.clear();
	m_ent_chat.set_sensitive(true);
	m_btn_chat.set_sensitive(true);

	set_sensitive(true);
}

void Gobby::Chat::obby_end()
{
	m_ent_chat.clear_history();
	m_ent_chat.set_sensitive(false);
	m_btn_chat.set_sensitive(false);
}

void Gobby::Chat::obby_user_join(obby::user& user)
{
	m_log_chat.log(user.get_name() + " has joined", "blue");
}

void Gobby::Chat::obby_user_part(obby::user& user)
{
	m_log_chat.log(user.get_name() + " has left", "blue");
}

void Gobby::Chat::obby_document_insert(obby::document& document)
{
}

void Gobby::Chat::obby_document_remove(obby::document& document)
{
}

void Gobby::Chat::obby_message(obby::user& user, const Glib::ustring& message)
{
	m_log_chat.log("<" + user.get_name() + "> " + message, "black");
}

void Gobby::Chat::obby_server_message(const Glib::ustring& message)
{
	m_log_chat.log(message, "forest green");
}

void Gobby::Chat::on_chat()
{
	Glib::ustring message = m_ent_chat.get_text();
	if(message.empty() ) return;
	m_ent_chat.set_text("");

	// Send each line separately
	Glib::ustring::size_type prev = 0, pos = 0;
	while( (pos = message.find('\n', pos)) != Glib::ustring::npos)
	{
		m_signal_chat.emit(message.substr(prev, pos - prev) );
		prev = ++pos;
	}
	m_signal_chat.emit(message.substr(prev) );
}
