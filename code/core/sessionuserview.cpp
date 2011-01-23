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

#include "core/sessionuserview.hpp"
#include "core/iconmanager.hpp"
#include "core/closableframe.hpp"

#include "util/i18n.hpp"

// TODO: Consider using a single user list for all SessionViews, reparenting
// into the current SessionUserView's frame. Keep dummy widgets in other
// SessionUserViews so text does not resize.
// Or, maybe more favorable, just put the userlist outside of the notebook,
// but keep the notebook tabs on top of the userlist.

Gobby::SessionUserView::SessionUserView(SessionView& view,
                                        bool show_disconnected,
                                        Preferences::Option<bool>& opt_view,
                                        Preferences::Option<unsigned int>& w):
	m_view(view), m_userlist_width(w),
	m_userlist(inf_session_get_user_table(view.get_session()))
{
	m_userlist.show();
	m_userlist.set_show_disconnected(show_disconnected);
	Gtk::Frame* frame = Gtk::manage(new ClosableFrame(
		_("User List"), IconManager::STOCK_USERLIST, opt_view));
	frame->set_shadow_type(Gtk::SHADOW_IN);
	frame->add(m_userlist);
	// frame manages visibility itself

	pack1(view, true, false);
	pack2(*frame, false, false);
}

void Gobby::SessionUserView::on_size_allocate(Gtk::Allocation& allocation)
{
	Gtk::HPaned::on_size_allocate(allocation);

	// Setup initial paned position. We can't do this simply every time
	// on_size_allocate() is called since this would lead to an endless
	// loop somehow when the userlist width is changed forcefully 
	// (for example by a m_view.set_info() requiring much width).
	if(!m_doc_userlist_width_changed_connection.connected())
	{
		Glib::SignalProxyProperty proxy =
			property_position().signal_changed();

		m_doc_userlist_width_changed_connection =
			proxy.connect(sigc::mem_fun(
				*this,
				&SessionUserView::
					on_doc_userlist_width_changed));

		Preferences::Option<unsigned int>& option =
			m_userlist_width;

		m_pref_userlist_width_changed_connection =
			option.signal_changed().connect(sigc::mem_fun(
				*this,
				&SessionUserView::
					on_pref_userlist_width_changed));

		int desired_position =
			get_width() - m_userlist_width;
		desired_position = std::min<unsigned int>(
			desired_position, property_max_position());

		if(get_position() != desired_position)
			set_position(desired_position);
	}
}

void Gobby::SessionUserView::on_doc_userlist_width_changed()
{
	unsigned int userlist_width = get_width() - get_position();

	if(m_userlist_width != userlist_width)
	{
		m_pref_userlist_width_changed_connection.block();
		m_userlist_width = userlist_width;
		m_pref_userlist_width_changed_connection.unblock();
	}
}

void Gobby::SessionUserView::on_pref_userlist_width_changed()
{
	int position = get_width() - m_userlist_width;

	if(get_position() != position)
	{
		m_doc_userlist_width_changed_connection.block();
		set_position(position);
		m_doc_userlist_width_changed_connection.unblock();
	}
}
