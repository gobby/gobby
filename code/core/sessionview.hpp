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
