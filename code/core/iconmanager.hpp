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

#ifndef _GOBBY_ICON_HPP_
#define _GOBBY_ICON_HPP_

#include <gtkmm/stockid.h>
#include <gdkmm/pixbuf.h>
#include <gtkmm/iconset.h>
#include <gtkmm/iconfactory.h>

namespace Gobby
{
	class IconManager
	{
	public:
		static Gtk::StockID STOCK_SAVE_ALL;
		static Gtk::StockID STOCK_USERLIST;
		static Gtk::StockID STOCK_DOCLIST;
		static Gtk::StockID STOCK_CHAT;
		static Gtk::StockID STOCK_USER_COLOR_INDICATOR;

		IconManager();

	protected:
		Glib::RefPtr<Gtk::IconSet> m_is_save_all;
		Glib::RefPtr<Gtk::IconSet> m_is_userlist;
		Glib::RefPtr<Gtk::IconSet> m_is_doclist;
		Glib::RefPtr<Gtk::IconSet> m_is_chat;
		Glib::RefPtr<Gtk::IconSet> m_is_user_color_indicator;

		Glib::RefPtr<Gtk::IconFactory> m_icon_factory;
	};
}

#endif // _GOBBY_ICON_HPP_
