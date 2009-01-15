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

#include "core/nodewatch.hpp"

Gobby::NodeWatch::NodeWatch(InfGtkBrowserModel* model, InfcBrowser* browser,
                            InfcBrowserIter* iter):
	m_model(model), m_browser(browser), m_iter(*iter)
{
	m_set_browser_handler = g_signal_connect(m_model, "set-browser",
	                 G_CALLBACK(on_set_browser_static), this);
	m_node_removed_handler = g_signal_connect(m_browser, "node-removed",
	                 G_CALLBACK(on_node_removed_static), this);
}

Gobby::NodeWatch::~NodeWatch()
{
	if(m_model != NULL)
		reset();
}

void Gobby::NodeWatch::on_set_browser(InfGtkBrowserModel* model,
                                      GtkTreeIter* iter,
				      InfcBrowser* browser)
{
        if(browser == NULL)
        {
                InfcBrowser* old_browser;
                gtk_tree_model_get(
                        GTK_TREE_MODEL(model), iter,
                        INF_GTK_BROWSER_MODEL_COL_BROWSER, &old_browser,
                        -1);

                if(old_browser == m_browser)
		{
			reset();
			// Note that we want to allow signal handlers to
			// delete the watch, so make sure we don't access
			// member variables anymore after having emitted this.
			m_signal_node_removed.emit();
		}

                g_object_unref(old_browser);
        }
}

void Gobby::NodeWatch::on_node_removed(InfcBrowser* browser,
                                       InfcBrowserIter* iter)
{
        InfcBrowserIter test_iter = m_iter;

        do
        {
                if(test_iter.node == iter->node &&
                   test_iter.node_id == iter->node_id)
                {
			reset();
			// Note that we want to allow signal handlers to
			// delete the watch, so make sure we don't access
			// member variables anymore after having emitted this.
			m_signal_node_removed.emit();
                        break;
                }
        } while(infc_browser_iter_get_parent(browser, &test_iter));
}

void Gobby::NodeWatch::reset()
{
	g_signal_handler_disconnect(m_model, m_set_browser_handler);
	g_signal_handler_disconnect(m_browser, m_node_removed_handler);

	m_model = NULL;
	m_browser = NULL;
}
