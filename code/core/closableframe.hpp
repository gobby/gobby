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

#ifndef _GOBBY_CLOSABLE_FRAME_HPP_
#define _GOBBY_CLOSABLE_FRAME_HPP_

#include "core/preferences.hpp"

#include <gtkmm/frame.h>
#include <gtkmm/grid.h>

namespace Gobby
{

class ClosableFrame: public Gtk::Frame
{
public:
	ClosableFrame(const Glib::ustring& title,
	              const Glib::ustring& icon_name,
	              Preferences::Option<bool>& option);

	void set_allow_visible(bool allow_visible);

protected:
	virtual void on_add(Gtk::Widget* widget);

	void on_clicked();
	void on_option();

	Preferences::Option<bool>& m_option;
	Gtk::Grid m_grid;
	bool m_allow_visible;
};

}
	
#endif // _GOBBY_CLOSABLE_FRAME_HPP_
