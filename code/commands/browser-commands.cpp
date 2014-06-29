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

#include "commands/browser-commands.hpp"
#include "util/i18n.hpp"

#include <libinfinity/common/inf-request-result.h>

class Gobby::BrowserCommands::BrowserInfo
{
public:
	BrowserInfo(BrowserCommands& commands,
	            InfBrowser* browser);

	~BrowserInfo();

	InfBrowser* get_browser() { return m_browser; }
private:
	static void on_notify_status_static(GObject* object,
	                                    GParamSpec* pspec,
	                                    gpointer user_data)
	{
		static_cast<BrowserCommands*>(user_data)->on_notify_status(
			INF_BROWSER(object));
	}

	InfBrowser* m_browser;

	gulong m_notify_status_handler;
};

class Gobby::BrowserCommands::RequestInfo
{
	friend class BrowserCommands;
public:
	RequestInfo(BrowserCommands& commands,
	            InfBrowser* browser, InfBrowserIter* iter,
	            StatusBar& status_bar);
	~RequestInfo();

	InfBrowser* get_browser() { return m_browser; }

	void set_request(InfRequest* request);
private:
	static void on_node_finished_static(InfRequest* request,
	                                    const InfRequestResult* result,
	                                    const GError* error,
	                                    gpointer user_data)
	{
		const InfBrowserIter* iter = NULL;
		RequestInfo* info = static_cast<RequestInfo*>(user_data);

		if(error == NULL)
		{
			inf_request_result_get_subscribe_session(
				result, NULL, &iter, NULL);

			g_assert(iter == NULL ||
			         iter->node == info->m_iter.node);
			g_assert(iter == NULL ||
		        	 iter->node_id == info->m_iter.node_id);
		}

		info->m_commands.on_finished(
			INF_REQUEST(request),
			info->m_browser, iter, error);
	}
	
	static void on_chat_finished_static(InfRequest* request,
	                                    const InfRequestResult* result,
	                                    const GError* error,
	                                    gpointer user_data)
	{
		RequestInfo* info = static_cast<RequestInfo*>(user_data);

		info->m_commands.on_finished(
			INF_REQUEST(request), info->m_browser, NULL, error);
	}

	BrowserCommands& m_commands;
	InfBrowser* m_browser;
	InfBrowserIter m_iter;

	StatusBar& m_status_bar;
	StatusBar::MessageHandle m_handle;

	InfRequest* m_request;
	gulong m_finished_handler;
};

Gobby::BrowserCommands::BrowserInfo::BrowserInfo(BrowserCommands& cmds,
                                                 InfBrowser* browser):
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
                                                 InfBrowser* browser,
                                                 InfBrowserIter* iter,
                                                 StatusBar& status_bar):
	m_commands(commands), m_browser(browser), m_status_bar(status_bar),
	m_request(NULL), m_finished_handler(0)
{
	if(iter)
	{
		m_iter = *iter;

		m_handle = m_status_bar.add_info_message(
			Glib::ustring::compose(
				_("Subscribing to %1..."), Glib::ustring(
					inf_browser_get_node_name(
						browser, iter))));
	}
	else
	{
		InfXmlConnection* connection =
			infc_browser_get_connection(INFC_BROWSER(browser));
		gchar* remote_hostname;
		g_object_get(G_OBJECT(connection),
		             "remote-hostname", &remote_hostname, NULL);
		m_handle = m_status_bar.add_info_message(
			Glib::ustring::compose(
				_("Subscribing to chat on %1..."),
					remote_hostname));
		g_free(remote_hostname);
	}
}

Gobby::BrowserCommands::RequestInfo::~RequestInfo()
{
	m_status_bar.remove_message(m_handle);

	if(m_request != NULL)
	{
		g_signal_handler_disconnect(m_request, m_finished_handler);
		g_object_unref(m_request);
	}
}

void Gobby::BrowserCommands::RequestInfo::set_request(InfRequest* request)
{
	g_assert(m_request == NULL);

	m_finished_handler = g_signal_handler_find(
		request, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, this);
	g_assert(m_finished_handler != 0);
}

Gobby::BrowserCommands::BrowserCommands(Browser& browser,
                                        FolderManager& folder_manager,
                                        StatusBar& status_bar,
                                        Operations& operations):
	m_browser(browser), m_folder_manager(folder_manager),
	m_operations(operations), m_status_bar(status_bar)
{
	m_browser.signal_connect().connect(
		sigc::mem_fun(*this, &BrowserCommands::on_connect));
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
                                            InfBrowser* old_browser,
                                            InfBrowser* new_browser)
{
	if(old_browser != NULL)
	{
		// Find by browser in case old_browser has its connection
		// reset.
		BrowserMap::iterator iter = m_browser_map.find(old_browser);

		g_assert(iter != m_browser_map.end());

		delete iter->second;
		m_browser_map.erase(iter);
	}

	if(new_browser != NULL)
	{
		g_assert(m_browser_map.find(new_browser) ==
		         m_browser_map.end());

		InfBrowserStatus browser_status;
		g_object_get(
			G_OBJECT(new_browser), "status",
			&browser_status, NULL);

		m_browser_map[new_browser] =
			new BrowserInfo(*this, new_browser);
		if(browser_status == INF_BROWSER_OPEN)
		{
			if(INFC_IS_BROWSER(new_browser))
			{
				InfcBrowser* infcbrowser =
					INFC_BROWSER(new_browser);
				InfcSessionProxy* proxy =
					infc_browser_get_chat_session(
						infcbrowser);
				if(!proxy)
					subscribe_chat(infcbrowser);
			}
		}
	}
}

void Gobby::BrowserCommands::on_notify_status(InfBrowser* browser)
{
	InfXmlConnection* connection;
	InfXmlConnectionStatus status;
	InfBrowserStatus browser_status;

	g_object_get(G_OBJECT(browser), "status", &browser_status, NULL);
	switch(browser_status)
	{
	case INF_BROWSER_CLOSED:
		// Close connection if browser got disconnected. This for
		// example happens when the server does not send an initial
		// welcome message.
		connection =
			infc_browser_get_connection(INFC_BROWSER(browser));
		g_object_get(G_OBJECT(connection), "status", &status, NULL);
		if(status != INF_XML_CONNECTION_CLOSED &&
		   status != INF_XML_CONNECTION_CLOSING)
		{
			inf_xml_connection_close(connection);
		}

		break;
	case INF_BROWSER_OPENING:
		break;
	case INF_BROWSER_OPEN:
		if(!infc_browser_get_chat_session(INFC_BROWSER(browser)))
			subscribe_chat(INFC_BROWSER(browser));
		break;
	default:
		g_assert_not_reached();
		break;
	}
}

void Gobby::BrowserCommands::subscribe_chat(InfcBrowser* browser)
{
	std::auto_ptr<RequestInfo> info(new RequestInfo(
		*this, INF_BROWSER(browser), NULL, m_status_bar));

	InfRequest* request = INF_REQUEST(
		infc_browser_subscribe_chat(
			browser,
			RequestInfo::on_chat_finished_static, info.get()));

	if(request != NULL)
	{
		info->set_request(request);
		g_assert(m_request_map.find(request) == m_request_map.end());
		m_request_map[request] = info.release();
	}
}

void Gobby::BrowserCommands::on_connect(const Glib::ustring& hostname)
{
	m_operations.subscribe_path(hostname, true);
}

void Gobby::BrowserCommands::on_activate(InfBrowser* browser,
                                         InfBrowserIter* iter)
{
	InfSessionProxy* proxy = inf_browser_get_session(browser, iter);

	if(proxy != NULL)
	{
		InfSession* session;
		g_object_get(G_OBJECT(proxy), "session", &session, NULL);
		SessionView* view = m_folder_manager.lookup_document(session);
		g_object_unref(session);

		if(view != NULL)
		{
			m_folder_manager.switch_to_document(*view);
		}
		else
		{
			m_folder_manager.add_document(browser, iter, proxy);
		}
	}
	else
	{
		InfRequest* request =
			inf_browser_get_pending_request(
				browser, iter, "subscribe-session");

		// If there is already a request don't re-request
		if(request == NULL)
		{
			std::auto_ptr<RequestInfo> info(new RequestInfo(
				*this, browser, iter, m_status_bar));

			request = INF_REQUEST(
				inf_browser_subscribe(
					browser, iter,
					RequestInfo::on_node_finished_static,
					info.get()));

			if(request != NULL)
			{
				info->set_request(request);
				g_assert(m_request_map.find(request) ==
				         m_request_map.end());
				m_request_map[request] = info.release();
			}
		}
	}
}

void Gobby::BrowserCommands::on_finished(InfRequest* request,
                                         InfBrowser* browser,
                                         const InfBrowserIter* iter,
                                         const GError* error)
{
	RequestMap::iterator map_iter = m_request_map.find(request);
	if(map_iter != m_request_map.end())
	{
		delete map_iter->second;
		m_request_map.erase(map_iter);
	}

	if(error != NULL)
	{
		m_status_bar.add_error_message(
			_("Subscription failed"),
			error->message);
	}
	else if(iter != NULL)
	{
		InfSessionProxy* proxy =
			inf_browser_get_session(browser, iter);
		g_assert(proxy != NULL);

		m_folder_manager.add_document(browser, iter, proxy);
	}
	else
	{
		if(INFC_IS_BROWSER(browser))
		{
			InfSessionProxy* proxy = INF_SESSION_PROXY(
				infc_browser_get_chat_session(
					INFC_BROWSER(browser)));
			g_assert(proxy != NULL);

			m_folder_manager.add_document(browser, NULL, proxy);
		}
	}
}
