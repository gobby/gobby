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

#ifndef _GOBBY_JOINDIALOG_HPP_
#define _GOBBY_JOINDIALOG_HPP_

#include <gtkmm/table.h>
#include <gtkmm/label.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/entry.h>
#include <gtkmm/colorbutton.h>
#include "defaultdialog.hpp"
#include "config.hpp"
#include "features.hpp"
#include "colorsel.hpp"
#ifdef WITH_HOWL
#include <gtkmm/box.h>
#include <gtkmm/expander.h>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include <sigc++/connection.h>
#include <net6/address.hpp>
#include <obby/zeroconf.hpp>
#endif

namespace Gobby
{

class JoinDialog : public DefaultDialog
{
public:
#ifdef WITH_HOWL
	class Columns : public Gtk::TreeModel::ColumnRecord
	{
	public:
		Columns();

		Gtk::TreeModelColumn<Glib::ustring> name;
		Gtk::TreeModelColumn<Glib::ustring> host;
		Gtk::TreeModelColumn<unsigned int> port;
	};
#endif

#ifndef WITH_HOWL
	JoinDialog(Gtk::Window& parent, Gobby::Config& config);
#else
	JoinDialog(Gtk::Window& parent, Gobby::Config& config,
	           obby::zeroconf* zeroconf);
#endif
	virtual ~JoinDialog();

	Glib::ustring get_host() const;
	unsigned int get_port() const;
	Glib::ustring get_name() const;
	Gdk::Color get_color() const;

	void set_host(const Glib::ustring& host);
	void set_port(unsigned int port);
	void set_name(const Glib::ustring& name);
	void set_color(const Gdk::Color& color);

protected:
	virtual void on_response(int response_id);

#ifdef WITH_HOWL
	Gtk::TreeModel::iterator find_entry(const std::string& name) const;
	virtual bool on_timer();
	virtual void on_discover(const std::string& name,
	                         const net6::ipv4_address& addr);
	virtual void on_leave(const std::string& name);
	virtual void on_change();
#endif

	Gobby::Config& m_config;

	Gtk::Table m_table;
	Gtk::Label m_lbl_host;
	Gtk::Label m_lbl_port;
	Gtk::Label m_lbl_name;
	Gtk::Label m_lbl_color;

#ifdef WITH_HOWL
	Gtk::VBox m_vbox;
	Gtk::Expander m_ep_discover;
	Gtk::TreeView m_session_view;
	Glib::RefPtr<Gtk::ListStore> m_session_list;
	Columns m_session_cols;
	obby::zeroconf* m_zeroconf;
	sigc::connection m_timer_connection;
#endif

	Gtk::Entry m_ent_host;
	Gtk::SpinButton m_ent_port;
	Gtk::Entry m_ent_name;
	ColorButton m_btn_color;
};

}

#endif // _GOBBY_JOINDIALOG_HPP_
