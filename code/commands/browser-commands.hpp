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

#ifndef _GOBBY_BROWSER_COMMANDS_HPP_
#define _GOBBY_BROWSER_COMMANDS_HPP_

#include "core/browser.hpp"
#include "core/statusbar.hpp"

#include <sigc++/trackable.h>

namespace Gobby
{

class BrowserCommands: public sigc::trackable
{
public:
	BrowserCommands(Browser& browser, Folder& folder,
	                StatusBar& status_bar);
	~BrowserCommands();

protected:
	static void
	on_set_browser_static(InfGtkBrowserModel* model,
	                      GtkTreePath* path,
	                      GtkTreeIter* iter,
	                      InfcBrowser* browser,
	                      gpointer user_data)
	{
		static_cast<BrowserCommands*>(user_data)->
			on_set_browser(model, iter, browser);
	}

	void on_set_browser(InfGtkBrowserModel* model, GtkTreeIter* iter,
	                    InfcBrowser* browser);
	void on_notify_status(InfcBrowser* browser);

	void subscribe_chat(InfcBrowser* browser);

	void on_activate(InfcBrowser* browser, InfcBrowserIter* iter);
	void on_finished(InfcNodeRequest* request);
	void on_failed(InfcNodeRequest* request, const GError* error);

	Browser& m_browser;
	Folder& m_folder;
	StatusBar& m_status_bar;

	gulong m_set_browser_handler;

	class BrowserInfo;
	typedef std::map<InfcBrowser*, BrowserInfo*> BrowserMap;
	BrowserMap m_browser_map;

	class RequestInfo;
	typedef std::map<InfcNodeRequest*, RequestInfo*> RequestMap;
	RequestMap m_request_map;
};

}

#endif // _GOBBY_BROWSER_COMMANDS_HPP_
