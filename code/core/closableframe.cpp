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

#include "core/closableframe.hpp"
#include "util/closebutton.hpp"

#include <gtkmm/image.h>
#include <gtkmm/label.h>

Gobby::ClosableFrame::ClosableFrame(const Glib::ustring& title,
                                    const Glib::ustring& icon_name,
                                    Preferences::Option<bool>& option):
	m_option(option), m_allow_visible(true)
{
	CloseButton* button = Gtk::manage(new CloseButton);

	button->set_hexpand(true);
	button->set_halign(Gtk::ALIGN_END);

	button->signal_clicked().connect(
		sigc::mem_fun(*this, &ClosableFrame::on_clicked));
	m_option.signal_changed().connect(
		sigc::mem_fun(*this, &ClosableFrame::on_option));

	button->show();

	Gtk::Image* image = Gtk::manage(
		new Gtk::Image);
	image->set_from_icon_name(icon_name, Gtk::ICON_SIZE_MENU);
	image->show();

	Gtk::Label* label_title = Gtk::manage(
		new Gtk::Label(title, Gtk::ALIGN_START));
	label_title->show();

	m_grid.set_border_width(6);
	m_grid.set_column_spacing(6);
	m_grid.set_row_spacing(6);
	m_grid.attach(*image, 0, 0, 1, 1);
	m_grid.attach(*label_title, 1, 0, 1, 1);
	m_grid.attach(*button, 2, 0, 1, 1);
	m_grid.show();

	add(m_grid);

	on_option();
}

void Gobby::ClosableFrame::set_allow_visible(bool allow_visible)
{
	m_allow_visible = allow_visible;

	if(m_option && m_allow_visible) show();
	else hide();
}

void Gobby::ClosableFrame::on_add(Gtk::Widget* widget)
{
	if(widget == &m_grid)
		Gtk::Frame::on_add(widget);
	else
		m_grid.attach(*widget, 0, 1, 3, 1);
}

void Gobby::ClosableFrame::on_clicked()
{
	m_option = false;
}

void Gobby::ClosableFrame::on_option()
{
	if(m_option && m_allow_visible) show();
	else hide();
}
