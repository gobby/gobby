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

#ifndef _GOBBY_DEFAULTDIALOG_HPP_
#define _GOBBY_DEFAULTDIALOG_HPP_

#include <gtkmm/dialog.h>

namespace Gobby
{

/** A dialog that emits RESPONSE_OK on Enter and RESPONSE_CANCEL on Escape.
 */

class DefaultDialog : public Gtk::Dialog
{
public:
	DefaultDialog(const Glib::ustring& title, Gtk::Window& parent,
	              bool modal = false, bool use_separator = false);
	DefaultDialog(const Glib::ustring& title, bool modal = false,
	              bool use_separator = false);
	DefaultDialog();

	void set_response_sensitive(int response_id, bool sensitive);
protected:
	virtual bool on_key_press_event(GdkEventKey* event);

	bool m_ok_sensitive;
};

}

#endif // _GOBBY_DEFAULTDIALOG_HPP_
