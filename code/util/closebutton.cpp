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

#include "util/closebutton.hpp"

#include <gtkmm/image.h>
#include <gtkmm/stock.h>
#include <gtk/gtk.h>

Gobby::CloseButton::CloseButton()
{
        set_relief(Gtk::RELIEF_NONE);
        set_focus_on_click(false);
	//set_flags(get_flags() & ~Gtk::CAN_FOCUS);

        GtkRcStyle* rc_style = gtk_rc_style_new();
        rc_style->xthickness = 0;
        rc_style->ythickness = 0;
        gtk_widget_modify_style(GTK_WIDGET(gobj()), rc_style);
        g_object_unref(rc_style);

        Gtk::Image* button_image = Gtk::manage(
                new Gtk::Image(Gtk::Stock::CLOSE, Gtk::ICON_SIZE_MENU));
        add(*button_image);
        button_image->show();
}

#ifdef USE_GTKMM3
void Gobby::CloseButton::on_style_updated()
#else
void Gobby::CloseButton::on_style_changed(
	const Glib::RefPtr<Gtk::Style>& previous_style)
#endif
{
	int width;
	int height;

	gtk_icon_size_lookup_for_settings(
		gtk_widget_get_settings(GTK_WIDGET(gobj())),
		GTK_ICON_SIZE_MENU, &width, &height);
	set_size_request(width + 2, height + 2);
}
