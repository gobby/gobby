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

#ifndef _GOBBY_TITLEBAR_HPP_
#define _GOBBY_TITLEBAR_HPP_

#include "core/folder.hpp"
#include "core/sessionview.hpp"

#include <gtkmm/window.h>
#include <sigc++/trackable.h>

namespace Gobby
{

class TitleBar: public sigc::trackable
{
public:
	TitleBar(Gtk::Window& window, Folder& folder);

private:
	static void on_notify_status_static(GObject* object,
	                                    GParamSpec* pspec,
	                                    gpointer user_data)
	{
		static_cast<TitleBar*>(user_data)->on_notify_status();
	}

	static void on_notify_modified_static(InfBuffer* buffer,
	                                      GParamSpec* pspec,
	                                      gpointer user_data)
	{
		static_cast<TitleBar*>(user_data)->on_notify_modified();
	}

	void on_document_removed(SessionView& view);
	void on_document_changed(SessionView* view);

	void on_notify_status();
	void on_notify_modified();

	void update_title();

	Gtk::Window& m_window;
	Folder& m_folder;
	SessionView* m_current_view;

	gulong m_notify_status_handler;
	gulong m_modified_changed_handler;
};

}

#endif // _GOBBY_TITLEBAR_HPP_
