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

#ifndef _GOBBY_GOTODIALOG_HPP_
#define _GOBBY_GOTODIALOG_HPP_

#include "core/toolwindow.hpp"
#include "core/folder.hpp"

#include <gtkmm/separator.h>
#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/spinbutton.h>

namespace Gobby
{

class GotoDialog: public ToolWindow
{
public:
	GotoDialog(Gtk::Window& parent, Folder& m_folder);

protected:
	virtual void on_show();
	virtual void on_goto();

	Folder& m_folder;

	Gtk::VBox m_mainbox;
	Gtk::HBox m_box_top;
	Gtk::HBox m_box_bottom;

	Gtk::Label m_lbl_info;
	Gtk::SpinButton m_ent_line;

	Gtk::HSeparator m_sep;

	Gtk::Button m_btn_close;
	Gtk::Button m_btn_goto;
};

}

#endif // _GOBBY_GOTODIALOG_HPP_
