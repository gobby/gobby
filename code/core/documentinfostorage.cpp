/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2011 Armin Burgmeier <armin@arbur.net>
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

#include "core/documentinfostorage.hpp"
#include "util/file.hpp"

#include "features.hpp"

#include <libxml++/nodes/textnode.h>
#include <libxml++/parsers/domparser.h>
#include <glibmm/miscutils.h>
#include <glibmm/exception.h>

namespace
{
	Gobby::DocumentInfoStorage::EolStyle
	eol_style_from_text(const std::string& text)
	{
		if(text == "cr")
			return Gobby::DocumentInfoStorage::EOL_CR;
		if(text == "lf")
			return Gobby::DocumentInfoStorage::EOL_LF;
		if(text == "crlf")
			return Gobby::DocumentInfoStorage::EOL_CRLF;
		// Fallback:
		return Gobby::DocumentInfoStorage::EOL_LF;
	}

	std::string
	eol_style_to_text(Gobby::DocumentInfoStorage::EolStyle style)
	{
		switch(style)
		{
		case Gobby::DocumentInfoStorage::EOL_CR: return "cr";
		case Gobby::DocumentInfoStorage::EOL_LF: return "lf";
		case Gobby::DocumentInfoStorage::EOL_CRLF: return "crlf";
		default: return "lf";
		}
	}

	std::string load_document(xmlpp::Element* node,
	                          Gobby::DocumentInfoStorage::Info& info)
	{
		std::string root;

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

			if(child->get_name() == "root")
				root = text->get_content();
			else if(child->get_name() == "uri")
				info.uri = text->get_content();
			else if(child->get_name() == "eol-style")
				info.eol_style = eol_style_from_text(
					text->get_content());
			else if(child->get_name() == "encoding")
				info.encoding = text->get_content();
		}

		return root;
	}

	// Location to store the documents file:
	std::string filename()
	{
		return Gobby::config_filename("documents.xml");
	}
}

class Gobby::DocumentInfoStorage::BrowserConn
{
public:
	BrowserConn(DocumentInfoStorage& storage, InfcBrowser* browser):
		m_browser(browser)
	{
		m_begin_explore_handler =
			g_signal_connect(G_OBJECT(browser), "begin-explore",
			                 G_CALLBACK(on_begin_explore_static),
			                 &storage);

		m_node_removed_handler =
			g_signal_connect(G_OBJECT(browser), "node-removed",
			                 G_CALLBACK(on_node_removed_static),
			                 &storage);
	}

	~BrowserConn()
	{
		g_signal_handler_disconnect(m_browser,
		                            m_begin_explore_handler);
		g_signal_handler_disconnect(m_browser,
		                            m_node_removed_handler);
	}

private:
	InfcBrowser* m_browser;
	gulong m_begin_explore_handler;
	gulong m_node_removed_handler;
};

Gobby::DocumentInfoStorage::DocumentInfoStorage(InfGtkBrowserModel* model):
	m_model(model)
{
	xmlpp::DomParser parser;

	try
	{
		parser.parse_file(filename());
		xmlpp::Document* document = parser.get_document();
		if(document)
		{
			xmlpp::Element* root = document->get_root_node();
			if(root)
			{
				init(root);
			}
		}
	}
	catch(xmlpp::exception& e)
	{
		// Could not read file, ignore
	}

	g_object_ref(m_model);

	m_set_browser_handler = g_signal_connect(
		G_OBJECT(m_model), "set-browser",
		G_CALLBACK(&on_set_browser_static), this);
}

Gobby::DocumentInfoStorage::~DocumentInfoStorage()
{
	try
	{
		create_directory_with_parents(
			Glib::path_get_dirname(filename()), 0700);
		xmlpp::Document document;
		xmlpp::Element* root = document.create_root_node("documents");

		for(InfoMap::iterator iter = m_infos.begin();
		    iter != m_infos.end(); ++ iter)
		{
			xmlpp::Element* child = root->add_child("document");

			xmlpp::Element* root_child = child->add_child("root");
			root_child->set_child_text(iter->first);

			xmlpp::Element* uri_child = child->add_child("uri");
			uri_child->set_child_text(iter->second.uri);

			xmlpp::Element* eol_style_child =
				child->add_child("eol-style");
			eol_style_child->set_child_text(
				eol_style_to_text(iter->second.eol_style));

			xmlpp::Element* encoding_child =
				child->add_child("encoding");
			encoding_child->set_child_text(iter->second.encoding);
		}

		document.write_to_file_formatted(filename());
	}
	catch(Glib::Exception& e)
	{
		g_warning("Could not write documents file: %s",
		          e.what().c_str());
	}
	catch(std::exception& e)
	{
		g_warning("Could not write documents file: %s",
		          e.what());
	}

	g_signal_handler_disconnect(m_model, m_set_browser_handler);
	g_object_unref(m_model);

	for(BrowserMap::iterator iter = m_browsers.begin();
	    iter != m_browsers.end(); ++ iter)
	{
		delete iter->second;
	}
}

void Gobby::DocumentInfoStorage::init(xmlpp::Element* node)
{
	xmlpp::Node::NodeList list = node->get_children();
	for(xmlpp::Node::NodeList::iterator iter = list.begin();
	    iter != list.end();
	    ++ iter)
	{
		xmlpp::Element* child = dynamic_cast<xmlpp::Element*>(*iter);
		if(child == NULL) continue;

		if(child->get_name() == "document")
		{
			Info info;
			std::string root = load_document(child, info);
			m_infos[root] = info;
		}
	}
}

std::string
Gobby::DocumentInfoStorage::get_key(InfcBrowser* browser,
                                    InfcBrowserIter* iter) const
{
	InfXmlConnection* connection = infc_browser_get_connection(browser);
	g_assert(connection != NULL);

	gchar* path = infc_browser_iter_get_path(browser, iter);
	gchar* remote_id;
	g_object_get(G_OBJECT(connection), "remote-id", &remote_id, NULL);
	std::string result = std::string(remote_id) + "?" + path;
	g_free(path);
	g_free(remote_id);

	return result;
}

const Gobby::DocumentInfoStorage::Info*
Gobby::DocumentInfoStorage::get_info(InfcBrowser* browser,
                                     InfcBrowserIter* iter) const
{
	return get_info(get_key(browser, iter));
}

const Gobby::DocumentInfoStorage::Info*
Gobby::DocumentInfoStorage::get_info(const std::string& key) const
{
	InfoMap::const_iterator map_iter = m_infos.find(key);
	if(map_iter != m_infos.end()) return &map_iter->second;
	return NULL;
}

void Gobby::DocumentInfoStorage::set_info(InfcBrowser* browser,
                                          InfcBrowserIter* iter,
                                          const Info& info)
{
	set_info(get_key(browser, iter), info);
}

void Gobby::DocumentInfoStorage::set_info(const std::string& key,
                                          const Info& info)
{
	m_infos[key] = info;
}

void Gobby::DocumentInfoStorage::on_set_browser(GtkTreeIter* iter,
                                                InfcBrowser* browser)
{
	if(browser != NULL)
	{
		g_assert(m_browsers.find(browser) == m_browsers.end());
		m_browsers[browser] = new BrowserConn(*this, browser);

		// TODO: Remove all infos that refer to no longer existing
		// documents in all explored directories in browser. Also
		// add all existing explore requests, to check
		// subdirectories when the explore finishes (see
		// on_begin_explore).
	}
	else
	{
		InfcBrowser* old_browser;
		gtk_tree_model_get(GTK_TREE_MODEL(m_model), iter,
		                   INF_GTK_BROWSER_MODEL_COL_BROWSER,
				   &old_browser,
				   -1);

		if(old_browser)
		{
			BrowserMap::iterator iter =
				m_browsers.find(old_browser);
			g_assert(iter != m_browsers.end());

			delete iter->second;
			m_browsers.erase(iter);
			g_object_unref(old_browser);
		}
	}
}

void Gobby::DocumentInfoStorage::on_begin_explore(InfcBrowser* browser,
                                                  InfcBrowserIter* iter,
                                                  InfcExploreRequest* request)
{
	// TODO: When request has finished, remove all infos that refer to
	// no longer existing documents in the explored directory.
}

void Gobby::DocumentInfoStorage::on_node_removed(InfcBrowser* browser,
                                                 InfcBrowserIter* iter)
{
	// Remove info when the corresponding document is removed.
	std::string key = get_key(browser, iter);
	InfoMap::iterator map_iter = m_infos.find(key);

	if(map_iter != m_infos.end())
		m_infos.erase(map_iter);
}

