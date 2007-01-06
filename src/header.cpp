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
#include "header.hpp"

Gobby::Header::Error::Error(Code error_code, const Glib::ustring& error_message)
 : Glib::Error(g_quark_from_static_string("GOBBY_HEADER_ERROR"),
               static_cast<int>(error_code), error_message)
{
}

Gobby::Header::Error::Code Gobby::Header::Error::code() const
{
	return static_cast<Code>(gobject_->code);
}

Gobby::Header::Header()
 : m_ui_manager(Gtk::UIManager::create() ),
   m_group_app(Gtk::ActionGroup::create() ),
   m_group_session(Gtk::ActionGroup::create() )
{
	// App menu
	m_group_app->add(Gtk::Action::create("MenuApp", "Gobby") );

	// Create session
	m_group_app->add(
		Gtk::Action::create(
			"CreateSession",
			Gtk::Stock::NETWORK,
			"Create session",
			"Opens a new obby session"
		),
		sigc::mem_fun(
			*this,
			&Header::on_app_session_create
		)
	);

	// Join session
	m_group_app->add(
		Gtk::Action::create(
			"JoinSession",
			Gtk::Stock::CONNECT,
			"Join session",
			"Join an existing obby session"
		),
		sigc::mem_fun(
			*this,
			&Header::on_app_session_join
		)
	);

	// Quit session
	m_group_app->add(
		Gtk::Action::create(
			"QuitSession",
			Gtk::Stock::DISCONNECT,
			"Quit session",
			"Leaves the currently running obby session"
		),
		sigc::mem_fun(
			*this,
			&Header::on_app_session_quit
		)
	);

	// Quit application
	m_group_app->add(
		Gtk::Action::create(
			"Quit",
			Gtk::Stock::QUIT,
			"Quit",
			"Quits the application"
		),
		sigc::mem_fun(
			*this,
			&Header::on_app_quit
		)
	);

	m_ui_manager->insert_action_group(m_group_app);
//	m_ui_manager->insert_action_group(m_group_session);
	m_ui_manager->add_ui_from_file("ui.xml");

	m_menubar = static_cast<Gtk::MenuBar*>(
		m_ui_manager->get_widget("/MenuMainBar") );
	m_toolbar = static_cast<Gtk::Toolbar*>(
		m_ui_manager->get_widget("/ToolMainBar") );

	if(m_menubar == NULL)
		throw Error(Error::MENUBAR_MISSING, "ui.xml lacks menubar");
	if(m_toolbar == NULL)
		throw Error(Error::TOOLBAR_MISSING, "ui.xml lacks toolbar");

	pack_start(*m_menubar, Gtk::PACK_SHRINK);
	pack_start(*m_toolbar, Gtk::PACK_SHRINK);

	// Without having joined a session, it may not be quit
	m_group_app->get_action("QuitSession")->set_sensitive(false);
}

Gobby::Header::~Header()
{
}

Gobby::Header::signal_session_create_type
Gobby::Header::session_create_event() const
{
	return m_signal_session_create;
}

Gobby::Header::signal_session_join_type
Gobby::Header::session_join_event() const
{
	return m_signal_session_join;
}

Gobby::Header::signal_session_quit_type
Gobby::Header::session_quit_event() const
{
	return m_signal_session_quit;
}

Gobby::Header::signal_quit_type
Gobby::Header::quit_event() const
{
	return m_signal_quit;
}

void Gobby::Header::on_app_session_create()
{
	m_signal_session_create.emit();
}

void Gobby::Header::on_app_session_join()
{
	m_signal_session_join.emit();
}

void Gobby::Header::on_app_session_quit()
{
	m_signal_session_quit.emit();
}

void Gobby::Header::on_app_quit()
{
	m_signal_quit.emit();
}

