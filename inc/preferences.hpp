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

#ifndef _GOBBY_PREFERENCES_HPP_
#define _GOBBY_PREFERENCES_HPP_

#include "config.hpp"

namespace Gobby
{

class Preferences
{
public:
	/** Uninitialised preferences.
	 */
	Preferences();

	/** Reads preferences values out of a config.
	 */
	Preferences(Config& m_config);

	/** Copies preferences.
	 */
	Preferences(const Preferences& other);
	~Preferences();

	/** Copies preferences.
	 */
	Preferences& operator=(const Preferences& other);

	/** Serialises preferences back to config.
	 */
	void serialise(Config& config);

	struct
	{
		unsigned int tab_width;
		bool tab_spaces;
		bool indentation_auto;
	} editor;

	struct
	{
		bool wrap_text;
		bool wrap_words;
		bool linenum_display;
		bool curline_highlight;
		bool margin_display;
		unsigned int margin_pos;
		bool bracket_highlight;
	} view;
};

}

#endif // _GOBBY_PREFERENCES_HPP_

