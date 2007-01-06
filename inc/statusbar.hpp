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

#ifndef _GOBBY_STATUSBAR_HPP_
#define _GOBBY_STATUSBAR_HPP_

#include <glibmm/ustring.h>
#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/frame.h>
#include <gtkmm/separator.h>
#include <obby/user.hpp>
#include <obby/local_document_info.hpp>
#include <obby/local_buffer.hpp>
#include "document.hpp"
#include "folder.hpp"

namespace Gobby
{

class StatusBar : public Gtk::Frame
{
public:
	StatusBar(const Folder& folder);
	virtual ~StatusBar();

	void update_language(Document& document);
	void update_cursor(Document& document);
	void update_from_document(Document& document);

	void update_connection(const Glib::ustring& str);

	// Calls from the window
	void obby_start(obby::local_buffer& buf);
	void obby_end();
	void obby_user_join(const obby::user& user);
	void obby_user_part(const obby::user& user);
	void obby_document_insert(obby::local_document_info& document);
	void obby_document_remove(obby::local_document_info& document);

	virtual void on_show();

protected:
	Gtk::HBox m_box;
	Gtk::Label m_language;
	Gtk::Label m_connection;
	Gtk::Label m_position;

	Gtk::VSeparator m_sep;
};

}

#endif // _GOBBY_STATUSBAR_HPP_
