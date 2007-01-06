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

Gobby::Preferences::Preferences(Config& config)
 : m_config(config)
{
	editor.tab_width =
		config["editor"]["tab"]["width"].get<unsigned int>(8);
	editor.tab_spaces = config["editor"]["tab"]["spaces"].get<bool>(false);
	editor.indentation_auto =
		config["editor"]["indentation"]["auto"].get<bool>(true);

	view.wrap_text = config["editor"]["wrap"]["text"].get<bool>(true);
	view.wrap_words = config["editor"]["wrap"]["words"].get<bool>(false);
	view.linenum_display =
		config["editor"]["linenum"]["display"].get<bool>(true);
}

Gobby::Preferences::~Preferences()
{
	m_config["editor"]["tab"]["width"].set(editor.tab_width);
	m_config["editor"]["tab"]["spaces"].set(editor.tab_spaces);
	m_config["editor"]["indentation"]["auto"].set(editor.indentation_auto);

	m_config["view"]["wrap"]["text"].set(view.wrap_text);
	m_config["view"]["wrap"]["words"].set(view.wrap_words);
	m_config["view"]["linenum"]["display"].set(view.linenum_display);
}

