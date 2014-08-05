/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2014 Armin Burgmeier <armin@arbur.net>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
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
	                                   InfRequest* request,
	                                   gpointer user_data)
	{
		static_cast<NodeWatch*>(user_data)->
			on_node_removed(browser, iter, request);
	}

	void on_notify_status();
	void on_node_removed(InfBrowser* browser, InfBrowserIter* iter,
	                     InfRequest* request);

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
