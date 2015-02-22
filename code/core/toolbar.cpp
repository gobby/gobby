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

#include "core/toolbar.hpp"

#include <gtkmm/toolbar.h>
#include <gtkmm/builder.h>

Gobby::Toolbar::Toolbar(const Preferences& preferences):
	Gtk::Box(Gtk::ORIENTATION_VERTICAL),
	m_preferences(preferences)
{
	Glib::RefPtr<Gtk::Builder> builder =
		Gtk::Builder::create_from_resource(
			"/de/0x539/gobby/ui/toolbar.ui");

	builder->get_widget("toolbar", m_toolbar);

	// Initial settings
	on_toolbar_style_changed();
	on_show_toolbar_changed();

	preferences.appearance.toolbar_style.signal_changed().connect(
		sigc::mem_fun(*this, &Toolbar::on_toolbar_style_changed));
	preferences.appearance.show_toolbar.signal_changed().connect(
		sigc::mem_fun(*this, &Toolbar::on_show_toolbar_changed));

	pack_start(*m_toolbar, Gtk::PACK_SHRINK);
}

void Gobby::Toolbar::on_toolbar_style_changed()
{
	m_toolbar->set_toolbar_style(m_preferences.appearance.toolbar_style);
}

void Gobby::Toolbar::on_show_toolbar_changed()
{
	if(m_preferences.appearance.show_toolbar)
		m_toolbar->show();
	else
		m_toolbar->hide();
}
