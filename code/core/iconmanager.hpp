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

#ifndef _GOBBY_ICON_HPP_
#define _GOBBY_ICON_HPP_

#include <gtkmm/stockid.h>
#include <gdkmm/pixbuf.h>
#include <gtkmm/iconset.h>
#include <gtkmm/iconfactory.h>

#include "util/gtk-compat.hpp"

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
		GtkCompat::IconSet m_is_save_all;
		GtkCompat::IconSet m_is_userlist;
		GtkCompat::IconSet m_is_doclist;
		GtkCompat::IconSet m_is_chat;
		GtkCompat::IconSet m_is_user_color_indicator;

		Glib::RefPtr<Gtk::IconFactory> m_icon_factory;
	};
}

#endif // _GOBBY_ICON_HPP_
