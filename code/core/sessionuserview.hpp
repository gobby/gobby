/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2010 Armin Burgmeier <armin@arbur.net>
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

#ifndef _GOBBY_SESSIONUSERVIEW_HPP_
#define _GOBBY_SESSIONUSERVIEW_HPP_

#include "core/sessionview.hpp"
#include "core/preferences.hpp"
#include "core/userlist.hpp"

#include <gtkmm/paned.h>

// Shows a sessionview with a userlist on the right hand side of it
namespace Gobby
{

class SessionUserView: public Gtk::HPaned
{
public:
	SessionUserView(SessionView& view, bool show_disconnected,
	                Preferences::Option<bool>& userlist_view,
	                Preferences::Option<unsigned int>& userlist_width);

	SessionView& get_session_view() { return m_view; }
	const SessionView& get_session_view() const { return m_view; }

protected:
	virtual void on_size_allocate(Gtk::Allocation& allocation);

	void on_doc_userlist_width_changed();
	void on_pref_userlist_width_changed();

	SessionView& m_view;
	Preferences::Option<unsigned int>& m_userlist_width;
	UserList m_userlist;

	sigc::connection m_doc_userlist_width_changed_connection;
	sigc::connection m_pref_userlist_width_changed_connection;
};

}

#endif // _GOBBY_SESSIONUSERVIEW_HPP_
