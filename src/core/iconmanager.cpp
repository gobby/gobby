/* gobby - A GTKmm driven libobby client
 * Copyright (C) 2005, 2006 0x539 dev group
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

#include "core/iconmanager.hpp"
#include "util/i18n.hpp"

#include <gtkmm/stockitem.h>
#include <glib/gtypes.h>

namespace
{
	Glib::RefPtr<Gdk::Pixbuf> load_pixbuf(const char* dir, const char* file)
	{
		try
		{
			return Gdk::Pixbuf::create_from_file(
				Glib::build_filename(dir, file)
			);
		}
		catch(const Glib::FileError& e)
		{
			// Not installed
#ifdef G_OS_WIN32
			// Windows: Look on path relative to executable, as
			// installed by the installer.
			const gchar* subdir;
			if(dir == APPICON_DIR)
				subdir = "share/pixmaps";
			else
				subdir = "share/pixmaps/gobby";
			
			path = g_win32_get_package_installation_subdirectory(
				subdir);
			std::string path_str = path;
			g_free(path);

			return Gdk::Pixbuf::create_from_file(
				Glib::build_filename(path_str, file));
#else
			// Unix: Assume we run the program from the build
			// directory, where stuff is simply in ./pixmaps/
			return Gdk::Pixbuf::create_from_file(
				Glib::build_filename("pixmaps", file)
			);
#endif
		}
	}
}

Gtk::StockID Gobby::IconManager::STOCK_USERLIST("gobby-userlist");
Gtk::StockID Gobby::IconManager::STOCK_DOCLIST("gobby-doclist");
Gtk::StockID Gobby::IconManager::STOCK_SAVE_ALL("gobby-save-all");

Gobby::IconManager::IconManager():
	gobby(load_pixbuf(APPICON_DIR, "gobby.png") ),
	userlist(load_pixbuf(PIXMAPS_DIR, "userlist.png") ),
	doclist(load_pixbuf(PIXMAPS_DIR, "doclist.png") ),
	save_all(load_pixbuf(PIXMAPS_DIR, "save-all.svg")),
	m_is_userlist(userlist),
	m_is_doclist(doclist),
	m_is_save_all(save_all),
	m_icon_factory(Gtk::IconFactory::create() )
{
	Gtk::StockItem userlist_item(STOCK_USERLIST, _("User list") );
	m_icon_factory->add(STOCK_USERLIST, m_is_userlist);

	Gtk::StockItem doclist_item(STOCK_DOCLIST, _("Document list") );
	m_icon_factory->add(STOCK_DOCLIST, m_is_doclist);

	Gtk::StockItem save_all_item(STOCK_SAVE_ALL, _("Save All"));
	m_icon_factory->add(STOCK_SAVE_ALL, m_is_save_all);

	m_icon_factory->add_default();
}
