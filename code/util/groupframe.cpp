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

#include "util/groupframe.hpp"

#include <gtkmm/label.h>
#include <glibmm/markup.h>

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
