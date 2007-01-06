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

#ifndef _GOBBY_PASSWORDDIALOG_HPP_
#define _GOBBY_PASSWORDDIALOG_HPP_

#include "defaultdialog.hpp"
#include <gtkmm/table.h>
#include <gtkmm/label.h>
#include <gtkmm/image.h>
#include <gtkmm/entry.h>

namespace Gobby
{

class PasswordDialog : public DefaultDialog
{
public:
	PasswordDialog(Gtk::Window& parent, const Glib::ustring& title,
	               bool request);
	~PasswordDialog();

	void set_info(const Glib::ustring& info);
	Glib::ustring get_password() const;
protected:
	virtual void on_response(int response_id);

	bool m_request;

	Gtk::Table m_table;

	Gtk::Image m_icon;
	Gtk::Label m_lbl_password;
	Gtk::Label m_lbl_conf_password;
	Gtk::Label m_info;
	Gtk::Entry m_ent_password;
	Gtk::Entry m_ent_conf_password;
};

}

#endif // _GOBBY_PASSWORDDIALOG_HPP_
