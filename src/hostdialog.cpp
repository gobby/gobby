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
#include "hostdialog.hpp"

Gobby::HostDialog::HostDialog(Gtk::Window& parent, Config& config)
 : DefaultDialog(_("Create obby session"), parent, true, true),
   m_config(config),
   m_table(3, 2),
   m_lbl_port(_("Port:"), Gtk::ALIGN_RIGHT),
   m_lbl_name(_("Name:"), Gtk::ALIGN_RIGHT),
   m_lbl_color(_("Colour:"), Gtk::ALIGN_RIGHT),
   m_lbl_password(_("Password:"), Gtk::ALIGN_RIGHT)
{
	m_ent_port.set_range(1024, 65535);
	m_ent_port.set_value(config["create"]["port"].get(6522) );
	m_ent_port.set_increments(1, 256);

	m_ent_password.set_visibility(false);

	// TODO: Share user settings between create and join dialog
	// TODO: Read default color as random one from tom's color map
	Gdk::Color default_color;
	default_color.set_red(0xcccc);
	default_color.set_green(0xcccc);
	default_color.set_blue(0xffff);

	Glib::ustring name =
		config["create"]["name"].get(Glib::get_user_name() );
	Gdk::Color color =
		config["create"]["color"].get(Gdk::Color(default_color) );

	m_ent_name.set_text(name);
	m_btn_color.set_color(color);

	m_table.attach(m_lbl_port, 0, 1, 0, 1,
		Gtk::SHRINK | Gtk::FILL, Gtk::SHRINK);
	m_table.attach(m_lbl_name, 0, 1, 1, 2,
		Gtk::SHRINK | Gtk::FILL, Gtk::SHRINK);
	m_table.attach(m_lbl_color, 0, 1, 2, 3,
		Gtk::SHRINK | Gtk::FILL, Gtk::SHRINK);
	m_table.attach(m_lbl_password, 0, 1, 3, 4,
		Gtk::SHRINK | Gtk::FILL, Gtk::SHRINK);

	m_table.attach(m_ent_port, 1, 2, 0, 1,
		Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK);
	m_table.attach(m_ent_name, 1, 2, 1, 2,
		Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK);
	m_table.attach(m_btn_color, 1, 2, 2, 3,
		Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK);
	m_table.attach(m_ent_password, 1, 2, 3, 4,
		Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK);

	m_table.set_spacings(5);
  
	get_vbox()->set_spacing(5);
	get_vbox()->pack_start(m_table);

	add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);

	show_all();
	set_border_width(10);
	set_resizable(false);
}

Gobby::HostDialog::~HostDialog()
{
}

unsigned int Gobby::HostDialog::get_port() const
{
	return static_cast<unsigned int>(m_ent_port.get_value() );
}

Glib::ustring Gobby::HostDialog::get_name() const
{
	return m_ent_name.get_text();
}

Gdk::Color Gobby::HostDialog::get_color() const
{
	return m_btn_color.get_color();
}

Glib::ustring Gobby::HostDialog::get_password() const
{
	return m_ent_password.get_text();
}

void Gobby::HostDialog::set_port(unsigned int port)
{
	m_ent_port.set_value(static_cast<double>(port) );
}

void Gobby::HostDialog::set_name(const Glib::ustring& name)
{
	m_ent_name.set_text(name);
}

void Gobby::HostDialog::set_color(const Gdk::Color& color)
{
	m_btn_color.set_color(color);
}

void Gobby::HostDialog::set_password(const Glib::ustring& password)
{
	m_ent_password.set_text(password);
}

void Gobby::HostDialog::on_response(int response_id)
{
	if(response_id == Gtk::RESPONSE_OK)
	{
		// Write new values into config
		m_config["create"]["port"].set(get_port() );
		m_config["create"]["name"].set(get_name() );
		m_config["create"]["color"].set(get_color() );
	}

	Gtk::Dialog::on_response(response_id);
}

