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

#include "features.hpp"
#include "core/preferences.hpp"

Gobby::Preferences::User::User(Config::ParentEntry& entry):
	name(entry.get_value<Glib::ustring>("name", Glib::get_user_name())),
	hue(entry.get_value<double>("hue", Glib::Rand().get_double())),
	host_directory(entry.get_value<std::string>("host-directory", Glib::build_filename(Glib::get_home_dir(), ".infinote")))
{
}

void Gobby::Preferences::User::serialise(Config::ParentEntry& entry) const
{
	entry.set_value("name", name);
	entry.set_value("hue", hue);
	entry.set_value("host-directory", host_directory);
}

Gobby::Preferences::Editor::Editor(Config::ParentEntry& entry):
	tab_width(entry.get_value<unsigned int>("tab_width", 8)),
	tab_spaces(entry.get_value<bool>("tab_insert_spaces", false)),
	indentation_auto(entry.get_value<bool>("auto_indentation", true)),
	homeend_smart(entry.get_value<bool>("smart_homeend", false) )
{
}

void Gobby::Preferences::Editor::serialise(Config::ParentEntry& entry) const
{
	entry.set_value("tab_width", tab_width);
	entry.set_value("tab_insert_spaces", tab_spaces);
	entry.set_value("auto_indentation", indentation_auto);
	entry.set_value("smart_homeend", homeend_smart);
}

Gobby::Preferences::View::View(Config::ParentEntry& entry):
	wrap_mode(
		static_cast<Gtk::WrapMode>(
			entry.get_value<int>(
				"wrap_mode",
				static_cast<int>(Gtk::WRAP_WORD_CHAR)
			)
		)
	),
	linenum_display(entry.get_value<bool>("display_line_numbers", true)),
	curline_highlight(
		entry.get_value<bool>("highlight_current_line", true)
	),
	margin_display(entry.get_value<bool>("margin_display", true) ),
	margin_pos(entry.get_value<unsigned int>("margin_position", 80) ),
	bracket_highlight(
		entry.get_value<bool>("highlight_matching_brackets", true)
	)
{
}

void Gobby::Preferences::View::serialise(Config::ParentEntry& entry) const
{
	entry.set_value("wrap_mode", static_cast<int>(wrap_mode));
	entry.set_value("display_line_numbers", linenum_display);
	entry.set_value("highlight_current_line", curline_highlight);
	entry.set_value("margin_display", margin_display);
	entry.set_value("margin_position", margin_pos);
	entry.set_value("highlight_matching_brackets", bracket_highlight);
}

Gobby::Preferences::Appearance::Appearance(Config::ParentEntry& entry):
	toolbar_style(
		static_cast<Gtk::ToolbarStyle>(
			entry.get_value<int>(
				"toolbar_style",
				// TODO: Get default value from gconf
				static_cast<int>(Gtk::TOOLBAR_BOTH)
			)
		)
	),
	font(Pango::FontDescription(entry.get_value<Glib::ustring>(
		"font", "Monospace 10"))),
	show_toolbar(entry.get_value<bool>("show_toolbar", true)),
	show_statusbar(entry.get_value<bool>("show_statusbar", true))
{
}

void Gobby::Preferences::Appearance::
	serialise(Config::ParentEntry& entry) const
{
	entry.set_value("toolbar_style", static_cast<int>(toolbar_style) );

	entry.set_value(
		"font",
		static_cast<const Pango::FontDescription&>(font).to_string()
	);

	entry.set_value("show_toolbar", show_toolbar);
	entry.set_value("show_statusbar", show_statusbar);
}

Gobby::Preferences::Preferences(Config& config):
	user(config.get_root()["user"]),
	editor(config.get_root()["editor"]),
	view(config.get_root()["view"]),
	appearance(config.get_root()["appearance"])
{
}

void Gobby::Preferences::serialise(Config& config) const
{
	// Serialise into config
	user.serialise(config.get_root()["user"]);
	editor.serialise(config.get_root()["editor"]);
	view.serialise(config.get_root()["view"]);
	appearance.serialise(config.get_root()["appearance"]);
}

