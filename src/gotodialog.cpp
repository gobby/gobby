/* gobby - A GTKmm driven libobby client
 * Copyright (C) 2005 0x539 dev group
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

#include <gtkmm/stock.h>
#include <gtkmm/messagedialog.h>

#include "common.hpp"
#include "document.hpp"
#include "gotodialog.hpp"
#include "window.hpp"

Gobby::GotoDialog::GotoDialog(Gobby::Window& parent):
	ToolWindow(parent),
	m_window(parent),
	m_lbl_info(_("Line number:"), Gtk::ALIGN_RIGHT),
	m_btn_close(Gtk::Stock::CLOSE),
	m_btn_goto(_("_Go to line") )
{
	Gtk::Image* goto_img = Gtk::manage(
		new Gtk::Image(
			Gtk::Stock::JUMP_TO,
			Gtk::ICON_SIZE_BUTTON
		)
	);

	m_btn_goto.set_image(*goto_img);

	// TODO: Change this value according to line count in current document?
	m_ent_line.set_range(1, 0x7fffffff);
	m_ent_line.set_increments(1, 10);
	m_ent_line.set_activates_default(true);

	m_box_top.set_spacing(10);
	m_box_top.pack_start(m_lbl_info, Gtk::PACK_SHRINK);
	m_box_top.pack_start(m_ent_line, Gtk::PACK_EXPAND_WIDGET);

	m_box_bottom.set_homogeneous(true);
	m_box_bottom.set_spacing(6);
	m_box_bottom.pack_end(m_btn_goto, Gtk::PACK_SHRINK);
	m_box_bottom.pack_end(m_btn_close, Gtk::PACK_SHRINK);

	m_mainbox.set_spacing(12);
	m_mainbox.pack_start(m_box_top, Gtk::PACK_SHRINK);
	m_mainbox.pack_start(m_sep, Gtk::PACK_SHRINK);
	m_mainbox.pack_start(m_box_bottom, Gtk::PACK_SHRINK);

	add(m_mainbox);

	m_btn_close.signal_clicked().connect(
		sigc::mem_fun(*this, &GotoDialog::hide) );
	m_btn_goto.signal_clicked().connect(
		sigc::mem_fun(*this, &GotoDialog::on_goto) );
	m_ent_line.signal_activate().connect(
		sigc::mem_fun(*this, &GotoDialog::on_goto) );

	GTK_WIDGET_SET_FLAGS(m_btn_goto.gobj(), GTK_CAN_DEFAULT);
	set_default(m_btn_goto);

	set_border_width(16);

	set_resizable(false);
	set_title(_("Go to line") );

	show_all_children();
}

void Gobby::GotoDialog::on_show()
{
	m_ent_line.grab_focus();

	Gobby::DocWindow* window = m_window.get_current_document();
	if(window != NULL)
	{
		Gtk::TextIter cursor = window->get_document().get_buffer()->
			get_insert()->get_iter();

		m_ent_line.set_value(cursor.get_line() + 1);
	}

	ToolWindow::on_show();
}

void Gobby::GotoDialog::on_goto()
{
	Gobby::DocWindow* window = m_window.get_current_document();
	if(window != NULL)
	{
		int value = m_ent_line.get_value_as_int();
		Gtk::TextIter begin = window->get_document().get_buffer()->
			get_iter_at_line(value - 1);
		window->set_selection(begin, begin);
	}
}
