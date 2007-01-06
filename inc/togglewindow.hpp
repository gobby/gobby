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

#ifndef _GOBBY_TOGGLEWINDOW_HPP_
#define _GOBBY_TOGGLEWINDOW_HPP_

#include <gtkmm/toggleaction.h>
#include "config.hpp"
#include "preferences.hpp"
#include "toolwindow.hpp"

namespace Gobby
{

/** Tool window whose visibility may be toggled via a Gtk::ToggleAction.
 */
class ToggleWindow: public ToolWindow
{
public:
	ToggleWindow(Gtk::Window& parent,
	             const Glib::RefPtr<Gtk::ToggleAction>& action,
	             const Preferences& preferences,
		     Config::ParentEntry& config_entry);
	~ToggleWindow();

protected:
	virtual void on_activate();

	virtual void on_show();
	virtual void on_hide();

	Glib::RefPtr<Gtk::ToggleAction> m_action;
	const Preferences& m_preferences;
	Config::ParentEntry& m_config_entry;
};

} // namespace obby

#endif // _GOBBY_TOGGLEWINDOW_HPP_
