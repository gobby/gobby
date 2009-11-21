/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008, 2009 Armin Burgmeier <armin@arbur.net>
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

#ifndef _GOBBY_SESSIONVIEW_HPP_
#define _GOBBY_SESSIONVIEW_HPP_

#include "util/closebutton.hpp"

#include <gtkmm/box.h>
#include <gtkmm/frame.h>
#include <gtkmm/label.h>

#include <libinfinity/common/inf-session.h>

namespace Gobby
{

class SessionView: public Gtk::VBox
{
public:
	typedef sigc::signal<void, InfUser*> SignalActiveUserChanged;

	SessionView(InfSession* session, const Glib::ustring& title,
	            const Glib::ustring& path, const Glib::ustring& hostname);
	virtual ~SessionView();

	const InfSession* get_session() const { return m_session; }
	InfSession* get_session() { return m_session; }

	const Glib::ustring& get_title() const { return m_title; }
	const Glib::ustring& get_path() const { return m_path; }
	const Glib::ustring& get_hostname() const { return m_hostname; }

	void set_info(const Glib::ustring& info, bool closable);
	void unset_info();

	virtual InfUser* get_active_user() const;

	SignalActiveUserChanged signal_active_user_changed() const
	{
		return m_signal_active_user_changed;
	}

protected:
	void active_user_changed(InfUser* new_user);

	InfSession* m_session;

	const Glib::ustring m_title;
	const Glib::ustring m_path;
	const Glib::ustring m_hostname;

	Gtk::Frame m_info_frame;
	Gtk::VBox m_info_box;
	Gtk::HBox m_info_close_button_box;
	CloseButton m_info_close_button;
	Gtk::Label m_info_label;

private:
	SignalActiveUserChanged m_signal_active_user_changed;
};

}

#endif // _GOBBY_SESSIONVIEW_HPP_
