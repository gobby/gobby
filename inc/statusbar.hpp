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

#ifndef _GOBBY_STATUSBAR_HPP_
#define _GOBBY_STATUSBAR_HPP_

#include <glibmm/ustring.h>
#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/frame.h>
#include <gtkmm/statusbar.h>
#include <obby/user.hpp>
#include "document.hpp"
#include "buffer_def.hpp"
#include "header.hpp"
#include "folder.hpp"

namespace Gobby
{

class StatusBar: public Gtk::HBox
{
protected:
	typedef std::list<Gtk::Widget*> MessageList;

public:
	enum MessageType {
		INFO,
		ERROR
	};

	typedef MessageList::iterator MessageHandle;

	StatusBar(const Folder& folder);

	MessageHandle add_message(MessageType type,
	                          const Glib::ustring& message,
	                          unsigned int timeout); // timeout in seconds

	void remove_message(const MessageHandle& handle);

protected:
	MessageList m_list;

	Gtk::Statusbar m_bar_position;
};

}

#endif // _GOBBY_STATUSBAR_HPP_
