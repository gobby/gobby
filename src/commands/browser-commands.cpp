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

#include "commands/browser-commands.hpp"
#include "i18n.hpp"

Gobby::BrowserCommands::BrowserCommands(Browser& browser, Folder& folder,
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
		m_status_bar.remove_message(iter->second.handle);
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
		InfTextSession* text_session = INF_TEXT_SESSION(session);
		DocWindow* window = m_folder.lookup_document(text_session);

		if(window != NULL)
		{
			m_folder.switch_to_document(*window);
		}
		else
		{
			// This should not happen: We insert every document
			// we subscribe to directly into the folder.
			g_assert_not_reached();
			/*folder.add_document(
				text_session,
				infc_browser_iter_get_name(browser, iter));*/
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

			RequestNode& node = m_request_map[request];
			node.browser = browser;
			node.handle = m_status_bar.add_message(
				StatusBar::INFO,
				Glib::ustring::compose(
					_("Subscribing to %1…"),
					Glib::ustring(
						infc_browser_iter_get_name(
							browser, iter))), 0);

			g_signal_connect(
				request, "finished",
				G_CALLBACK(&on_finished_static), this);

			g_signal_connect(
				request, "failed",
				G_CALLBACK(&on_failed_static), this);
		}
		else
		{
			m_status_bar.add_message(
				StatusBar::INFO,
				_("Subscription already in progress"), 2);
		}
	}
}

void Gobby::BrowserCommands::on_finished(InfcNodeRequest* request)
{
	RequestMap::iterator iter = m_request_map.find(request);
	g_assert(iter != m_request_map.end());

	InfcBrowser* browser = iter->second.browser;
	InfcBrowserIter browser_iter;
	if(infc_browser_iter_from_node_request(browser, request,
	                                       &browser_iter))
	{
		InfcSessionProxy* proxy = infc_browser_iter_get_session(
			iter->second.browser, &browser_iter);

		m_folder.add_document(
			INF_TEXT_SESSION(
				infc_session_proxy_get_session(proxy)),
			infc_browser_iter_get_name(browser, &browser_iter));

		// TODO: Track synchronization and user join, update info
		// of docwindow accordingly.
	}

	m_status_bar.remove_message(iter->second.handle);
	m_request_map.erase(iter);
}

void Gobby::BrowserCommands::on_failed(InfcNodeRequest* request,
                                       const GError* error)
{
	RequestMap::iterator iter = m_request_map.find(request);
	g_assert(iter != m_request_map.end());

	m_status_bar.remove_message(iter->second.handle);
	m_request_map.erase(iter);

	m_status_bar.add_message(
		StatusBar::ERROR,
		Glib::ustring::compose(_("Subscription failed: %1"),
			error->message), 5);
}
