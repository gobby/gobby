/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2011 Armin Burgmeier <armin@arbur.net>
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

#ifndef _GOBBY_FILE_CHOOSER_HPP_
#define _GOBBY_FILE_CHOOSER_HPP_

#include <gtkmm/filechooserdialog.h>
#include <gtkmm/window.h>

namespace Gobby
{

// This class manages a common "current folder uri" for file chooser
// dialogs.
class FileChooser
{
public:
	class Dialog: public Gtk::FileChooserDialog
	{
	public:
		Dialog(Gobby::FileChooser& chooser, Gtk::Window& parent,
		       const Glib::ustring& title,
		       Gtk::FileChooserAction action);
		~Dialog();

	protected:
		Gobby::FileChooser& m_chooser;
	};

	FileChooser();

	const std::string& get_current_folder_uri() const;
	void set_current_folder_uri(const std::string& uri);

private:
	std::string m_current_folder_uri;
};

}

#endif // _GOBBY_FILE_CHOOSER_HPP_
