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

#include "core/knownhoststorage.hpp"
#include "util/file.hpp"

#include "features.hpp"

#include <libxml++/nodes/textnode.h>
#include <libxml++/parsers/domparser.h>

#include <glibmm/miscutils.h>

#include <libinfinity/common/inf-protocol.h>

namespace
{
	struct HostInfo
	{
		std::string name;
		std::string hostname;
		std::string service;
	};

	bool load_host(xmlpp::Element* node, HostInfo& info)
	{
		bool found_name = false;
		bool found_hostname = false;

		info.service = Glib::ustring::compose(
			"%1", inf_protocol_get_default_port());

		xmlpp::Node::NodeList list = node->get_children();
		for(xmlpp::Node::NodeList::iterator iter = list.begin();
		    iter != list.end();
		    ++ iter)
		{
			xmlpp::Element* child =
				dynamic_cast<xmlpp::Element*>(*iter);
			if(child == NULL) continue;

			xmlpp::TextNode* text = child->get_child_text();
			if(text == NULL) continue;

			if(child->get_name() == "name")
			{
				info.name = text->get_content();
				found_name = true;
			}
			else if(child->get_name() == "hostname")
			{
				info.hostname = text->get_content();
				found_hostname = true;
			}
			else if(child->get_name() == "service")
			{
				info.service = text->get_content();
			}
		}

		return found_name && found_hostname;
	}

	// Location to store the hosts file:
	std::string filename()
	{
		return Gobby::config_filename("hosts.xml");
	}
}

Gobby::KnownHostStorage::KnownHostStorage(Browser& browser):
	m_browser(browser)
{
	xmlpp::DomParser parser;

	try
	{
		parser.parse_file(filename());
	}
	catch(xmlpp::exception& e)
	{
		// Could not open file, or file is invalid. Start
		// with a single entry.
		std::string service = Glib::ustring::compose(
			"%1", inf_protocol_get_default_port());
		browser.add_remote("gobby.0x539.de", service, 0, false);
	}

	try
	{
		xmlpp::Document* document = parser.get_document();
		if(document)
		{
			xmlpp::Element* root = document->get_root_node();
			if(root)
			{
				xmlpp::Node::NodeList list =
					root->get_children();
				for(xmlpp::Node::NodeList::iterator iter =
					list.begin();
				    iter != list.end();
				    ++ iter)
				{
					xmlpp::Element* child =
						dynamic_cast<xmlpp::Element*>(
							*iter);
					if(child == NULL) continue;

					HostInfo info;
					if(child->get_name() == "host" &&
					   load_host(child, info))
					{
						// TODO: Store device name
						// in the file as well so
						// that we can recover IPv6
						// link-local connections.
						browser.add_remote(
							info.hostname,
							info.service,
							0, false);
					}
				}
			}
		}
	}
	catch(xmlpp::exception& e)
	{
		// Could not read file, ignore
	}
}

Gobby::KnownHostStorage::~KnownHostStorage()
{
	try
	{
		create_directory_with_parents(
			Glib::path_get_dirname(filename()), 0700);
		xmlpp::Document document;
		xmlpp::Element* root = document.create_root_node("hosts");
		
		GtkTreeModel* model = GTK_TREE_MODEL(m_browser.get_store());

		GtkTreeIter iter;
		for(gboolean have_item =
			gtk_tree_model_get_iter_first(model, &iter);
		    have_item == TRUE;
		    have_item = gtk_tree_model_iter_next(model, &iter))
		{
			InfBrowser* browser;
			InfDiscovery* discovery;
			gtk_tree_model_get(
				model, &iter,
				INF_GTK_BROWSER_MODEL_COL_BROWSER,
				&browser,
				INF_GTK_BROWSER_MODEL_COL_DISCOVERY,
				&discovery,
				-1);

			// Skip items that were discovered
			if(discovery != NULL)
			{
				g_object_unref(discovery);
				if(browser != NULL) g_object_unref(browser);
				continue;
			}

			if(browser == NULL)
				continue;

			if(!INFC_IS_BROWSER(browser))
			{
				g_object_unref(browser);
				continue;
			}

			InfXmlConnection* connection =
				infc_browser_get_connection(
					INFC_BROWSER(browser));
			if(connection == NULL ||
			   !INF_IS_XMPP_CONNECTION(connection))
			{
				g_object_unref(browser);
				continue;
			}

			InfTcpConnection* tcp;
			g_object_get(
				G_OBJECT(connection), 
				"tcp-connection", &tcp,
				NULL);
			g_object_unref(browser);
			
			InfNameResolver* resolver;
			g_object_get(
				G_OBJECT(tcp), "resolver", &resolver, NULL);
			g_object_unref(tcp);

			// TODO: If resolver is NULL, should we instead
			// record hostname and port number?
			if(resolver == NULL)
				continue;

			const char* hostname =
				inf_name_resolver_get_hostname(resolver);
			const char* service =
				inf_name_resolver_get_service(resolver);
			const char* srv =
				inf_name_resolver_get_srv(resolver);

			if(strcmp(srv, "_infinote._tcp") != 0)
			{
				g_object_unref(resolver);
				continue;
			}

			gchar* name;
			gtk_tree_model_get(
				model, &iter,
				INF_GTK_BROWSER_MODEL_COL_NAME, &name,
				-1);

			xmlpp::Element* child = root->add_child("host");
			xmlpp::Element* name_elem =
				child->add_child("name");
			name_elem->set_child_text(name);
			
			xmlpp::Element* hostname_elem =
				child->add_child("hostname");
			hostname_elem->set_child_text(hostname);

			xmlpp::Element* service_elem =
				child->add_child("service");
			service_elem->set_child_text(service);

			g_object_unref(resolver);
		}

		document.write_to_file_formatted(filename());
	}
	catch(Glib::Exception& e)
	{
		g_warning("Could not write hosts file: %s",
		          e.what().c_str());
	}
	catch(std::exception& e)
	{
		g_warning("Could not write hosts file: %s",
		          e.what());
	}
}

