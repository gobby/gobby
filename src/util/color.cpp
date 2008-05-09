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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "util/color.hpp"

namespace
{
	void rgb_to_hsv (double *r, double *g, double *b)
	{
		double red, green, blue;
		double h, s, v;
		double min, max;
		double delta;

		red = *r;
		green = *g;
		blue = *b;

		h = 0.0;

		if (red > green)
		{
			if (red > blue)
				max = red;
			else
				max = blue;

			if (green < blue)
				min = green;
			else
				min = blue;
		}
		else
		{
			if (green > blue)
				max = green;
			else
				max = blue;

			if (red < blue)
				min = red;
			else
				min = blue;
		}

		v = max;

		if (max != 0.0)
			s = (max - min) / max;
		else
			s = 0.0;

		if (s == 0.0)
			h = 0.0;
		else
		{
			delta = max - min;

			if (red == max)
				h = (green - blue) / delta;
			else if (green == max)
				h = 2 + (blue - red) / delta;
			else if (blue == max)
				h = 4 + (red - green) / delta;

			h /= 6.0;

			if (h < 0.0)
				h += 1.0;
			else if (h > 1.0)
				h -= 1.0;
		}

		*r = h;
		*g = s;
		*b = v;
	}
}

namespace Gobby
{
	double hue_from_gdk_color(const Gdk::Color& color)
	{
		double r = color.get_red() / 65536.0;
		double g = color.get_green() / 65536.0;
		double b = color.get_blue() / 65536.0;

		rgb_to_hsv(&r, &g, &b);
		return r;
	}
}
