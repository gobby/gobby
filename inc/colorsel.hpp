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

#ifndef _GOBBY_COLORSEL_HPP_
#define _GOBBY_COLORSEL_HPP_

#include <gtkmm/table.h>
#include <gtkmm/colorselection.h>
#include <gtkmm/colorbutton.h>
#include "config.hpp"

namespace Gobby
{

/** ColorSelection with custom palette.
 */
	
class ColorSelection : public Gtk::ColorSelection
{
public:
	ColorSelection(Config::ParentEntry& config_entry);
	~ColorSelection();

protected:
	Config::ParentEntry& m_config_entry;
};

/** ColorSelectionDialog with custom palette.
 */
class ColorSelectionDialog : public Gtk::ColorSelectionDialog
{
public:
	ColorSelectionDialog(Config::ParentEntry& config_entry);
	ColorSelectionDialog(Config::ParentEntry& config_entry,
	                     const Glib::ustring& title);
	~ColorSelectionDialog();

protected:
	Config::ParentEntry& m_config_entry;
};

/** ColorButton with custom palette.
 */
class ColorButton : public Gtk::ColorButton
{
public:
	ColorButton(Config::ParentEntry& config_entry);
	ColorButton(Config::ParentEntry& config_entry,
	            const Gdk::Color& color);
	~ColorButton();

protected:
	virtual void on_clicked();

	Config::ParentEntry& m_config_entry;
};

}

#endif // _GOBBY_COLORSEL_HPP_
