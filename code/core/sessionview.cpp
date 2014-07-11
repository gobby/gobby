/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2014 Armin Burgmeier <armin@arbur.net>
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
 * Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include "features.hpp"

#include "core/sessionview.hpp"

Gobby::SessionView::SessionView(InfSession* session,
                                const Glib::ustring& title,
                                const Glib::ustring& path,
                                const Glib::ustring& hostname):
	m_session(session), m_title(title), m_path(path),
	m_hostname(hostname), m_info_box(false, 0),
	m_info_close_button_box(false, 6)
{
	g_object_ref(m_session);

	m_info_label.set_selectable(true);
	m_info_label.set_line_wrap(true);
	m_info_label.show();

	m_info_close_button.signal_clicked().connect(
		sigc::mem_fun(m_info_frame, &Gtk::Frame::hide));
	m_info_close_button.show();

	m_info_close_button_box.pack_end(m_info_close_button, Gtk::PACK_SHRINK);
	// Don't show info close button box by default

	m_info_box.pack_start(m_info_close_button_box, Gtk::PACK_SHRINK);
	m_info_box.pack_start(m_info_label, Gtk::PACK_SHRINK);
	m_info_box.set_border_width(6);
	m_info_box.show();

	m_info_frame.set_shadow_type(Gtk::SHADOW_IN);
	m_info_frame.add(m_info_box);
	// Don't show infoframe by default

	pack_start(m_info_frame, Gtk::PACK_SHRINK);
	
	// TODO: Would this be the best way to handle it?
	/*m_notify_rename_handler = g_signal_connect(
		G_OBJECT(session), "notify::rename",
		G_CALLBACK(on_session_name_changed_static), this);*/
}

Gobby::SessionView::~SessionView()
{
	/*g_signal_handler_disconnect(G_OBJECT(m_session),
					m_notify_rename_handler);*/
	g_object_unref(m_session);
}

void Gobby::SessionView::set_info(const Glib::ustring& info, bool closable)
{
	m_info_label.set_text(info);

	if(closable) m_info_close_button_box.show();
	else m_info_close_button_box.hide();

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

void Gobby::SessionView::session_name_changed(const gchar* new_name)
{
	const Glib::ustring& str(new_name);
	m_title = str;
	m_signal_session_name_changed.emit(str);
}