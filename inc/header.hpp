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

#ifndef _GOBBY_HEADER_HPP_
#define _GOBBY_HEADER_HPP_

#include <gtkmm/box.h>
#include <gtkmm/uimanager.h>
#include <gtkmm/menubar.h>
#include <gtkmm/toolbar.h>

namespace Gobby
{

class Header : public Gtk::VBox
{
public:
	class Error : public Glib::Error
	{
	public:
		enum Code {
			MENUBAR_MISSING,
			TOOLBAR_MISSING
		};

		Error(Code error_code, const Glib::ustring& error_message);
		Code code() const;
	};

	typedef sigc::signal<void> signal_session_create_type;
	typedef sigc::signal<void> signal_session_join_type;
	typedef sigc::signal<void> signal_session_quit_type;
	typedef sigc::signal<void> signal_quit_type;

	Header();
	~Header();

	signal_session_create_type session_create_event() const;
	signal_session_join_type session_join_event() const;
	signal_session_quit_type session_quit_event() const;
	signal_quit_type quit_event() const;

protected:
	void on_app_session_create();
	void on_app_session_join();
	void on_app_session_quit();
	void on_app_quit();

	Glib::RefPtr<Gtk::UIManager> m_ui_manager;
	Glib::RefPtr<Gtk::ActionGroup> m_group_app;
	Glib::RefPtr<Gtk::ActionGroup> m_group_session;

	Gtk::MenuBar* m_menubar;
	Gtk::Toolbar* m_toolbar;

	signal_session_create_type m_signal_session_create;
	signal_session_join_type m_signal_session_join;
	signal_session_quit_type m_signal_session_quit;
	signal_quit_type m_signal_quit;
};

}

#endif // _GOBBY_HEADER_HPP_
