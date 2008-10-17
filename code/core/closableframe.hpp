/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008 Armin Burgmeier <armin@arbur.net>
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

#ifndef _GOBBY_CLOSABLE_FRAME_HPP_
#define _GOBBY_CLOSABLE_FRAME_HPP_

#include "core/preferences.hpp"

#include <gtkmm/frame.h>
#include <gtkmm/box.h>

namespace Gobby
{

class ClosableFrame: public Gtk::Frame
{
public:
	ClosableFrame(const Glib::ustring& title,
	              const Gtk::StockID& stock_id,
	              Preferences::Option<bool>& option);

protected:
	virtual void on_add(Gtk::Widget* widget);

	void on_clicked();
	void on_option();

	Preferences::Option<bool>& m_option;
	Gtk::VBox m_box;
};

}
	
#endif // _GOBBY_CLOSABLE_FRAME_HPP_
