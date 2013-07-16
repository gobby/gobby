/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2013 Armin Burgmeier <armin@arbur.net>
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

#ifndef _GOBBY_ENTRYDIALOG_HPP_
#define _GOBBY_ENTRYDIALOG_HPP_

#include <gtkmm/dialog.h>
#include <gtkmm/label.h>
#include <gtkmm/entry.h>
#include <gtkmm/box.h>

namespace Gobby
{

class EntryDialog: public Gtk::Dialog
{
public:
	EntryDialog(Gtk::Window& parent, const Glib::ustring& title,
	            const Glib::ustring& intro_text);

	Glib::ustring get_text() const;
	void set_text(const Glib::ustring& text);

protected:
	virtual void on_show();

	Gtk::HBox m_box;
	Gtk::Label m_intro_label;
	Gtk::Entry m_entry;
};

}

#endif // _GOBBY_ENTRYDIALOG_HPP_

