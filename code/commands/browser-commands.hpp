/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2014 Armin Burgmeier <armin@arbur.net>
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

#ifndef _GOBBY_BROWSER_COMMANDS_HPP_
#define _GOBBY_BROWSER_COMMANDS_HPP_

#include "operations/operations.hpp"
#include "core/browser.hpp"
#include "core/statusbar.hpp"

#include <sigc++/trackable.h>

namespace Gobby
{

class BrowserCommands: public sigc::trackable
{
public:
	BrowserCommands(Browser& browser, Folder& folder,
	                StatusBar& status_bar, Operations& operations);
	~BrowserCommands();

protected:
	static void
	on_set_browser_static(InfGtkBrowserModel* model,
	                      GtkTreePath* path,
	                      GtkTreeIter* iter,
	                      InfBrowser* old_browser,
	                      InfBrowser* new_browser,
	                      gpointer user_data)
	{
		static_cast<BrowserCommands*>(user_data)->
			on_set_browser(model, iter, old_browser, new_browser);
	}

	static void
	on_subscribe_chat_finished_static(InfcChatRequest* request,
	                                  const GError* error,
	                                  gpointer user_data)
	{
		static_cast<BrowserCommands*>(user_data)->
			on_finished(INF_REQUEST(request), NULL, error);
	}

	static void
	on_subscribe_node_finished_static(InfNodeRequest* request,
	                                  const InfBrowserIter* iter,
	                                  const GError* error,
	                                  gpointer user_data)
	{
		static_cast<BrowserCommands*>(user_data)->
			on_finished(INF_REQUEST(request), iter, error);
	}

	void on_set_browser(InfGtkBrowserModel* model, GtkTreeIter* iter,
	                    InfBrowser* old_browser, InfBrowser* new_browser);
	void on_notify_status(InfBrowser* browser);

	void subscribe_chat(InfcBrowser* browser);

	void on_connect(const Glib::ustring& hostname);
	void on_activate(InfBrowser* browser, InfBrowserIter* iter);

	void on_finished(InfRequest* request,
	                 const InfBrowserIter* iter,
	                 const GError* error);

	Browser& m_browser;
	Folder& m_folder;
	StatusBar& m_status_bar;
	Operations& m_operations;
	gulong m_set_browser_handler;

	class BrowserInfo;
	typedef std::map<InfBrowser*, BrowserInfo*> BrowserMap;
	BrowserMap m_browser_map;

	class RequestInfo;
	typedef std::map<InfRequest*, RequestInfo*> RequestMap;
	RequestMap m_request_map;
};

}

#endif // _GOBBY_BROWSER_COMMANDS_HPP_
