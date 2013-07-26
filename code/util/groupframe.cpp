/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2013 Armin Burgmeier <armin@arbur.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include "util/groupframe.hpp"

#include <gtkmm/label.h>

Gobby::GroupFrame::GroupFrame(const Glib::ustring& title):
	m_box(false, 6)
{
	Gtk::Label* title_label = Gtk::manage(new Gtk::Label);
	title_label->set_markup(
		"<b>" + Glib::Markup::escape_text(title) + "</b>");
	set_label_widget(*title_label);
	title_label->show();

	m_box.show();

	m_alignment.set_padding(6, 0, 12, 0);
	m_alignment.add(m_box);
	m_alignment.show();

	set_shadow_type(Gtk::SHADOW_NONE);
	Gtk::Frame::add(m_alignment);
}

void Gobby::GroupFrame::add(Gtk::Widget& widget)
{
	m_box.pack_start(widget, Gtk::PACK_EXPAND_WIDGET);
}
