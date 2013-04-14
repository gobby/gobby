/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2011 Armin Burgmeier <armin@arbur.net>
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

#include "commands/subscription-commands.hpp"
#include "util/i18n.hpp"

class Gobby::SubscriptionCommands::BrowserInfo
{
public:
	BrowserInfo(SubscriptionCommands& commands, InfBrowser* browser):
		m_browser(browser)
	{
		g_object_ref(m_browser);

		m_subscribe_session_handler = g_signal_connect(
			G_OBJECT(browser),
			"subscribe-session",
			G_CALLBACK(&on_subscribe_session_static),
			&commands);
		m_unsubscribe_session_handler = g_signal_connect(
			G_OBJECT(browser),
			"unsubscribe-session",
			G_CALLBACK(&on_unsubscribe_session_static),
			&commands);
	}

	~BrowserInfo()
	{
		g_signal_handler_disconnect(G_OBJECT(m_browser),
		                            m_subscribe_session_handler);
		g_signal_handler_disconnect(G_OBJECT(m_browser),
		                            m_unsubscribe_session_handler);

		g_object_unref(m_browser);
	}

private:
	InfBrowser* m_browser;
	gulong m_subscribe_session_handler;
	gulong m_unsubscribe_session_handler;
};

class Gobby::SubscriptionCommands::SessionInfo
{
public:
	SessionInfo(SubscriptionCommands& commands, Folder& folder,
	            InfSessionProxy* proxy):
		m_folder(folder), m_proxy(proxy)
	{
		g_object_ref(proxy);

		InfSession* session;
		g_object_get(G_OBJECT(proxy), "session", &session, NULL);

		m_notify_subscription_group_handler = g_signal_connect(
			G_OBJECT(session),
			"notify::subscription-group",
			G_CALLBACK(on_notify_subscription_group_static),
			&commands);

		g_object_unref(session);
	}

	~SessionInfo()
	{
		InfSession* session;
		g_object_get(G_OBJECT(m_proxy), "session", &session, NULL);

		g_signal_handler_disconnect(
			G_OBJECT(session),
			m_notify_subscription_group_handler);

		g_object_unref(session);
		g_object_unref(m_proxy);
	}

	Folder& get_folder() { return m_folder; }
	InfSessionProxy* get_proxy() { return m_proxy; }

private:
	static void on_notify_subscription_group_static(InfSession* session,
	                                                GParamSpec* pspec,
	                                                gpointer user_data)
	{
		static_cast<SubscriptionCommands*>(user_data)->
			on_notify_subscription_group(session);
	}

	Folder& m_folder;
	InfSessionProxy* m_proxy;

	gulong m_notify_subscription_group_handler;
};

Gobby::SubscriptionCommands::SubscriptionCommands(Browser& browser,
                                                  Folder& text_folder,
                                                  Folder& chat_folder,
                                                  DocumentInfoStorage& strg):
	m_browser(browser), m_text_folder(text_folder),
	m_chat_folder(chat_folder), m_info_storage(strg)
{
	InfGtkBrowserModel* model =
		INF_GTK_BROWSER_MODEL(browser.get_store());

	m_set_browser_handler =
		g_signal_connect(G_OBJECT(model), "set-browser",
		                 G_CALLBACK(&on_set_browser_static), this);
}

Gobby::SubscriptionCommands::~SubscriptionCommands()
{
	for(BrowserMap::iterator iter = m_browser_map.begin();
	    iter != m_browser_map.end(); ++ iter)
	{
		delete iter->second;
	}

	for(SessionMap::iterator iter = m_session_map.begin();
	    iter != m_session_map.end(); ++iter)
	{
		delete iter->second;
	}

	g_signal_handler_disconnect(
		INF_GTK_BROWSER_MODEL(m_browser.get_store()),
		m_set_browser_handler);
}

void Gobby::SubscriptionCommands::on_set_browser(InfGtkBrowserModel* model,
                                                 GtkTreeIter* iter,
                                                 InfBrowser* browser)
{
	InfBrowser* old_browser;
	gtk_tree_model_get(
		GTK_TREE_MODEL(model), iter,
		INF_GTK_BROWSER_MODEL_COL_BROWSER, &old_browser, -1);

	if(old_browser != NULL)
	{
		BrowserMap::iterator iter = m_browser_map.find(old_browser);
		g_assert(iter != m_browser_map.end());
		delete iter->second;
		m_browser_map.erase(iter);
		g_object_unref(old_browser);
	}

	if(browser != NULL)
	{
		g_assert(m_browser_map.find(browser) == m_browser_map.end());
		m_browser_map[browser] = new BrowserInfo(*this, browser);
	}
}

void Gobby::SubscriptionCommands::on_subscribe_session(
	InfBrowser* browser,
	const InfBrowserIter* iter,
	InfSessionProxy* proxy)
{
	gchar* hostname;
	g_object_get(
		G_OBJECT(infc_browser_get_connection(INFC_BROWSER(browser))),
		"remote-hostname", &hostname, NULL);
	InfSession* session;
	g_object_get(G_OBJECT(proxy), "session", &session, NULL);

	TextSessionView* text_view = NULL;

	Folder* folder;
	SessionView* view;

	if(iter != NULL)
	{
		gchar* path = inf_browser_get_path(browser, iter);

		text_view = &m_text_folder.add_text_session(
			INF_TEXT_SESSION(session),
			inf_browser_get_node_name(browser, iter),
			path, hostname,
			m_info_storage.get_key(browser, iter));

		folder = &m_text_folder;
		view = text_view;

		g_free(path);
	}
	else
	{
		view = &m_chat_folder.add_chat_session(
			INF_CHAT_SESSION(session), hostname, "", hostname);
		folder = &m_chat_folder;
	}

	g_free(hostname);

	m_signal_subscribe_session.emit(proxy, *folder, *view);

	// For now we always highlight the newly created session...
	// TODO: If the user issued other browserview events in the meanwhile,
	// then don't select the item, and if the user did issue other folder
	// events, then don't switch to the document in the folder.
	folder->switch_to_document(*view);
	if(text_view)
		gtk_widget_grab_focus(GTK_WIDGET(text_view->get_text_view()));
	if(iter) m_browser.set_selected(browser, iter);

	g_assert(m_session_map.find(session) == m_session_map.end());
	m_session_map[session] =
		new SessionInfo(*this, *folder, proxy);
	g_object_unref(session);
}

void Gobby::SubscriptionCommands::on_unsubscribe_session(
	InfBrowser* browser,
	const InfBrowserIter* iter,
	InfSessionProxy* proxy)
{
	InfSession* session;
	g_object_get(G_OBJECT(proxy), "session", &session, NULL);
	SessionMap::iterator session_iter = m_session_map.find(session);
	g_assert(session_iter != m_session_map.end());

	Folder& folder = session_iter->second->get_folder();
	SessionView* view = folder.lookup_document(session);
	g_assert(view != NULL);

	g_object_ref(proxy);
	g_object_unref(session);

	delete session_iter->second;
	m_session_map.erase(session_iter);

	m_signal_unsubscribe_session.emit(proxy, folder, *view);
	g_object_unref(proxy);
}

void Gobby::SubscriptionCommands::
	on_notify_subscription_group(InfSession* session)
{
	SessionMap::iterator iter = m_session_map.find(session);
	g_assert(iter != m_session_map.end());

	if(inf_session_get_subscription_group(session) == NULL)
	{
		Folder& folder = iter->second->get_folder();
		SessionView* view = folder.lookup_document(session);
		g_assert(view != NULL);

		TextSessionView* text_view =
			dynamic_cast<TextSessionView*>(view);
		ChatSessionView* chat_view =
			dynamic_cast<ChatSessionView*>(view);

		if(text_view)
		{
			/* If the session is in SYNCHRONIZING state then the
			 * session is closed due to a synchronization error.
			 * In that case synchronization-commands.cpp will set
			 * a more meaningful error message. */
			if(inf_session_get_status(session) ==
			   INF_SESSION_RUNNING)
			{
				view->set_info(_(
					"The connection to the publisher of "
					"this document has been lost. "
					"Further changes to the document "
					"could not be synchronized to others "
					"anymore, therefore the document "
					"cannot be edited anymore.\n\n"
					"Please note also that it is "
					"possible that not all of your "
					"latest changes have reached the "
					"publisher before the connection was "
					"lost."), true);
			}

			text_view->set_active_user(NULL);
		}
		else if(chat_view)
		{
			chat_view->set_active_user(NULL);
		}

		m_signal_unsubscribe_session.emit(
			iter->second->get_proxy(), folder, *view);
	}
}
