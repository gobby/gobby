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
	const gchar* DEFAULT_GCOLOR_STRING =
		"#f9dad4:#fddfbe:#fdf3be:#f5fdbe:#dcfdbe:#befde2:#bee9fd:"
		"#becffd:#c7befd:#f2befd:#fdbedd:#f6abab:#fbc759:#fcffba:"
		"#f4ffee:#bce9b8:#eefdff:#e8ebff:#fdeeff:#dfdfdf";

	void set_palette(Gobby::Config::ParentEntry& config_entry,
	                 Gtk::ColorSelection& selection)
	{
		GtkSettings* settings =
			gtk_widget_get_settings(GTK_WIDGET(selection.gobj()) );

		Glib::ustring palette = config_entry.get_value<Glib::ustring>(
			"color-palette",
			DEFAULT_GCOLOR_STRING
			);

		g_object_set(
			settings,
			"gtk-color-palette",
			palette.c_str(),
			NULL
		);

		selection.set_has_palette(true);
	}

	void save_palette(Gobby::Config::ParentEntry& config_entry,
	                  Gtk::ColorSelection& selection)
	{
		GtkSettings* settings =
			gtk_widget_get_settings(GTK_WIDGET(selection.gobj()) );

		gchar* palette;
		g_object_get(settings, "gtk-color-palette", &palette, NULL);
		config_entry.set_value("color-palette", palette);
		g_free(palette);
	}
}

Gobby::ColorSelection::ColorSelection(Config::ParentEntry& config_entry)
 : Gtk::ColorSelection(),
   m_config_entry(config_entry)
{
	set_palette(config_entry, *this);
}

Gobby::ColorSelection::~ColorSelection()
{
	save_palette(m_config_entry, *this);
}

Gobby::ColorSelectionDialog::
	ColorSelectionDialog(Config::ParentEntry& config_entry)
 : Gtk::ColorSelectionDialog(),
   m_config_entry(config_entry)
{
	set_palette(config_entry, *get_colorsel() );
}

Gobby::ColorSelectionDialog::
	ColorSelectionDialog(Config::ParentEntry& config_entry,
	                     const Glib::ustring& title)
 : Gtk::ColorSelectionDialog(title),
   m_config_entry(config_entry)
{
	set_palette(config_entry, *get_colorsel() );
}

Gobby::ColorSelectionDialog::~ColorSelectionDialog()
{
	save_palette(m_config_entry, *get_colorsel() );
}

Gobby::ColorButton::ColorButton(Config::ParentEntry& config_entry)
 : Gtk::ColorButton(),
   m_config_entry(config_entry)
{
}

Gobby::ColorButton::ColorButton(Config::ParentEntry& config_entry,
                                const Gdk::Color& color)
 : Gtk::ColorButton(color),
   m_config_entry(config_entry)
{
}

Gobby::ColorButton::~ColorButton()
{
}

void Gobby::ColorButton::on_clicked()
{
	ColorSelectionDialog dlg(m_config_entry);
	dlg.get_colorsel()->set_current_color(get_color() );
	dlg.get_colorsel()->set_current_alpha(get_alpha() );

	if(dlg.run() == Gtk::RESPONSE_OK)
	{
		set_color(dlg.get_colorsel()->get_current_color() );
		set_alpha(dlg.get_colorsel()->get_current_alpha() );
	}
}

