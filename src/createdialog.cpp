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
#include "createdialog.hpp"

Gobby::CreateDialog::CreateDialog(Gtk::Window& parent, Config& config)
 : Gtk::Dialog("Create obby session", parent, true, true), m_config(config),
   m_table(3, 2),
   m_lbl_port("Port:", Gtk::ALIGN_RIGHT),
   m_lbl_name("Name:", Gtk::ALIGN_RIGHT),
   m_lbl_color("Color:", Gtk::ALIGN_RIGHT)
{
	m_ent_port.set_range(1024, 65535);
	m_ent_port.set_value(config["create"]["port"].get(6522) );
	m_ent_port.set_increments(1, 256);

	// TODO: Share user settings between create and join dialog
	Glib::ustring name =
		config["create"]["name"].get(Glib::get_user_name() );
	Gdk::Color color =
		config["create"]["color"].get(Gdk::Color("black") );

	m_ent_name.set_text(name);
	m_btn_color.set_color(color);

	m_table.attach(m_lbl_port, 0, 1, 0, 1,
		Gtk::SHRINK | Gtk::FILL, Gtk::SHRINK);
	m_table.attach(m_lbl_name, 0, 1, 1, 2,
		Gtk::SHRINK | Gtk::FILL, Gtk::SHRINK);
	m_table.attach(m_lbl_color, 0, 1, 2, 3,
		Gtk::SHRINK | Gtk::FILL, Gtk::SHRINK);

	m_table.attach(m_ent_port, 1, 2, 0, 1,
		Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK);
	m_table.attach(m_ent_name, 1, 2, 1, 2,
		Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK);
	m_table.attach(m_btn_color, 1, 2, 2, 3,
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

Gobby::CreateDialog::~CreateDialog()
{
}

unsigned int Gobby::CreateDialog::get_port() const
{
	return static_cast<unsigned int>(m_ent_port.get_value() );
}

Glib::ustring Gobby::CreateDialog::get_name() const
{
	return m_ent_name.get_text();
}

Gdk::Color Gobby::CreateDialog::get_color() const
{
	return m_btn_color.get_color();
}

void Gobby::CreateDialog::set_port(unsigned int port)
{
	m_ent_port.set_value(static_cast<double>(port) );
}

void Gobby::CreateDialog::set_name(const Glib::ustring& name)
{
	m_ent_name.set_text(name);
}

void Gobby::CreateDialog::set_color(const Gdk::Color& color)
{
	m_btn_color.set_color(color);
}

void Gobby::CreateDialog::on_response(int response_id)
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

