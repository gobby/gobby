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

#ifndef _GOBBY_CHAT_HPP_
#define _GOBBY_CHAT_HPP_

#include <gtkmm/box.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/button.h>
#include <gtkmm/image.h>
#include <obby/user.hpp>
#include <obby/document.hpp>
#include "historyentry.hpp"
#include "logview.hpp"

namespace Gobby
{

class Chat : public Gtk::VBox
{
public:
	typedef sigc::signal<void, const Glib::ustring&> signal_chat_type;

	Chat();
	virtual ~Chat();

	signal_chat_type chat_event() const;

	// Calls from the window
	void obby_start();
	void obby_end();
	void obby_user_join(obby::user& user);
	void obby_user_part(obby::user& user);
	void obby_document_insert(obby::document& document);
	void obby_document_remove(obby::document& document);
	void obby_message(obby::user& user, const Glib::ustring& message);
	void obby_server_message(const Glib::ustring& message);
protected:
	void add_line(obby::user& user, const Glib::ustring& message);
	void on_chat();

	signal_chat_type m_signal_chat;

	Gtk::Image m_img_btn;
	Gtk::HBox m_box_chat;
	Gtk::ScrolledWindow m_wnd_chat;
	LogView m_log_chat;
	HistoryEntry m_ent_chat;
	Gtk::Button m_btn_chat;
};

}
	
#endif // _GOBBY_CHAT_HPP_
