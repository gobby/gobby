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

#include "core/nodewatch.hpp"

Gobby::NodeWatch::NodeWatch(InfBrowser* browser,
                            const InfBrowserIter* iter):
	m_browser(browser), m_node_removed_handler(0)
{
	if(iter != NULL)
		m_iter = *iter;
	else
		m_iter.node = NULL;

	// TODO: (weak-)ref browser?
	InfBrowserStatus status;
	g_object_get(G_OBJECT(m_browser), "status", &status, NULL);
	g_assert(status == INF_BROWSER_OPEN);
	
	m_notify_status_handler = g_signal_connect(
		browser, "notify::status",
		G_CALLBACK(on_notify_status_static), this);

	if(m_iter.node != NULL)
	{
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
                                       InfBrowserIter* iter)
{
	g_assert(browser == m_browser);
	g_assert(m_iter.node != NULL);

	if(inf_browser_is_ancestor(m_browser, &m_iter, iter))
	{
		reset();
		m_signal_node_removed.emit();
	}
}

void Gobby::NodeWatch::reset()
{
	g_assert(m_browser != NULL);

	g_signal_handler_disconnect(
		m_browser, m_notify_status_handler);

	if(m_node_removed_handler != 0)
	{
		g_signal_handler_disconnect(
			m_browser, m_node_removed_handler);
	}

	m_iter.node = NULL;
	m_browser = NULL;
}
