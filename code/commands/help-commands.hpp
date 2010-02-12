/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2010 Armin Burgmeier <armin@arbur.net>
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

#ifndef _GOBBY_HELP_COMMANDS_HPP_
#define _GOBBY_HELP_COMMANDS_HPP_

#include "core/header.hpp"
#include "core/iconmanager.hpp"

#include <gtkmm/window.h>
#include <gtkmm/aboutdialog.h>
#include <sigc++/trackable.h>

#include <memory>

namespace Gobby
{

class HelpCommands: public sigc::trackable
{
public:
	HelpCommands(Gtk::Window& parent, Header& header,
	             const IconManager& icon_manager);

protected:
	void on_contents();

	void on_about();
	void on_about_response(int response_id);

	Gtk::Window& m_parent;
	Header& m_header;
	const IconManager& m_icon_manager;

	std::auto_ptr<Gtk::AboutDialog> m_about_dialog;
};

}

#endif // _GOBBY_HELP_COMMANDS_HPP_
