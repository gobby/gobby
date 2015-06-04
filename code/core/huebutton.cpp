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

#include "core/huebutton.hpp"

#include "util/color.hpp"
#include "util/i18n.hpp"

#include <libinftextgtk/inf-text-gtk-hue-chooser.h>

Gobby::HueButton::HueButton(GtkColorButton* cobject,
                            const Glib::RefPtr<Gtk::Builder>& builder):
	Gtk::ColorButton(cobject), m_saturation(1.0), m_value(1.0)
{
	set_hue(1.0);
}

Gobby::HueButton::HueButton(const Glib::ustring& title):
	m_saturation(1.0), m_value(1.0)
{
	set_title(title);
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
		Gtk::Window* parent = NULL;
		Gtk::Widget* toplevel_widget = get_toplevel();
		if(gtk_widget_is_toplevel(toplevel_widget->gobj()))
			parent = dynamic_cast<Gtk::Window*>(toplevel_widget);

		if(parent == NULL)
		{
			g_warning("Gobby::HueButton::on_clicked: "
			          "No toplevel widget found");
			return;
		}

		m_dialog.reset(new Gtk::Dialog(get_title(), *parent));
		m_hue_chooser =
			inf_text_gtk_hue_chooser_new_with_hue(get_hue());
		gtk_box_pack_start(GTK_BOX(m_dialog->get_vbox()->gobj()),
		                   m_hue_chooser, FALSE, FALSE, 0);
		gtk_widget_show(m_hue_chooser);
		m_dialog->add_button(_("_Cancel"), Gtk::RESPONSE_CANCEL);
		m_dialog->add_button(_("_Ok"), Gtk::RESPONSE_OK);
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
