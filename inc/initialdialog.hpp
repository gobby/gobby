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

#ifndef _GOBBY_INITIALDIALOG_HPP_
#define _GOBBY_INITIALDIALOG_HPP_

#include <gtkmm/dialog.h>
#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/entry.h>
#include <gtkmm/table.h>
#include <gtkmm/colorbutton.h>
#include "preferences.hpp"
#include "icon.hpp"

namespace Gobby
{

class InitialDialog : public Gtk::Dialog
{
public:
	InitialDialog(Gtk::Window& parent,
	              Preferences& preferences,
	              const IconManager& icon_manager);

protected:
	virtual void on_response(int id);

	Preferences& m_preferences;

	Gtk::VBox m_topbox;
	Gtk::Label m_title;

	Gtk::HBox m_hbox;
	Gtk::Image m_image;

	Gtk::VBox m_vbox;
	Gtk::Label m_intro;

	Gtk::Table m_table;
	Gtk::Label m_name_label;
	Gtk::Entry m_name_entry;

	Gtk::Label m_color_label;
	Gtk::ColorButton m_color_button;
};

}

#endif // _GOBBY_INITIALDIALOG_HPP_

