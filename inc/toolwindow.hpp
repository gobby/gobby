/* gobby - A GTKmm driven libobby client
 * Copyright (C) 2005 0x539 dev group
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

#ifndef _GOBBY_TOOLWINDOW_HPP_
#define _GOBBY_TOOLWINDOW_HPP_

#include <gtkmm/window.h>
#include <gtkmm/toggleaction.h>
#include "config.hpp"

namespace Gobby
{

/** Popup window that stays in top of the main application window and its
 * presence is toggleable by a ToggleAction.
 */
class ToolWindow: public Gtk::Window
{
public:
	ToolWindow(Gtk::Window& parent,
	           const Glib::ustring& title,
	           const Glib::RefPtr<Gtk::ToggleAction>& action,
		   Config& config,
		   const Glib::ustring& config_key);
	~ToolWindow();

protected:
	virtual void on_activate();

	virtual void on_show();
	virtual void on_hide();

	Glib::RefPtr<Gtk::ToggleAction> m_action;

	Config& m_config;
	Glib::ustring m_config_key;
};

} // namespace obby

#endif // _GOBBY_TOOLWINDOW_HPP_
