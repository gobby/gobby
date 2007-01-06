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
#include <obby/format_string.hpp>
#include "common.hpp"
#include "chat.hpp"

Gobby::Chat::Chat()
 : Gtk::VBox(), m_buffer(NULL),
   m_img_btn(Gtk::Stock::JUMP_TO, Gtk::ICON_SIZE_BUTTON)
{
	m_btn_chat.set_label(_("Send"));
	m_btn_chat.set_image(m_img_btn);

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

void Gobby::Chat::obby_start(obby::local_buffer& buf)
{
	m_buffer = &buf;

	m_log_chat.clear();
	m_ent_chat.set_sensitive(true);
	m_btn_chat.set_sensitive(true);

	set_sensitive(true);
}

void Gobby::Chat::obby_end()
{
	m_buffer = NULL;

	m_ent_chat.clear_history();
	m_ent_chat.set_sensitive(false);
	m_btn_chat.set_sensitive(false);
}

void Gobby::Chat::obby_user_join(const obby::user& user)
{
	obby::format_string str(_("%0% has joined") );
	str << user.get_name();
	m_log_chat.log(str.str(), "blue");
}

void Gobby::Chat::obby_user_part(const obby::user& user)
{
	obby::format_string str(_("%0% has left") );
	str << user.get_name();
	m_log_chat.log(str.str(), "blue");
}

void Gobby::Chat::obby_document_insert(obby::local_document_info& document)
{
}

void Gobby::Chat::obby_document_remove(obby::local_document_info& document)
{
}

void Gobby::Chat::obby_message(const obby::user& user,
                               const Glib::ustring& message)
{
	// Make sure we are not deceived by rogue multi line messages
	Glib::ustring::size_type prev = 0, pos = 0;
	while( (pos = message.find('\n', pos)) != Glib::ustring::npos)
	{
		add_line(user, message.substr(prev, pos - prev) );
		prev = ++pos;
	}
	add_line(user, message.substr(prev));
}

void Gobby::Chat::add_line(const obby::user& user, const Glib::ustring& message)
{
	const obby::user& self = m_buffer->get_self();
	Glib::ustring name = self.get_name();

	// Check if we have to highlight the line becasue the user's nick name
	// was found in the the message.
	Glib::ustring colour = "black";

	// Only check for highlighting if another user wrote this message
	if(&self != &user)
	{
		Glib::ustring::size_type pos = 0;
		while( (message.find(name, pos)) != Glib::ustring::npos)
		{
			// Check that the found position is not part of another
			// word ('ck' should not be found in 'luck' and such).
			if(pos > 0 && Glib::Unicode::isalnum(message[pos - 1]) )
				{ ++ pos; continue; }

			if(pos + name.length() < message.length() &&
			   Glib::Unicode::isalnum(message[pos + name.length()]))
				{ ++ pos; continue; }

			// Found occurence
			colour = "darkred";
			break;
		}
	}

	m_log_chat.log("<" + user.get_name() + "> " + message, colour);
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

