/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2014 Armin Burgmeier <armin@arbur.net>
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

	SessionView& get_session_view() const { return m_view; }

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
