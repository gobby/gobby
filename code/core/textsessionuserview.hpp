/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2013 Armin Burgmeier <armin@arbur.net>
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

#ifndef _GOBBY_TEXTSESSIONUSERVIEW_HPP_
#define _GOBBY_TEXTSESSIENUSERVIEW_HPP_

#include "core/sessionuserview.hpp"
#include "core/textsessionview.hpp"
#include "core/preferences.hpp"
#include "core/userlist.hpp"

// Allows a user in the user list to be double-clicked at, scrolling
// the text view to that user's cursor.
namespace Gobby
{

class TextSessionUserView: public SessionUserView
{
public:
	TextSessionUserView(TextSessionView& view, bool show_disconnected,
	                    Preferences::Option<bool>& userlist_view,
	                    Preferences::Option<unsigned int>& w);

	TextSessionView& get_session_view()
	{
		return static_cast<TextSessionView&>(
			SessionUserView::get_session_view());
	}

	const TextSessionView& get_session_view() const
	{
		return static_cast<const TextSessionView&>(
			SessionUserView::get_session_view());
	}

protected:
	void on_user_activated(InfUser* user);
};

}

#endif // _GOBBY_TEXTSESSIONUSERVIEW_HPP_
