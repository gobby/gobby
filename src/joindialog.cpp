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
#include "joindialog.hpp"

Gobby::JoinDialog::JoinDialog(Gtk::Window& parent, Gobby::Config& config)
 : Gtk::Dialog("Join obby session", parent, true, true), m_config(config),
   m_table(4, 2),
   m_lbl_host("Host:", Gtk::ALIGN_RIGHT),
   m_lbl_port("Port:", Gtk::ALIGN_RIGHT),
   m_lbl_name("Name:", Gtk::ALIGN_RIGHT),
   m_lbl_color("Color:", Gtk::ALIGN_RIGHT)
{
	Glib::ustring host =
		config["join"]["host"].get(Glib::ustring("localhost") );
	unsigned int port =
		config["join"]["port"].get(6522);
	Glib::ustring name =
		config["join"]["name"].get(Glib::get_user_name() );
	Gdk::Color color =
		config["join"]["color"].get(Gdk::Color("black") );

	m_ent_host.set_text(host);

	m_ent_port.set_range(1024, 65535);
	m_ent_port.set_value(port);
	m_ent_port.set_increments(1, 256);

	m_ent_name.set_text(name);
	m_btn_color.set_color(color);

	m_table.attach(m_lbl_host, 0, 1, 0, 1,
		Gtk::SHRINK | Gtk::FILL, Gtk::SHRINK);
	m_table.attach(m_lbl_port, 0, 1, 1, 2,
		Gtk::SHRINK | Gtk::FILL, Gtk::SHRINK);
	m_table.attach(m_lbl_name, 0, 1, 2, 3,
		Gtk::SHRINK | Gtk::FILL, Gtk::SHRINK);
	m_table.attach(m_lbl_color, 0, 1, 3, 4,
		Gtk::SHRINK | Gtk::FILL, Gtk::SHRINK);

	m_table.attach(m_ent_host, 1, 2, 0, 1,
		Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK);
	m_table.attach(m_ent_port, 1, 2, 1, 2,
		Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK);
	m_table.attach(m_ent_name, 1, 2, 2, 3,
		Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK);
	m_table.attach(m_btn_color, 1, 2, 3, 4,
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

Gobby::JoinDialog::~JoinDialog()
{
}

Glib::ustring Gobby::JoinDialog::get_host() const
{
	return m_ent_host.get_text();
}

unsigned int Gobby::JoinDialog::get_port() const
{
	return static_cast<unsigned int>(m_ent_port.get_value() );
}

Glib::ustring Gobby::JoinDialog::get_name() const
{
	return m_ent_name.get_text();
}

Gdk::Color Gobby::JoinDialog::get_color() const
{
	return m_btn_color.get_color();
}

void Gobby::JoinDialog::set_host(const Glib::ustring& host)
{
	m_ent_host.set_text(host);
}

void Gobby::JoinDialog::set_port(unsigned int port)
{
	m_ent_port.set_value(static_cast<double>(port) );
}

void Gobby::JoinDialog::set_name(const Glib::ustring& name)
{
	m_ent_name.set_text(name);
}

void Gobby::JoinDialog::set_color(const Gdk::Color& color)
{
	m_btn_color.set_color(color);
}

void Gobby::JoinDialog::on_response(int response_id)
{
	if(response_id == Gtk::RESPONSE_OK)
	{
		m_config["join"]["host"].set(get_host() );
		m_config["join"]["port"].set(get_port() );
		m_config["join"]["name"].set(get_name() );
		m_config["join"]["color"].set(get_color() );
	}

	Gtk::Dialog::on_response(response_id);
}

