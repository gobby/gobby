/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2015 Armin Burgmeier <armin@arbur.net>
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

#include "core/nodewatch.hpp"

Gobby::NodeWatch::NodeWatch(InfBrowser* browser,
                            const InfBrowserIter* iter):
	m_browser(browser), m_node_removed_handler(0),
	m_notify_status_handler(0)
{
	if(iter != NULL)
		m_iter = *iter;
	else
		m_iter.node = NULL;

	// TODO: (weak-)ref browser?
	InfBrowserStatus status;
	g_object_get(G_OBJECT(m_browser), "status", &status, NULL);
	g_assert(iter == NULL || status == INF_BROWSER_OPEN);
	
	if(m_iter.node != NULL)
	{
		m_notify_status_handler = g_signal_connect(
			browser, "notify::status",
			G_CALLBACK(on_notify_status_static), this);

		m_node_removed_handler = g_signal_connect(
			m_browser, "node-removed",
			G_CALLBACK(on_node_removed_static), this);
	}
}

Gobby::NodeWatch::~NodeWatch()
{
	if(m_browser != NULL)
		reset();
}

void Gobby::NodeWatch::on_notify_status()
{
	InfBrowserStatus status;
	g_object_get(G_OBJECT(m_browser), "status", &status, NULL);

	// Connection was closed: Node is no longer reachable
	if(status != INF_BROWSER_OPEN)
	{
		reset();
		m_signal_node_removed.emit();
	}
}

void Gobby::NodeWatch::on_node_removed(InfBrowser* browser,
                                       InfBrowserIter* iter,
                                       InfRequest* request)
{
	g_assert(browser == m_browser);
	g_assert(m_iter.node != NULL);

	if(inf_browser_is_ancestor(m_browser, iter, &m_iter))
	{
		reset();
		m_signal_node_removed.emit();
	}
}

void Gobby::NodeWatch::reset()
{
	g_assert(m_browser != NULL);

	if(m_notify_status_handler != 0)
	{
		g_signal_handler_disconnect(
			m_browser, m_notify_status_handler);
	}

	if(m_node_removed_handler != 0)
	{
		g_signal_handler_disconnect(
			m_browser, m_node_removed_handler);
	}

	m_iter.node = NULL;
	m_browser = NULL;
}
