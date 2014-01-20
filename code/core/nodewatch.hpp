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

#ifndef _GOBBY_NODE_WATCH_HPP_
#define _GOBBY_NODE_WATCH_HPP_

#include <sigc++/signal.h>

#include <libinfgtk/inf-gtk-browser-model.h>
#include <libinfinity/client/infc-browser.h>

namespace Gobby
{

class NodeWatch
{
public:
	typedef sigc::signal<void> SignalNodeRemoved;

	NodeWatch(InfBrowser* browser, const InfBrowserIter* iter);
	~NodeWatch();

	InfBrowser* get_browser() const { return m_browser; }
	const InfBrowserIter* get_browser_iter() const {
		g_assert(m_browser);
		if(m_iter.node != NULL)
			return &m_iter;
		else
			return NULL;
	}

	SignalNodeRemoved signal_node_removed() const
	{
		return m_signal_node_removed;
	}
protected:
	static void on_notify_status_static(GObject* object,
	                                    GParamSpec* pspec,
	                                    gpointer user_data)
	{
		static_cast<NodeWatch*>(user_data)->
			on_notify_status();
	}

	static void on_node_removed_static(InfBrowser* browser,
	                                   InfBrowserIter* iter,
	                                   InfNodeRequest* request,
	                                   gpointer user_data)
	{
		static_cast<NodeWatch*>(user_data)->
			on_node_removed(browser, iter, request);
	}

	void on_notify_status();
	void on_node_removed(InfBrowser* browser, InfBrowserIter* iter,
	                     InfNodeRequest* request);

	void reset();

	InfBrowser* m_browser;
	InfXmlConnection* m_connection;
	InfBrowserIter m_iter;

	gulong m_notify_status_handler;
	gulong m_node_removed_handler;

	SignalNodeRemoved m_signal_node_removed;
};

}
	
#endif // _GOBBY_NODE_WATCH_HPP_
