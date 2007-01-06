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
#include <gtkmm/enums.h>
#include "common.hpp"
#include "joindialog.hpp"

#ifdef WITH_ZEROCONF
Gobby::JoinDialog::Columns::Columns()
{
	add(name);
	add(host);
	add(port);
}
#endif

#ifndef WITH_ZEROCONF
Gobby::JoinDialog::JoinDialog(Gtk::Window& parent,
                              Config::ParentEntry& config_entry):
#else
Gobby::JoinDialog::JoinDialog(Gtk::Window& parent,
                              Config::ParentEntry& config_entry,
                              obby::zeroconf* zeroconf):
#endif
	Gtk::Dialog(_("Join obby session"), parent, true, true),
   m_config_entry(config_entry),
   m_table(4, 2),
   m_lbl_host(_("Host:"), Gtk::ALIGN_RIGHT),
   m_lbl_port(_("Port:"), Gtk::ALIGN_RIGHT),
   m_lbl_name(_("Name:"), Gtk::ALIGN_RIGHT),
   m_lbl_color(_("Colour:"), Gtk::ALIGN_RIGHT)
#ifdef WITH_ZEROCONF
   , m_ep_discover(_("Local network")), m_zeroconf(zeroconf)
#endif
{
	// TODO: Read default color as random one from tom's color map
	Gdk::Color default_color;
	default_color.set_red(0xcccc);
	default_color.set_green(0xcccc);
	default_color.set_blue(0xffff);

	Glib::ustring host = config_entry.get_value<Glib::ustring>(
		"join_host",
		Glib::ustring("localhost")
	);

	unsigned int port = config_entry.get_value<unsigned int>(
		"join_port",
		6522
	);

	Glib::ustring name = config_entry.get_value<Glib::ustring>(
		"name",
		Glib::get_user_name()
	);

	Gdk::Color color =  config_entry.get_value<Gdk::Color>(
		"color",
		default_color
	);

	m_ent_host.set_text(host);

	m_ent_port.set_range(1, 65535);
	m_ent_port.set_value(port);
	m_ent_port.set_increments(1, 256);

	m_ent_name.set_text(name);
	m_btn_color.set_color(color);

	m_ent_host.set_activates_default(true);
	m_ent_port.set_activates_default(true);
	m_ent_name.set_activates_default(true);

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
	m_vbox.set_spacing(5);
	m_vbox.pack_start(m_table);

#ifdef WITH_ZEROCONF
	if(m_zeroconf != NULL)
	{
		m_session_list = Gtk::ListStore::create(m_session_cols);
		m_session_view.set_model(m_session_list);
		m_session_view.append_column(_("User"), m_session_cols.name);
		m_session_view.append_column(_("Host"), m_session_cols.host);
		m_session_view.append_column(_("Port"), m_session_cols.port);
		m_session_view.get_selection()->set_mode(
			Gtk::SELECTION_SINGLE);
		m_session_view.get_selection()->signal_changed().connect(
			sigc::mem_fun(*this, &JoinDialog::on_change) );
		m_ep_discover.add(m_session_view);

		m_zeroconf->discover_event().connect(
			sigc::mem_fun(*this, &JoinDialog::on_discover));
		m_zeroconf->leave_event().connect(
			sigc::mem_fun(*this, &JoinDialog::on_leave) );

		m_vbox.pack_start(m_ep_discover);
	}
#endif

	get_vbox()->pack_start(m_vbox);

	add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
	set_default_response(Gtk::RESPONSE_OK);

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
		m_config_entry.set_value("join_host", get_host() );
		m_config_entry.set_value("join_port", get_port() );
		m_config_entry.set_value("name", get_name() );
		m_config_entry.set_value("color", get_color() );
	}

	Gtk::Dialog::on_response(response_id);
}

#ifdef WITH_ZEROCONF
Gtk::TreeModel::iterator
Gobby::JoinDialog::find_entry(const std::string& name) const
{
	Gtk::TreeModel::iterator iter = m_session_list->children().begin();
	for(iter; iter != m_session_list->children().end(); ++ iter)
		if( (*iter)[m_session_cols.name] == name)
			return iter;
	return m_session_list->children().end();
}

bool Gobby::JoinDialog::on_timer()
{
	m_zeroconf->select(0);
	return true;
}

void Gobby::JoinDialog::on_discover(const std::string& name,
                                    const net6::ipv4_address& addr)
{
	// Ignore entries which introduce user names which are already in
	// the list. The second of the clashing entries is just dropped.
	if(find_entry(name) != m_session_list->children().end() )
		return;
	Gtk::TreeModel::Row row = *(m_session_list->append() );
	row[m_session_cols.name] = name;
	row[m_session_cols.host] = addr.get_name();
	row[m_session_cols.port] = addr.get_port();
}

void Gobby::JoinDialog::on_leave(const std::string& name)
{
	Gtk::TreeModel::iterator iter = find_entry(name);
	if(iter == m_session_list->children().end() ) return;
	m_session_list->erase(iter);
}

void Gobby::JoinDialog::on_change()
{
	Gtk::TreeModel::iterator iter =
		m_session_view.get_selection()->get_selected();
	m_ent_host.set_text((*iter)[m_session_cols.host]);
	m_ent_port.set_value((*iter)[m_session_cols.port]);
}

void Gobby::JoinDialog::on_show()
{
	Gtk::Dialog::on_show();
	if(m_zeroconf != NULL && !m_timer_connection.connected())
	{
		m_session_list->clear();
		m_zeroconf->discover();
		m_timer_connection = Glib::signal_timeout().connect(
			sigc::mem_fun(*this, &JoinDialog::on_timer), 400);
	}
}

void Gobby::JoinDialog::on_hide()
{
	Gtk::Dialog::on_hide();
	m_timer_connection.disconnect();
}
#endif


