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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef _GOBBY_FILE_COMMANDS_HPP_
#define _GOBBY_FILE_COMMANDS_HPP_

#include "operations/operations.hpp"
#include "dialogs/documentlocationdialog.hpp"
#include "core/header.hpp"
#include "core/browser.hpp"

#include <gtkmm/window.h>
#include <gtkmm/filechooserdialog.h>
#include <sigc++/trackable.h>

namespace Gobby
{

class FileCommands: public sigc::trackable
{
public:
	FileCommands(Gtk::Window& parent, Header& header,
	             const Browser& browser, Folder& folder,
	             Operations& operations);

protected:
	enum Mode {
		MODE_NEW,
		MODE_OPEN,
		MODE_SAVE
	};

	void create_file_dialog();
	void create_location_dialog();

	void on_new();
	void on_open();

	void on_file_dialog_response(int id);
	void on_location_dialog_response(int id);

	Gtk::Window& m_parent;
	const Browser& m_browser;
	Folder& m_folder;
	Operations& m_operations;

	std::string m_open_uri;

	Mode m_mode;
	std::auto_ptr<DocumentLocationDialog> m_location_dialog;
	std::auto_ptr<Gtk::FileChooserDialog> m_file_dialog;
};

}
	
#endif // _GOBBY_FILE_COMMANDS_HPP_
