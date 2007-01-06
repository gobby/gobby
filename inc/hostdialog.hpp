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

#ifndef _GOBBY_HOSTDIALOG_HPP_
#define _GOBBY_HOSTDIALOG_HPP_

#include <gtkmm/table.h>
#include <gtkmm/label.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/entry.h>
#include <gtkmm/colorbutton.h>
#include "colorsel.hpp"
#include "config.hpp"
#include "fileentry.hpp"

namespace Gobby
{

class HostDialog: public Gtk::Dialog
{
public:
	HostDialog(Gtk::Window& parent, Config& config);
	virtual ~HostDialog();

	unsigned int get_port() const;
	Glib::ustring get_name() const;
	Gdk::Color get_color() const;
	Glib::ustring get_password() const;
	Glib::ustring get_session() const;

	void set_port(unsigned int port);
	void set_name(const Glib::ustring& name);
	void set_color(const Gdk::Color& color);
	void set_password(const Glib::ustring& password);
	void set_session(const Glib::ustring& session);

protected:
	virtual void on_response(int response_id);

	Config& m_config;

	Gtk::Table m_table;
	Gtk::Label m_lbl_port;
	Gtk::Label m_lbl_name;
	Gtk::Label m_lbl_color;
	Gtk::Label m_lbl_password;
	Gtk::Label m_lbl_session;

	Gtk::SpinButton m_ent_port;
	Gtk::Entry m_ent_name;
	ColorButton m_btn_color;
	Gtk::Entry m_ent_password;
	FileEntry m_ent_session;
};

}

#endif // _GOBBY_HOSTDIALOG_HPP_
