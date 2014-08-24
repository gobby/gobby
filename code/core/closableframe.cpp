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
		new Gtk::Label(title, Gtk::ALIGN_START));
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
