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

#include <gtkmm/frame.h>
#include <gtkmm/drawingarea.h>
#include "colorsel.hpp"

namespace
{
	const gchar* GCOLOR_STRING =
		"#f9dad4:#fddfbe:#fdf3be:#f5fdbe:#dcfdbe:#befde2:#bee9fd:"
		"#becffd:#c7befd:#f2befd:#fdbedd:#f6abab:#fbc759:#fcffba:"
		"#f4ffee:#bce9b8:#eefdff:#e8ebff:#fdeeff:#dfdfdf";

	void set_default_palette(Gtk::ColorSelection& selection)
	{
		GtkSettings* settings =
			gtk_widget_get_settings(GTK_WIDGET(selection.gobj()) );

		g_object_set(
			settings,
			"gtk-color-palette",
			GCOLOR_STRING,
			NULL
		);

		selection.set_has_palette(true);
	}
}

Gobby::ColorSelection::ColorSelection()
 : Gtk::ColorSelection()
{
	set_default_palette(*this);
}

Gobby::ColorSelection::~ColorSelection()
{
}

Gobby::ColorSelectionDialog::ColorSelectionDialog()
 : Gtk::ColorSelectionDialog()
{
	set_default_palette(*get_colorsel() );
}

Gobby::ColorSelectionDialog::ColorSelectionDialog(const Glib::ustring& title)
 : Gtk::ColorSelectionDialog(title)
{
	set_default_palette(*get_colorsel() );
}

Gobby::ColorSelectionDialog::~ColorSelectionDialog()
{
}

Gobby::ColorButton::ColorButton()
 : Gtk::ColorButton()
{
}

Gobby::ColorButton::ColorButton(const Gdk::Color& color)
 : Gtk::ColorButton(color)
{
}

Gobby::ColorButton::~ColorButton()
{
}

void Gobby::ColorButton::on_clicked()
{
	ColorSelectionDialog dlg;
	if(dlg.run() == Gtk::RESPONSE_OK)
	{
		set_color(dlg.get_colorsel()->get_current_color() );
		set_alpha(dlg.get_colorsel()->get_current_alpha() );
	}
}

