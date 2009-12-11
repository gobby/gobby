/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008, 2009 Armin Burgmeier <armin@arbur.net>
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

#ifndef _GOBBY_AUTH_COMMANDS_HPP_
#define _GOBBY_AUTH_COMMANDS_HPP_

#include "core/browser.hpp"
#include "core/preferences.hpp"

#include <gsasl.h>

#include <gtkmm/window.h>
#include <sigc++/trackable.h>

namespace Gobby
{

class AuthCommands: public sigc::trackable
{
public:
	AuthCommands(Gtk::Window& parent, Browser& browser,
	             const Preferences& preferences);

	~AuthCommands();

protected:
	static int gsasl_callback_static(Gsasl* ctx,
	                                 Gsasl_session* session,
	                                 Gsasl_property prop)
	{
		AuthCommands* auth = static_cast<AuthCommands*>(
			gsasl_callback_hook_get(ctx));
		return auth->gsasl_callback(session, prop);
	}

	int gsasl_callback(Gsasl_session* session,
	                   Gsasl_property prop);

	Gtk::Window& m_parent;
	Browser& m_browser;
	const Preferences& m_preferences;
	Gsasl* m_gsasl;
};

}

#endif // _GOBBY_AUTH_COMMANDS_HPP_
