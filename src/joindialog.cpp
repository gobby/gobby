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

#include <list>
#include <iostream>
#include <gtkmm/stock.h>
#include <gtkmm/enums.h>
#include <gtkmm/alignment.h>

#ifdef GTKMM_ATKMM_ENABLED
#include <atkmm/relationset.h>
#include <atkmm/relation.h>
#include <atkmm/object.h>
#endif

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
                              obby::zeroconf_base* zeroconf):
#endif
	Gtk::Dialog(_("Join obby session"), parent, true, true),
   m_config_entry(config_entry),
   m_table(4, 2),
   m_lbl_host(_("Host:"), Gtk::ALIGN_RIGHT),
   m_lbl_port(_("Port:"), Gtk::ALIGN_RIGHT),
   m_lbl_name(_("Name:"), Gtk::ALIGN_RIGHT),
   m_lbl_color(_("Colour:"), Gtk::ALIGN_RIGHT),
#ifdef WITH_ZEROCONF
   m_ep_discover(_("Local network")), m_zeroconf(zeroconf),
#endif
   m_btn_color(config_entry)
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
	m_vbox.pack_start(m_table, Gtk::PACK_SHRINK);

#ifdef WITH_ZEROCONF
	if(m_zeroconf != NULL)
	{
		m_session_list = Gtk::ListStore::create(m_session_cols);
		m_session_view.set_model(m_session_list);
		m_session_view.get_selection()->set_mode(
			Gtk::SELECTION_SINGLE);
		m_session_view.get_selection()->signal_changed().connect(
			sigc::mem_fun(*this, &JoinDialog::on_change) );
		m_ep_discover.add(m_session_view);

		m_zeroconf->discover_event().connect(sigc::mem_fun(
			*this,
			&JoinDialog::on_discover<net6::ipv4_address>));
		m_zeroconf->discover6_event().connect(sigc::mem_fun(
			*this,
			&JoinDialog::on_discover<net6::ipv6_address>));
		m_zeroconf->leave_event().connect(
			sigc::mem_fun(*this, &JoinDialog::on_leave) );

		Gtk::Alignment* alignment = new Gtk::Alignment(0.5, 0.0);
		alignment->add(m_ep_discover);
		m_vbox.pack_start(*alignment, Gtk::PACK_EXPAND_WIDGET);
	}
#endif

	get_vbox()->pack_start(m_vbox);

	add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
	set_default_response(Gtk::RESPONSE_OK);

	show_all();
	set_border_width(10);
	set_resizable(true);

#ifdef GTKMM_ATKMM_ENABLED
	// Add label associations to get proper accessibility.
	m_lbl_host.get_accessible()->get_relation_set()->set_add(
		Atk::Relation::create(
			std::list<Glib::RefPtr<Atk::Object> >(1, m_ent_host.get_accessible()),
			Atk::RELATION_LABEL_FOR)
		);
	m_lbl_port.get_accessible()->get_relation_set()->set_add(
		Atk::Relation::create(
			std::list<Glib::RefPtr<Atk::Object> >(1, m_ent_port.get_accessible()),
			Atk::RELATION_LABEL_FOR)
		);
	m_lbl_name.get_accessible()->get_relation_set()->set_add(
		Atk::Relation::create(
			std::list<Glib::RefPtr<Atk::Object> >(1, m_ent_name.get_accessible()),
			Atk::RELATION_LABEL_FOR)
		);
	m_lbl_color.get_accessible()->get_relation_set()->set_add(
		Atk::Relation::create(
			std::list<Glib::RefPtr<Atk::Object> >(1, m_btn_color.get_accessible()),
			Atk::RELATION_LABEL_FOR)
		);
#endif
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

#ifndef WITH_AVAHI
bool Gobby::JoinDialog::on_timer()
{
	m_zeroconf->select(0);
	return true;
}
#endif

template <typename addr_type>
void Gobby::JoinDialog::on_discover(const std::string& name,
                                    const addr_type& addr)
{
	// Ignore entries which introduce user names which are already in
	// the list. The second of the clashing entries is just dropped.
	if(find_entry(name) != m_session_list->children().end() )
		return;

	Gtk::TreeModel::Row row = *(m_session_list->append() );
	row[m_session_cols.name] = name;
	row[m_session_cols.host] = addr.get_name();
	// Generic addresses do not bear ports, thus the passed addr_type
	// must implement get_port().
	row[m_session_cols.port] = addr.get_port();

	m_ep_discover.set_expanded(true);
}

void Gobby::JoinDialog::on_leave(const std::string& name)
{
	Gtk::TreeModel::iterator iter = find_entry(name);
	if(iter == m_session_list->children().end() ) return;
	m_session_list->erase(iter);
}

void Gobby::JoinDialog::on_change()
{
	if(m_session_view.get_selection()->count_selected_rows() > 0)
	{
		Gtk::TreeModel::iterator iter =
			m_session_view.get_selection()->get_selected();

		m_ent_host.set_text((*iter)[m_session_cols.host]);
		m_ent_port.set_value((*iter)[m_session_cols.port]);
	}
}

void Gobby::JoinDialog::on_show()
{
	Gtk::Dialog::on_show();
	if(m_zeroconf != NULL && !m_timer_connection.connected())
	{
		m_session_list->clear();
		try {
			// clear treeview columns
			Gtk::TreeViewColumn* col = NULL;
			while((col = m_session_view.get_column(0)) != NULL)
				m_session_view.remove_column(*col);

			// hide the expanders contents, it will automatically be opened
			// when items are discovered
			m_ep_discover.set_expanded(false);

			// discover elements
			m_zeroconf->discover();

			// resetup columns
			m_session_view.append_column(_("User"), m_session_cols.name);
			m_session_view.append_column(_("Host"), m_session_cols.host);
			m_session_view.append_column(_("Port"), m_session_cols.port);

			// Enable selection
			m_session_view.get_selection()->set_mode(Gtk::SELECTION_SINGLE);
		} catch(std::runtime_error& e) {
			std::cerr << "Discovery failed: " << e.what() << std::endl;

			// setup failure columns
			Gtk::CellRendererText* failure = Gtk::manage(
				new Gtk::CellRendererText());
			failure->property_text().set_value(e.what() );
			g_object_set(G_OBJECT(failure->gobj()), "ellipsize",
				PANGO_ELLIPSIZE_END, NULL);
			Gtk::CellRendererPixbuf* stop = Gtk::manage(
				new Gtk::CellRendererPixbuf());
			stop->property_stock_id().set_value("gtk-dialog-error");

			// append them
			Gtk::TreeViewColumn* column = Gtk::manage(
				new Gtk::TreeViewColumn(_("Failure")));

			column->pack_start(*stop, false);
			column->pack_start(*failure, true);
			column->set_spacing(8);
			m_session_view.append_column(*column);

			// Disable selection
			m_session_view.get_selection()->set_mode(Gtk::SELECTION_NONE);

			// create a dummy row for the renderer to be displayed
			// and discard the pointer
			m_session_list->append();
			m_ep_discover.set_expanded(true);
		}
#ifndef WITH_AVAHI
		m_timer_connection = Glib::signal_timeout().connect(
			sigc::mem_fun(*this, &JoinDialog::on_timer), 400);
#endif
	}
}

void Gobby::JoinDialog::on_hide()
{
	Gtk::Dialog::on_hide();
	m_timer_connection.disconnect();
}
#endif


