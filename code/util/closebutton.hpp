/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2014 Armin Burgmeier <armin@arbur.net>
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
