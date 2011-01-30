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

#include "dialogs/initial-dialog.hpp"

#include "util/color.hpp"
#include "util/i18n.hpp"
#include "util/gtk-compat.hpp"
#include "features.hpp"

#include <gtkmm/stock.h>
#include <gtkmm/alignment.h>

namespace
{
	Gtk::Widget* align_top(Gtk::Widget& widget)
	{
		Gtk::Alignment* alignment =
			new Gtk::Alignment(Gtk::ALIGN_CENTER,
			                   Gobby::GtkCompat::ALIGN_TOP,
			                   1.0, 0.0);
		alignment->add(widget);
		alignment->show();
		return Gtk::manage(alignment);
	}
}

Gobby::InitialDialog::InitialDialog(Gtk::Window& parent,
                                    Preferences& preferences,
                                    const IconManager& icon_manager):
	Gtk::Dialog("Gobby", parent),
	m_preferences(preferences),
	m_hbox(false, 12),
	m_vbox(false, 12),
	m_table(2, 2),
	m_color_button(_("Choose a user color"), *this)
{
	m_title.set_markup(
		"<span size=\"x-large\" weight=\"bold\">" +
		Glib::Markup::escape_text(_("Welcome to Gobby")) +
		"</span>");
	m_title.show();

	m_image.set_from_icon_name("gobby-0.5", Gtk::ICON_SIZE_DIALOG);
	m_image.set_alignment(Gtk::ALIGN_CENTER, GtkCompat::ALIGN_TOP);
	m_image.show();

	m_intro.set_text(
		_("Before we start, a few options need to be configured. "
		  "You can later change them by choosing Edit ▸ Preferences "
		  "from the menu."));
	m_intro.set_line_wrap(true);
	m_intro.show();

	m_name_label.set_markup(
		"<b>" + Glib::Markup::escape_text(_("User Name")) + "</b>"
		"<small>\n\n" +
		Glib::Markup::escape_text(_("Your name as shown to "
		                            "other users.")) +
		"</small>");
	m_name_label.set_alignment(GtkCompat::ALIGN_LEFT);
	m_name_label.set_line_wrap(true);
	m_name_label.set_width_chars(20);
	m_name_label.show();
	
	m_color_label.set_markup(
		"<b>" + Glib::Markup::escape_text(_("User Color")) + "</b>"
		"<small>\n\n" +
		Glib::Markup::escape_text(_("The color with which text you "
		                            "have written is branded.")) +
		"</small>");
	m_color_label.set_alignment(GtkCompat::ALIGN_LEFT);
	m_color_label.set_line_wrap(true);
	m_color_label.set_width_chars(20);
	m_color_label.show();

	m_name_entry.set_text(preferences.user.name);
	m_name_entry.set_activates_default(true);
	m_name_entry.show();

	m_color_button.set_hue(preferences.user.hue);
	m_color_button.set_saturation(0.35);
	m_color_button.set_value(1.0);
	m_color_button.show();

	m_table.set_row_spacings(12);

	m_table.attach(m_name_label, 0, 1, 0, 1,
	               Gtk::FILL | Gtk::SHRINK, Gtk::FILL | Gtk::SHRINK);
	m_table.attach(*align_top(m_name_entry), 1, 2, 0, 1,
	               Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::SHRINK);
	m_table.attach(m_color_label, 0, 1, 1, 2,
	               Gtk::FILL | Gtk::SHRINK, Gtk::FILL | Gtk::SHRINK);
	m_table.attach(*align_top(m_color_button), 1, 2, 1, 2,
	               Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::SHRINK);
	m_table.show();

	m_vbox.pack_start(m_intro, Gtk::PACK_SHRINK);
	m_vbox.pack_start(m_table, Gtk::PACK_EXPAND_WIDGET);
	m_vbox.show();

	m_hbox.pack_start(m_image, Gtk::PACK_SHRINK);
	m_hbox.pack_start(m_vbox, Gtk::PACK_EXPAND_WIDGET);
	m_hbox.show();

	m_topbox.pack_start(m_title, Gtk::PACK_SHRINK);
	m_topbox.pack_start(m_hbox, Gtk::PACK_EXPAND_WIDGET);
	m_topbox.set_spacing(24);
	m_topbox.set_border_width(12);
	m_topbox.show();

	get_vbox()->pack_start(m_topbox, Gtk::PACK_EXPAND_WIDGET);

	add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CLOSE);
}

void Gobby::InitialDialog::on_response(int id)
{
	m_preferences.user.name = m_name_entry.get_text();
	m_preferences.user.hue =
		hue_from_gdk_color(m_color_button.get_color());
	hide();
}
