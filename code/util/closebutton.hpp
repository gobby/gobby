/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2014 Armin Burgmeier <armin@arbur.net>
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

#ifndef _GOBBY_CLOSEBUTTON_HPP_
#define _GOBBY_CLOSEBUTTON_HPP_

#include "util/gtk-compat.hpp"

#include <gtkmm/button.h>

namespace Gobby
{

class CloseButton: public Gtk::Button
{
public:
	CloseButton();

protected:
#ifdef USE_GTKMM3
	virtual void on_style_updated();
#else
	virtual void on_style_changed(
		const Glib::RefPtr<Gtk::Style>& previous_style);
#endif
};

} // namespace Gobby

#endif // _GOBBY_CLOSEBUTTON_HPP_
