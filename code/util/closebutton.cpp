/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2015 Armin Burgmeier <armin@arbur.net>
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

#include <gtkmm/cssprovider.h>
#include <gtkmm/image.h>

Gobby::CloseButton::CloseButton()
{
        set_relief(Gtk::RELIEF_NONE);
        set_focus_on_click(false);

	static const gchar button_style[] =
		"* {\n"
		"  -GtkButton-default-border: 0;\n"
		"  -GtkButton-default-outside-border: 0;\n"
		"  -GtkButton-inner-border: 0;\n"
		"  -GtkWidget-focus-line-width: 0;\n"
		"  -GtkWidget-focus-padding: 0;\n"
		"  padding: 0;\n"
		"}";

	Glib::RefPtr<Gtk::CssProvider> provider = Gtk::CssProvider::create();
	provider->load_from_data(button_style);

	get_style_context()->add_provider(
		provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
	
        Gtk::Image* button_image = Gtk::manage(new Gtk::Image);
	button_image->set_from_icon_name("window-close", Gtk::ICON_SIZE_MENU);
        add(*button_image);
        button_image->show();
}
