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

#include "features.hpp"

#include "core/sessionview.hpp"

Gobby::SessionView::SessionView(InfSession* session,
                                const Glib::ustring& title,
                                const Glib::ustring& path,
                                const Glib::ustring& hostname):
	m_session(session), m_title(title), m_path(path),
	m_hostname(hostname)
{
	g_object_ref(m_session);

	m_info_label.set_halign(Gtk::ALIGN_START);
	m_info_label.set_hexpand(true);
	m_info_label.set_selectable(true);
	m_info_label.set_line_wrap(true);
	m_info_label.show();

	m_info_close_button.set_halign(Gtk::ALIGN_END);
	m_info_close_button.set_valign(Gtk::ALIGN_START);
	m_info_close_button.signal_clicked().connect(
		sigc::mem_fun(m_info_frame, &Gtk::Frame::hide));
	// Don't show info close button by default

	m_info_grid.set_orientation(Gtk::ORIENTATION_VERTICAL);
	m_info_grid.set_row_spacing(6);
	m_info_grid.attach(m_info_close_button, 0, 0, 1, 1);
	m_info_grid.attach(m_info_label, 0, 1, 1, 1);
	m_info_grid.set_border_width(6);
	m_info_grid.show();

	m_info_frame.set_shadow_type(Gtk::SHADOW_IN);
	m_info_frame.add(m_info_grid);
	// Don't show infoframe by default

	set_orientation(Gtk::ORIENTATION_VERTICAL);
	attach(m_info_frame, 0, 0, 1, 1);
}

Gobby::SessionView::~SessionView()
{
	g_object_unref(m_session);
	//m_session = NULL; // TODO: Any reason to reset this?
}

void Gobby::SessionView::set_info(const Glib::ustring& info, bool closable)
{
	m_info_label.set_text(info);

	if(closable) m_info_close_button.show();
	else m_info_close_button.hide();

	m_info_frame.show();
}

void Gobby::SessionView::unset_info()
{
	m_info_frame.hide();
}

InfUser* Gobby::SessionView::get_active_user() const
{
	return NULL;
}

void Gobby::SessionView::active_user_changed(InfUser* new_user)
{
	m_signal_active_user_changed.emit(new_user);
}
