/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008, 2009 Armin Burgmeier <armin@arbur.net>
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

#ifndef _GOBBY_COLORUTIL_HPP_
#define _GOBBY_COLORUTIL_HPP_

#include <gdkmm/color.h>

namespace Gobby
{
	double hsv_to_rgb(double& rh, double &gs, double &gv);
	double rgb_to_hsv(double &rh, double &gs, double &gv);

	double hue_from_gdk_color(const Gdk::Color& color);

	Gdk::Color hue_to_gdk_color(double hue, double saturation,
	                            double value);
}

#endif // _GOBBY_COLORUTIL_HPP_
