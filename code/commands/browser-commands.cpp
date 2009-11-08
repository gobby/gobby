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

#include "commands/browser-commands.hpp"
#include "util/i18n.hpp"

class Gobby::BrowserCommands::RequestInfo
{
public:
	RequestInfo(BrowserCommands& commands, InfcBrowser* browser,
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

Gobby::BrowserCommands::RequestInfo::RequestInfo(BrowserCommands& commands,
                                                 InfcBrowser* browser,
                                                 InfcNodeRequest* request,
                                                 StatusBar& status_bar):
	m_request(request), m_status_bar(status_bar)
{
	g_object_ref(request);

	InfcBrowserIter iter;
	gboolean have_iter =
		infc_browser_iter_from_node_request(browser, request, &iter);
	g_assert(have_iter);

	m_handle = m_status_bar.add_info_message(
		Glib::ustring::compose(_("Subscribing to %1..."),
			Glib::ustring(
				infc_browser_iter_get_name(browser, &iter))));

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
}

Gobby::BrowserCommands::~BrowserCommands()
{
	for(RequestMap::iterator iter = m_request_map.begin();
	    iter != m_request_map.end(); ++ iter)
	{
		delete iter->second;
	}
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
				new RequestInfo(*this, browser, request,
				                m_status_bar);
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
