/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2011 Armin Burgmeier <armin@arbur.net>
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

#include "core/closableframe.hpp"
#include "util/closebutton.hpp"
#include "util/gtk-compat.hpp"

#include <gtkmm/image.h>
#include <gtkmm/label.h>

Gobby::ClosableFrame::ClosableFrame(const Glib::ustring& title,
                                    const Gtk::StockID& stock_id,
                                    Preferences::Option<bool>& option):
	m_option(option), m_box(false, 6), m_allow_visible(true)
{
	CloseButton* button = Gtk::manage(new CloseButton);

	button->signal_clicked().connect(
		sigc::mem_fun(*this, &ClosableFrame::on_clicked));
	m_option.signal_changed().connect(
		sigc::mem_fun(*this, &ClosableFrame::on_option));

	button->show();

	Gtk::Image* image = Gtk::manage(
		new Gtk::Image(stock_id, Gtk::ICON_SIZE_MENU));
	image->show();

	Gtk::Label* label_title = Gtk::manage(
		new Gtk::Label(title, GtkCompat::ALIGN_LEFT));
	label_title->show();

	Gtk::HBox* hbox = Gtk::manage(new Gtk::HBox(false, 6));
	hbox->pack_start(*image, Gtk::PACK_SHRINK);
	hbox->pack_start(*label_title, Gtk::PACK_SHRINK);
	hbox->pack_end(*button, Gtk::PACK_SHRINK);
	hbox->show();

	m_box.set_border_width(6);
	m_box.pack_start(*hbox, Gtk::PACK_SHRINK);
	m_box.show();

	add(m_box);

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
	if(widget == &m_box)
		Gtk::Frame::on_add(widget);
	else
		m_box.pack_start(*widget, Gtk::PACK_EXPAND_WIDGET);
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
