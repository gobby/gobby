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
 * Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#ifndef _GOBBY_OPENLOCATIONDIALOG_HPP_
#define _GOBBY_OPENLOCATIONDIALOG_HPP_

#include <gtkmm/dialog.h>
#include <gtkmm/label.h>
#include <gtkmm/box.h>

#include "util/historyentry.hpp"

namespace Gobby
{

class OpenLocationDialog: public Gtk::Dialog
{
public:
	OpenLocationDialog(Gtk::Window& parent);

	Glib::ustring get_uri() const;
	void set_uri(const Glib::ustring& uri);

protected:
	virtual void on_response(int response_id);
	virtual void on_show();

	void on_entry_changed();

	Gtk::VBox m_box;
	Gtk::Label m_label;
	HistoryComboBoxEntry m_combo;
};

}

#endif // _GOBBY_OPENLOCATIONDIALOG_HPP_

