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
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "commands/browser-commands.hpp"
#include "util/i18n.hpp"

class Gobby::BrowserCommands::BrowserInfo
{
public:
	BrowserInfo(BrowserCommands& commands,
	            InfcBrowser* browser);

	~BrowserInfo();

	InfcBrowser* get_browser() { return m_browser; }
private:
	static void on_notify_status_static(GObject* object,
	                                    GParamSpec* pspec,
	                                    gpointer user_data)
	{
		static_cast<BrowserCommands*>(user_data)->on_notify_status(
			INFC_BROWSER(object));
	}

	InfcBrowser* m_browser;

	gulong m_notify_status_handler;
};

class Gobby::BrowserCommands::RequestInfo
{
public:
	RequestInfo(BrowserCommands& commands,
	            InfcBrowser* browser, InfcBrowserIter* iter,
	            InfcNodeRequest* request, StatusBar& status_bar);
	~RequestInfo();

private:
	static void on_failed_static(InfcNodeRequest* request,
	                             const GError* error, gpointer user_data)
	{
		static_cast<BrowserCommands*>(user_data)->
			on_failed(request, error);
	}

	static void on_finished_static(InfcNodeRequest* request,
	                               InfcBrowserIter* iter,
	                               gpointer user_data)
	{
		static_cast<BrowserCommands*>(user_data)->
			on_finished(request);
	}

	InfcNodeRequest* m_request;

	StatusBar& m_status_bar;
	StatusBar::MessageHandle m_handle;

	gulong m_failed_handler;
	gulong m_finished_handler;
};

Gobby::BrowserCommands::BrowserInfo::BrowserInfo(BrowserCommands& cmds,
                                                 InfcBrowser* browser):
	m_browser(browser)
{
	m_notify_status_handler = g_signal_connect(
		m_browser, "notify::status",
		G_CALLBACK(on_notify_status_static), &cmds);

	g_object_ref(browser);
}

Gobby::BrowserCommands::BrowserInfo::~BrowserInfo()
{
	g_signal_handler_disconnect(m_browser, m_notify_status_handler);

	g_object_unref(m_browser);
}

Gobby::BrowserCommands::RequestInfo::RequestInfo(BrowserCommands& commands,
                                                 InfcBrowser* browser,
                                                 InfcBrowserIter* iter,
                                                 InfcNodeRequest* request,
                                                 StatusBar& status_bar):
	m_request(request), m_status_bar(status_bar)
{
	g_object_ref(request);

	if(iter)
	{
		m_handle = m_status_bar.add_info_message(
			Glib::ustring::compose(
				_("Subscribing to %1..."), Glib::ustring(
					infc_browser_iter_get_name(
						browser, iter))));
	}
	else
	{
		InfXmlConnection* connection =
			infc_browser_get_connection(browser);
		gchar* remote_hostname;
		g_object_get(G_OBJECT(connection),
		             "remote-id", &remote_hostname, NULL);
		m_handle = m_status_bar.add_info_message(
			Glib::ustring::compose(
				_("Subscribing to chat on %1..."),
					remote_hostname));
		g_free(remote_hostname);
	}

	m_failed_handler = g_signal_connect(request, "failed",
	                 G_CALLBACK(on_failed_static), &commands);
	m_finished_handler = g_signal_connect(request, "finished",
	                 G_CALLBACK(on_finished_static), &commands);
}

Gobby::BrowserCommands::RequestInfo::~RequestInfo()
{
	m_status_bar.remove_message(m_handle);

	g_signal_handler_disconnect(m_request, m_failed_handler);
	g_signal_handler_disconnect(m_request, m_finished_handler);

	g_object_unref(m_request);
}

Gobby::BrowserCommands::BrowserCommands(Browser& browser,
                                        Folder& folder,
                                        StatusBar& status_bar):
	m_browser(browser), m_folder(folder), m_status_bar(status_bar)
{
	m_browser.signal_activate().connect(
		sigc::mem_fun(*this, &BrowserCommands::on_activate));

	m_set_browser_handler = g_signal_connect(
		browser.get_store(), "set-browser",
		G_CALLBACK(on_set_browser_static), this);
}

Gobby::BrowserCommands::~BrowserCommands()
{
	for(RequestMap::iterator iter = m_request_map.begin();
	    iter != m_request_map.end(); ++ iter)
	{
		delete iter->second;
	}

	for(BrowserMap::iterator iter = m_browser_map.begin();
	    iter != m_browser_map.end(); ++iter)
	{
		delete iter->second;
	}

	g_signal_handler_disconnect(m_browser.get_store(),
	                            m_set_browser_handler);
}

void Gobby::BrowserCommands::on_set_browser(InfGtkBrowserModel* model,
                                            GtkTreeIter* iter,
                                            InfcBrowser* browser)
{
	InfcBrowser* old_browser;
	gtk_tree_model_get(
		GTK_TREE_MODEL(model), iter,
		INF_GTK_BROWSER_MODEL_COL_BROWSER, &old_browser, -1);

	if(old_browser != NULL)
	{
		// Find by browser in case old_browser has it's connection
		// reset.
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
		if(infc_browser_get_status(browser) == INFC_BROWSER_CONNECTED)
			if(!infc_browser_get_chat_session(browser))
				subscribe_chat(browser);
	}
}

void Gobby::BrowserCommands::on_notify_status(InfcBrowser* browser)
{
	InfXmlConnection* connection;
	InfXmlConnectionStatus status;

	switch(infc_browser_get_status(browser))
	{
	case INFC_BROWSER_DISCONNECTED:
		// Close connection if browser got disconnected. This for
		// example happens when the server does not send an initial
		// welcome message.
		connection = infc_browser_get_connection(browser);
		g_object_get(G_OBJECT(connection), "status", &status, NULL);
		if(status != INF_XML_CONNECTION_CLOSED &&
		   status != INF_XML_CONNECTION_CLOSING)
		{
			inf_xml_connection_close(connection);
		}

		break;
	case INFC_BROWSER_CONNECTING:
		break;
	case INFC_BROWSER_CONNECTED:
		if(!infc_browser_get_chat_session(browser))
			subscribe_chat(browser);
		break;
	default:
		g_assert_not_reached();
		break;
	}
}

void Gobby::BrowserCommands::subscribe_chat(InfcBrowser* browser)
{
	InfcNodeRequest* request = infc_browser_subscribe_chat(browser);
	g_assert(m_request_map.find(request) == m_request_map.end());
	m_request_map[request] =
		new RequestInfo(*this, browser, NULL, request, m_status_bar);
}

void Gobby::BrowserCommands::on_activate(InfcBrowser* browser,
                                         InfcBrowserIter* iter)
{
	InfcSessionProxy* proxy =
		infc_browser_iter_get_session(browser, iter);
	if(proxy != NULL)
	{
		InfSession* session = infc_session_proxy_get_session(proxy);
		SessionView* view = m_folder.lookup_document(session);

		if(view != NULL)
		{
			m_folder.switch_to_document(*view);
		}
		else
		{
			// This should not happen: We insert every document
			// we subscribe to directly into the folder.
			g_assert_not_reached();
		}
	}
	else
	{
		InfcNodeRequest* request =
			infc_browser_iter_get_subscribe_request(browser,
			                                        iter);

		// If there is already a request don't re-request
		if(request == NULL)
		{
			request = infc_browser_iter_subscribe_session(browser,
			                                              iter);

			g_assert(m_request_map.find(request) ==
			         m_request_map.end());
			m_request_map[request] =
				new RequestInfo(*this, browser, iter,
				                request, m_status_bar);
		}
	}
}

void Gobby::BrowserCommands::on_finished(InfcNodeRequest* request)
{
	RequestMap::iterator iter = m_request_map.find(request);
	g_assert(iter != m_request_map.end());
	delete iter->second;
	m_request_map.erase(iter);
}

void Gobby::BrowserCommands::on_failed(InfcNodeRequest* request,
                                       const GError* error)
{
	RequestMap::iterator iter = m_request_map.find(request);
	g_assert(iter != m_request_map.end());
	delete iter->second;
	m_request_map.erase(iter);

	m_status_bar.add_error_message(
		_("Subscription failed"),
		error->message);
}
