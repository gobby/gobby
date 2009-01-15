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

	NodeWatch(InfGtkBrowserModel* model, InfcBrowser* browser,
	          InfcBrowserIter* iter);
	~NodeWatch();

	InfGtkBrowserModel* get_model() const { return m_model; }
	InfcBrowser* get_browser() const { return m_browser; }
	bool get_browser_iter(InfcBrowserIter* iter) const;

	SignalNodeRemoved signal_node_removed() const
	{
		return m_signal_node_removed;
	}
protected:
	static void on_set_browser_static(InfGtkBrowserModel* model,
	                                  GtkTreePath* path,
					  GtkTreeIter* iter,
					  InfcBrowser* browser,
					  gpointer user_data)
	{
		static_cast<NodeWatch*>(user_data)->
			on_set_browser(model, iter, browser);
	}

	static void on_node_removed_static(InfcBrowser* browser,
	                                   InfcBrowserIter* iter,
					   gpointer user_data)
	{
		static_cast<NodeWatch*>(user_data)->
			on_node_removed(browser, iter);
	}

	void on_set_browser(InfGtkBrowserModel* model, GtkTreeIter* iter,
	                    InfcBrowser* browser);
	void on_node_removed(InfcBrowser* browser, InfcBrowserIter* iter);

	void reset();

	InfGtkBrowserModel* m_model;
	InfcBrowser* m_browser;
	InfcBrowserIter m_iter;

	gulong m_set_browser_handler;
	gulong m_node_removed_handler;

	SignalNodeRemoved m_signal_node_removed;
};

}
	
#endif // _GOBBY_NODE_WATCH_HPP_
