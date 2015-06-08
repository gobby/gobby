/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2015 Armin Burgmeier <armin@arbur.net>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "util/color.hpp"

namespace
{
	void hsv_to_rgb(double* h, double* s, double* v)
	{
		double hue, saturation, value;
		double f, p, q, t;

		if (*s == 0.0)
		{
			*h = *v;
			*s = *v;
			*v = *v; /* heh */
		}
		else
		{
			hue = *h * 6.0;
			saturation = *s;
			value = *v;

			if (hue == 6.0)
				hue = 0.0;
      
			f = hue - (int) hue;
			p = value * (1.0 - saturation);
			q = value * (1.0 - saturation * f);
			t = value * (1.0 - saturation * (1.0 - f));

			switch ((int) hue)
			{
			case 0:
				*h = value;
				*s = t;
				*v = p;
				break;
			case 1:
				*h = q;
				*s = value;
				*v = p;
				break;
			case 2:
				*h = p;
				*s = value;
				*v = t;
				break;
			case 3:
				*h = p;
				*s = q;
				*v = value;
				break;

			case 4:
				*h = t;
				*s = p;
				*v = value;
				break;

			case 5:
				*h = value;
				*s = p;
				*v = q;
				break;

			default:
				g_assert_not_reached ();
				break;
			}
		}
	}

	void rgb_to_hsv(double* r, double* g, double* b)
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
	void hsv_to_rgb(double& rh, double &gs, double &gv)
	{
		::hsv_to_rgb(&rh, &gs, &gv);
	}

	void rgb_to_hsv(double &rh, double &gs, double &gv)
	{
		::rgb_to_hsv(&rh, &gs, &gv);
	}

	double hue_from_gdk_color(const Gdk::Color& color)
	{
		double r = color.get_red() / 65535.0;
		double g = color.get_green() / 65535.0;
		double b = color.get_blue() / 65535.0;

		::rgb_to_hsv(&r, &g, &b);
		return r;
	}

	Gdk::Color hue_to_gdk_color(double hue, double saturation,
	                            double value)
	{
		::hsv_to_rgb(&hue, &saturation, &value);
		Gdk::Color color;
		color.set_red(static_cast<gushort>(hue * 65535.0));
		color.set_green(static_cast<gushort>(saturation * 65535.0));
		color.set_blue(static_cast<gushort>(value * 65535.0));
		return color;
	}
}
