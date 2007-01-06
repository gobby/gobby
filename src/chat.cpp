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

#include <obby/client_buffer.hpp>

#include <gtkmm/stock.h>
#include <obby/format_string.hpp>
#include "common.hpp"
#include "chat.hpp"

namespace
{
	// Checks if name is highlightend in text.
	bool is_highlighted(const Glib::ustring& text,
	                    const Glib::ustring& name)
	{
		Glib::ustring::size_type pos = 0;
		while( (pos = text.find(name, pos)) != Glib::ustring::npos)
		{
			// Check that the found position is not part of another
			// word ('ck' should not be found in 'luck' and such).
			if(pos > 0 && Glib::Unicode::isalnum(text[pos - 1]) )
				{ ++ pos; continue; }

			if(pos + name.length() < text.length() &&
			   Glib::Unicode::isalnum(text[pos + name.length()]))
				{ ++ pos; continue; }

			// Found occurence
			return true;
		}

		return false;
	}

	void each_line(const std::string& text,
	               const sigc::slot<void, const std::string&> func)
	{
		std::string::size_type prev = 0, pos = 0;
		while( (pos = text.find('\n', pos)) != Glib::ustring::npos)
		{
			func(text.substr(prev, pos - prev) );
			prev = ++pos;
		}

		func(text.substr(prev) );
	}
}

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

/*Gobby::Chat::signal_chat_type Gobby::Chat::chat_event() const
{
	return m_signal_chat;
}*/

void Gobby::Chat::obby_start(obby::local_buffer& buf)
{
	m_buffer = &buf;

	m_log_chat.clear();
	m_ent_chat.set_sensitive(true);
	m_btn_chat.set_sensitive(true);

	set_sensitive(true);

	const obby::chat& chat = buf.get_chat();
	chat.message_event().connect(
		sigc::mem_fun(*this, &Chat::on_message) );

	for(obby::chat::message_iterator iter = chat.message_begin();
	    iter != chat.message_end();
	    ++ iter)
		on_message(*iter);
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
	/*if(~user.get_flags() & obby::user::flags::CONNECTED)
		return;

	obby::format_string str(_("%0% has joined") );
	str << user.get_name();
	m_log_chat.log(str.str(), "blue");*/
}

void Gobby::Chat::obby_user_part(const obby::user& user)
{
/*	obby::format_string str(_("%0% has left") );
	str << user.get_name();
	m_log_chat.log(str.str(), "blue");*/
}

void Gobby::Chat::obby_document_insert(obby::local_document_info& document)
{
}

void Gobby::Chat::obby_document_remove(obby::local_document_info& document)
{
}

#if 0
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
#endif

void Gobby::Chat::on_chat()
{
	if(m_buffer == NULL)
		throw std::logic_error("Gobby::Chat::on_chat");

	Glib::ustring message = m_ent_chat.get_text();
	if(message.empty() ) return;
	m_ent_chat.set_text("");

	// Send each line separately
	each_line(
		message,
		sigc::mem_fun(*this, &Gobby::Chat::send_line)
	);
}

void Gobby::Chat::on_message(const obby::chat::message& message)
{
	const obby::chat::user_message* user_message =
		dynamic_cast<const obby::chat::user_message*>(&message);
	const obby::chat::server_message* server_message =
		dynamic_cast<const obby::chat::server_message*>(&message);
	const obby::chat::system_message* system_message =
		dynamic_cast<const obby::chat::system_message*>(&message);

	if(user_message != NULL)
		on_user_message(*user_message);
	else if(server_message != NULL)
		on_server_message(*server_message);
	else if(system_message != NULL)
		on_system_message(*system_message);
	else
		throw std::logic_error("Gobby::Chat::on_message");
}

void Gobby::Chat::on_user_message(const obby::chat::user_message& message)
{
	// Split received message up into lines
	each_line(
		message.get_text(),
		sigc::bind(
			sigc::mem_fun(*this, &Gobby::Chat::recv_user_line),
			sigc::ref(message)
		)
	);
}

void Gobby::Chat::on_server_message(const obby::chat::server_message& message)
{
	// Split received message up into lines
	each_line(
		message.get_text(),
		sigc::bind(
			sigc::mem_fun(*this, &Gobby::Chat::recv_server_line),
			sigc::ref(message)
		)
	);
}

void Gobby::Chat::on_system_message(const obby::chat::system_message& message)
{
	each_line(
		message.get_text(),
		sigc::bind(
			sigc::mem_fun(*this, &Gobby::Chat::recv_system_line),
			sigc::ref(message)
		)
	);
}

void Gobby::Chat::send_line(const std::string& line)
{
	m_buffer->send_message(line);
}

void Gobby::Chat::recv_user_line(const std::string& line,
                                 const obby::chat::user_message& message)
{
	// Check each line for highlighting occurence
	Glib::ustring colour = "black";
	if(is_highlighted(line, m_buffer->get_self().get_name()) )
		colour = "darkred";

	m_log_chat.log(message.repr(), colour, message.get_timestamp() );
}

void Gobby::Chat::recv_server_line(const std::string& line,
                                   const obby::chat::server_message& message)
{
	m_log_chat.log(message.repr(), "forest green", message.get_timestamp());
}

void Gobby::Chat::recv_system_line(const std::string& line,
                                   const obby::chat::system_message& message)
{
	m_log_chat.log(message.repr(), "blue", message.get_timestamp() );
}

