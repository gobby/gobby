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

#ifndef _GOBBY_BROWSER_COMMANDS_HPP_
#define _GOBBY_BROWSER_COMMANDS_HPP_

#include "core/documentinfostorage.hpp"
#include "core/browser.hpp"
#include "core/folder.hpp"
#include "core/statusbar.hpp"

#include <libinfinity/client/infc-browser.h>
#include <libinfinity/client/infc-request.h>
#include <libinfinity/client/infc-node-request.h>

namespace Gobby
{

class BrowserCommands: public sigc::trackable
{
public:
	BrowserCommands(Browser& browser, Folder& folder,
	                DocumentInfoStorage& info_storage,
	                StatusBar& status_bar,
	                const Preferences& preferences);
	~BrowserCommands();

protected:
	static void on_set_browser_static(InfGtkBrowserModel* model,
	                                  GtkTreePath* path,
	                                  GtkTreeIter* iter,
	                                  InfcBrowser* browser,
	                                  gpointer user_data)
	{
		static_cast<BrowserCommands*>(user_data)->on_set_browser(
			model, iter, browser);
	}

	static void on_subscribe_session_static(InfcBrowser* browser,
	                                        InfcBrowserIter* iter,
	                                        InfcSessionProxy* proxy,
	                                        gpointer user_data)
	{
		static_cast<BrowserCommands*>(user_data)->
			on_subscribe_session(browser, iter, proxy);
	}

	static void on_finished_static(InfcNodeRequest* request,
	                               const InfcBrowserIter* iter,
	                               gpointer user_data)
	{
		static_cast<BrowserCommands*>(user_data)->on_finished(
			request);
	}

	static void on_failed_static(InfcRequest* request,
	                             const GError* error,
	                             gpointer user_data)
	{
		static_cast<BrowserCommands*>(user_data)->on_failed(
			INFC_NODE_REQUEST(request), error);
	}

	static void on_synchronization_failed_static(InfSession* session,
	                                             InfXmlConnection* conn,
	                                             const GError* error,
						     gpointer user_data)
	{
		static_cast<BrowserCommands*>(user_data)->
			on_synchronization_failed(session, conn, error);
	}

	static void on_synchronization_complete_static(InfSession* session,
	                                               InfXmlConnection* conn,
	                                               gpointer user_data)
	{
		static_cast<BrowserCommands*>(user_data)->
			on_synchronization_complete(session, conn);
	}

	static void on_synchronization_progress_static(InfSession* session,
	                                               InfXmlConnection* conn,
	                                               gdouble percentage,
	                                               gpointer user_data)
	{
		static_cast<BrowserCommands*>(user_data)->
			on_synchronization_progress(
				session, conn, percentage);
	}

	static void on_notify_connection_static(GObject* object,
	                                        GParamSpec* pspec,
	                                        gpointer user_data)
	{
		static_cast<BrowserCommands*>(user_data)->
			on_notify_connection(INFC_SESSION_PROXY(object));
	}

	static void on_close_static(InfSession* session,
	                            gpointer user_data)
	{
		static_cast<BrowserCommands*>(user_data)->on_close(session);
	}

	static void on_user_join_failed_static(InfcUserRequest* request,
	                                       const GError* error,
                                               gpointer user_data)
	{
		static_cast<BrowserCommands*>(user_data)->
			on_user_join_failed(request, error);
	}

	static void on_user_join_finished_static(InfcUserRequest* request,
	                                         InfUser* user,
                                                 gpointer user_data)
	{
		static_cast<BrowserCommands*>(user_data)->
			on_user_join_finished(request, user);
	}

	void on_set_browser(InfGtkBrowserModel* model, GtkTreeIter* iter,
	                    InfcBrowser* browser);

	void on_activate(InfcBrowser* browser, InfcBrowserIter* iter);
	void on_finished(InfcNodeRequest* request);
	void on_failed(InfcNodeRequest* request, const GError* error);

	void on_subscribe_session(InfcBrowser* browser, InfcBrowserIter* iter,
	                          InfcSessionProxy* proxy);

	void on_synchronization_failed(InfSession* session,
	                               InfXmlConnection* connection,
	                               const GError* error);
	void on_synchronization_complete(InfSession* session,
	                                 InfXmlConnection* connection);
	void on_synchronization_progress(InfSession* session,
	                                 InfXmlConnection* connection,
	                                 gdouble percentage);
	void on_close(InfSession* session);
	void on_notify_connection(InfcSessionProxy* proxy);

	void join_user(InfcSessionProxy* proxy);

	void on_user_join_failed(InfcUserRequest* request,
	                         const GError* error);
	void on_user_join_finished(InfcUserRequest* request, InfUser* user);
	void user_joined(InfcSessionProxy* proxy, InfUser* user);

	Browser& m_browser;
	Folder& m_folder;
	DocumentInfoStorage& m_info_storage;
	StatusBar& m_status_bar;
	const Preferences& m_preferences;

	gulong m_set_browser_handler;

	struct RequestNode {
		BrowserCommands* commands;
		InfcBrowser* browser;
		StatusBar::MessageHandle handle;

		RequestNode();
		RequestNode(const RequestNode& node);
		~RequestNode();
	};

	struct SessionNode {
		InfcSessionProxy* proxy;
		InfSessionStatus status;

		gulong notify_connection_id;
		gulong failed_id;
		gulong complete_id;
		gulong progress_id;
		gulong close_id;

		SessionNode();
		SessionNode(const SessionNode& node);
		~SessionNode();
	};

	struct BrowserNode;

	typedef std::map<InfcBrowser*, BrowserNode*> BrowserMap;
	BrowserMap m_browser_map;

	typedef std::map<InfcNodeRequest*, RequestNode> RequestMap;
	RequestMap m_request_map;

	typedef std::map<InfSession*, SessionNode> SessionMap;
	SessionMap m_session_map;
};

}
	
#endif // _GOBBY_BROWSER_COMMANDS_HPP_
