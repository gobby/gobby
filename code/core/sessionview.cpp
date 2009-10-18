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

#include "features.hpp"

#include "core/sessionview.hpp"

Gobby::SessionView::SessionView(InfSession* session,
                                const Glib::ustring& title):
	m_session(session), m_title(title),
	m_info_box(false, 0), m_info_close_button_box(false, 6)
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
}

Gobby::SessionView::~SessionView()
{
	g_object_unref(m_session);
	//m_session = NULL; // TODO: Any reason to reset this?
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
