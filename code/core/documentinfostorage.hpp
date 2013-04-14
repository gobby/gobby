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

#ifndef _GOBBY_OPERATIONS_DOCUMENTINFO_STORAGE_HPP_
#define _GOBBY_OPERATIONS_DOCUMENTINFO_STORAGE_HPP_

#include <libinfgtk/inf-gtk-browser-model.h>

#include <libxml++/nodes/element.h>
#include <glibmm/ustring.h>
#include <sigc++/trackable.h>

#include <map>

namespace Gobby
{

class DocumentInfoStorage: public sigc::trackable
{
public:
	enum EolStyle {
		EOL_CRLF,
		EOL_LF,
		EOL_CR
	};

	struct Info {
		Glib::ustring uri;
		EolStyle eol_style;
		std::string encoding;
	};

	DocumentInfoStorage(InfGtkBrowserModel* model);
	~DocumentInfoStorage();

	std::string get_key(InfBrowser* browser,
	                    const InfBrowserIter* iter) const;

	const Info* get_info(InfBrowser* browser,
	                     const InfBrowserIter* iter) const;
	const Info* get_info(const std::string& key) const;

	void set_info(InfBrowser* browser, const InfBrowserIter* iter,
	              const Info& info);
	void set_info(const std::string& key, const Info& info);

protected:
	static void on_set_browser_static(InfGtkBrowserModel* model,
	                                  GtkTreePath* path,
	                                  GtkTreeIter* iter,
	                                  InfBrowser* browser,
	                                  gpointer user_data)
	{
		static_cast<DocumentInfoStorage*>(user_data)->
			on_set_browser(iter, browser);
	}

	static void on_begin_request_explore_node_static(InfBrowser* browser,
	                                                 InfBrowserIter* iter,
	                                                 InfRequest* request,
	                                                 gpointer user_data)
	{
		static_cast<DocumentInfoStorage*>(user_data)->
			on_begin_request_explore_node(browser, iter, request);
	}

	static void on_node_removed_static(InfBrowser* browser,
	                                   InfBrowserIter* iter,
	                                   gpointer user_data)
	{
		static_cast<DocumentInfoStorage*>(user_data)->
			on_node_removed(browser, iter);
	}

	void on_set_browser(GtkTreeIter* iter, InfBrowser* browser);
	void on_begin_request_explore_node(InfBrowser* browser,
	                                   InfBrowserIter* iter,
	                                   InfRequest* request);
	void on_node_removed(InfBrowser* browser, InfBrowserIter* iter);

	typedef std::map<std::string, Info> InfoMap;
	InfoMap m_infos;

	class BrowserConn;
	typedef std::map<InfBrowser*, BrowserConn*> BrowserMap;
	BrowserMap m_browsers;

	gulong m_set_browser_handler;
	InfGtkBrowserModel* m_model;

private:
	void init(xmlpp::Element* node);
};

}

#endif // _GOBBY_OPERATIONS_DOCUMENTINFO_STORAGE_HPP_
