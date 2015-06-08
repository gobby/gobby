/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2015 Armin Burgmeier <armin@arbur.net>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "core/sessionuserview.hpp"
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
		_("User List"), "user-list", opt_view));
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
