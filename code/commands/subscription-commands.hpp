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

#ifndef _GOBBY_SUBSCRIPTION_COMMANDS_HPP_
#define _GOBBY_SUBSCRIPTION_COMMANDS_HPP_

#include "core/browser.hpp"
#include "core/folder.hpp"
#include "core/sessionview.hpp"
#include "core/documentinfostorage.hpp"

#include <libinfinity/client/infc-browser.h>
#include <libinfinity/client/infc-session-proxy.h>

namespace Gobby
{

class SubscriptionCommands: public sigc::trackable
{
public:
	typedef sigc::signal<void, InfcSessionProxy*, Folder&, SessionView&>
		SignalSubscribeSession;
	typedef sigc::signal<void, InfcSessionProxy*, Folder&, SessionView&>
		SignalUnsubscribeSession;

	SubscriptionCommands(Browser& browser, Folder& text_folder,
	                     Folder& chat_folder, DocumentInfoStorage& strg);
	~SubscriptionCommands();

	// Emitted whenever a session is subscribed to, both for text and
	// chat sessions. This also provides access to the InfcSessionProxy
	// of the subscription to allow others (especially user-join-commands
	// to make a user join).
	SignalSubscribeSession signal_subscribe_session() const
	{
		return m_signal_subscribe_session;
	}

	SignalUnsubscribeSession signal_unsubscribe_session() const
	{
		return m_signal_unsubscribe_session;
	}

protected:
	static void on_set_browser_static(InfGtkBrowserModel* model,
	                                  GtkTreePath* path,
	                                  GtkTreeIter* iter,
	                                  InfcBrowser* browser,
	                                  gpointer user_data)
	{
		static_cast<SubscriptionCommands*>(user_data)->on_set_browser(
			model, iter, browser);
	}

	static void on_subscribe_session_static(InfcBrowser* browser,
	                                        InfcBrowserIter* iter,
	                                        InfcSessionProxy* proxy,
	                                        gpointer user_data)
	{
		static_cast<SubscriptionCommands*>(user_data)->
			on_subscribe_session(browser, iter, proxy);
	}

	static void on_finished_static(InfcNodeRequest* request,
	                               const InfcBrowserIter* iter,
	                               gpointer user_data)
	{
		static_cast<SubscriptionCommands*>(user_data)->on_finished(
			request);
	}

	static void on_failed_static(InfcRequest* request,
	                             const GError* error,
	                             gpointer user_data)
	{
		static_cast<SubscriptionCommands*>(user_data)->on_failed(
			INFC_NODE_REQUEST(request), error);
	}

	void on_set_browser(InfGtkBrowserModel* model, GtkTreeIter* iter,
	                    InfcBrowser* browser);

	void on_finished(InfcNodeRequest* request);
	void on_failed(InfcNodeRequest* request, const GError* error);

	void on_subscribe_session(InfcBrowser* browser, InfcBrowserIter* iter,
	                          InfcSessionProxy* proxy);

	void on_close(InfSession* session);
	void on_notify_connection(InfcSessionProxy* proxy);

	Browser& m_browser;
	Folder& m_text_folder;
	Folder& m_chat_folder;
	DocumentInfoStorage& m_info_storage;

	gulong m_set_browser_handler;

	class BrowserInfo;
	typedef std::map<InfcBrowser*, BrowserInfo*> BrowserMap;
	BrowserMap m_browser_map;

	class SessionInfo;
	typedef std::map<InfSession*, SessionInfo*> SessionMap;
	SessionMap m_session_map;

	SignalSubscribeSession m_signal_subscribe_session;
	SignalUnsubscribeSession m_signal_unsubscribe_session;
};

}

#endif // _GOBBY_SUBSCRIPTION_COMMANDS_HPP_
