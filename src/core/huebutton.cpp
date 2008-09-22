/* gobby - A GTKmm driven libobby client
 * Copyright (C) 2005 - 2008 0x539 dev group
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

#include "core/huebutton.hpp"

#include "util/color.hpp"

#include <gtkmm/stock.h>

#include <libinftextgtk/inf-text-gtk-hue-chooser.h>

Gobby::HueButton::HueButton(const Glib::ustring& title, Gtk::Window& parent):
	m_title(title), m_parent(parent), m_saturation(1.0), m_value(1.0)
{
	parent.signal_hide().connect(
		sigc::mem_fun(*this, &HueButton::on_parent_hide));
	set_hue(1.0);
}

double Gobby::HueButton::get_hue() const
{
	return hue_from_gdk_color(get_color());
}

double Gobby::HueButton::get_saturation() const
{
	return m_saturation;
}

double Gobby::HueButton::get_value() const
{
	return m_value;
}

void Gobby::HueButton::set_hue(double hue)
{
	Gdk::Color color;
	color.set_hsv(hue * 360.0, m_saturation, m_value);
	set_color(color);
}

void Gobby::HueButton::set_saturation(double saturation)
{
	m_saturation = saturation;
	set_hue(get_hue()); // Update view
}

void Gobby::HueButton::set_value(double value)
{
	m_value = value;
	set_hue(get_hue()); // Update view
}

void Gobby::HueButton::on_clicked()
{
	if(!m_dialog.get())
	{
		m_dialog.reset(new Gtk::Dialog(m_title, m_parent));
		m_hue_chooser =
			inf_text_gtk_hue_chooser_new_with_hue(get_hue());
		gtk_box_pack_start(GTK_BOX(m_dialog->get_vbox()->gobj()),
		                   m_hue_chooser, FALSE, FALSE, 0);
		gtk_widget_show(m_hue_chooser);
		m_dialog->add_button(Gtk::Stock::CANCEL,
		                     Gtk::RESPONSE_CANCEL);
		m_dialog->add_button(Gtk::Stock::OK,
		                     Gtk::RESPONSE_OK);
		m_dialog->set_default_response(Gtk::RESPONSE_OK);
		m_dialog->set_resizable(false);
		m_dialog->signal_response().connect(
			sigc::mem_fun(*this,
			              &HueButton::on_dialog_response));
	}
	else
	{
		inf_text_gtk_hue_chooser_set_hue(
			INF_TEXT_GTK_HUE_CHOOSER(m_hue_chooser), get_hue());
	}

	m_dialog->present();
}

void Gobby::HueButton::on_parent_hide()
{
	// Hide child dialog when the window containing the color button
	// goes away
	m_dialog.reset(NULL);
}

void Gobby::HueButton::on_dialog_response(int response_id)
{
	if(response_id == Gtk::RESPONSE_OK)
	{
		set_hue(
			inf_text_gtk_hue_chooser_get_hue(
				INF_TEXT_GTK_HUE_CHOOSER(m_hue_chooser)));
	}

	m_dialog->hide();
}
