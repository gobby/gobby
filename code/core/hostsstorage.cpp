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

#include "core/hostsstorage.hpp"
#include "util/file.hpp"

#include <libinfinity/common/inf-browser.h>
#include <libinfgtk/inf-gtk-browser-store.h>

#include <glibmm/miscutils.h>

#include <libxml++/nodes/textnode.h>
#include <libxml++/parsers/domparser.h>

Gobby::HostsStorage::HostsStorage(const std::string &path):
	m_path(path)
{
}

void Gobby::HostsStorage::load(Operations &operations) try
{
	xmlpp::DomParser parser;

	parser.parse_file(m_path);
	xmlpp::Document* document = parser.get_document();
	if(document)
	{
		xmlpp::Element* root = document->get_root_node();
		if(root)
		{
			xmlpp::Node::NodeList list = root->get_children();
			for(xmlpp::Node::NodeList::iterator iter = list.begin();
				iter != list.end();
				++ iter)
			{
				xmlpp::Element* child = dynamic_cast<xmlpp::Element*>(*iter);
				if(child == NULL) continue;

				if(child->get_name() == "host")
				{
					xmlpp::Node::NodeList list = child->get_children();
					for(xmlpp::Node::NodeList::iterator iter = list.begin();
						iter != list.end();
						++ iter)
					{
						xmlpp::Element* child =
							dynamic_cast<xmlpp::Element*>(*iter);
						if(child == NULL) continue;

						xmlpp::TextNode* text = child->get_child_text();
						if(text == NULL) continue;

						if(child->get_name() == "address")
						{
							std::string host = text->get_content();
							operations.subscribe_path(host, false);
						}
					}
				}
			}
		}
	}
}
catch(xmlpp::exception& e)
{
	/* could not read file, ignore */
}

void Gobby::HostsStorage::save(InfGtkBrowserModel* model) try
{
	InfBrowser* browser;
	GtkTreeIter iter;

	create_directory_with_parents(
		Glib::path_get_dirname(m_path), 0700);
	xmlpp::Document document;
	xmlpp::Element* root = document.create_root_node("hosts");

	if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter))
	{
		do
		{
			gtk_tree_model_get(
				GTK_TREE_MODEL(model), &iter,
				INF_GTK_BROWSER_MODEL_COL_BROWSER, &browser,
				-1);

			if(INFC_IS_BROWSER(browser))
			{
				InfXmlConnection* connection = infc_browser_get_connection(
					INFC_BROWSER(browser));

				if(INF_IS_XMPP_CONNECTION(connection))
				{
					gchar *hostname;

					g_object_get(
						G_OBJECT(connection),
						"remote-hostname", &hostname,
						NULL);

					xmlpp::Element* child = root->add_child("host");
					xmlpp::Element* address_child = child->add_child("address");

					address_child->set_child_text(
						(std::string) "infinote://" + hostname + "/");

					g_free(hostname);
				}
			}

			if(browser)
			{
				g_object_unref(G_OBJECT(browser));
			}
		} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter));
	}

	document.write_to_file_formatted(m_path);
}
catch(Glib::Exception& e)
{
	g_warning("Could not write hosts file: %s",
	          e.what().c_str());
}
