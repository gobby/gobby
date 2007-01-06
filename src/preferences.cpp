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

#include "preferences.hpp"

Gobby::Preferences::Preferences()
{
	// Uninitialised preferences
}

Gobby::Preferences::Preferences(Config& config)
{
	// Read preferences from config
	editor.tab_width =
		config["editor"]["tab"]["width"].get<unsigned int>(8);
	editor.tab_spaces = config["editor"]["tab"]["spaces"].get<bool>(false);
	editor.indentation_auto =
		config["editor"]["indentation"]["auto"].get<bool>(true);

	view.wrap_text = config["view"]["wrap"]["text"].get<bool>(true);
	view.wrap_words = config["view"]["wrap"]["words"].get<bool>(true);
	view.linenum_display =
		config["view"]["linenum"]["display"].get<bool>(true);
	view.curline_highlight =
		config["view"]["curline"]["highlight"].get<bool>(true);
	view.margin_display =
		config["view"]["margin"]["display"].get<bool>(true);
	view.margin_pos =
		config["view"]["margin"]["pos"].get<unsigned int>(80);
	view.bracket_highlight =
		config["view"]["bracket"]["highlight"].get<bool>(true);
}

Gobby::Preferences::Preferences(const Preferences& other)
{
	*this = other;
}

Gobby::Preferences::~Preferences()
{
}

void Gobby::Preferences::serialise(Config& config)
{
	// Serialise into config
	config["editor"]["tab"]["width"].set(editor.tab_width);
	config["editor"]["tab"]["spaces"].set(editor.tab_spaces);
	config["editor"]["indentation"]["auto"].set(editor.indentation_auto);

	config["view"]["wrap"]["text"].set(view.wrap_text);
	config["view"]["wrap"]["words"].set(view.wrap_words);
	config["view"]["linenum"]["display"].set(view.linenum_display);
	config["view"]["curline"]["highlight"].set(view.curline_highlight);
	config["view"]["margin"]["display"].set(view.margin_display);
	config["view"]["margin"]["pos"].get(view.margin_pos);
	config["view"]["bracket"]["highlight"].get(view.bracket_highlight);
}

Gobby::Preferences& Gobby::Preferences::operator=(const Preferences& other)
{
	editor = other.editor;
	view = other.view;

	return *this;
}

