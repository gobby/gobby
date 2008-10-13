/* gobby - A GTKmm driven libobby client
 * Copyright (C) 2005 - 2008 0x539 dev group
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

	std::string get_key(InfcBrowser* browser,
	                    InfcBrowserIter* iter) const;

	const Info* get_info(InfcBrowser* browser,
	                     InfcBrowserIter* iter) const;
	const Info* get_info(const std::string& key) const;

	void set_info(InfcBrowser* browser, InfcBrowserIter* iter,
	              const Info& info);
	void set_info(const std::string& key, const Info& info);

protected:
	static void on_set_browser_static(InfGtkBrowserModel* model,
	                                  GtkTreePath* path,
	                                  GtkTreeIter* iter,
	                                  InfcBrowser* browser,
	                                  gpointer user_data)
	{
		static_cast<DocumentInfoStorage*>(user_data)->
			on_set_browser(iter, browser);
	}

	static void on_begin_explore_static(InfcBrowser* browser,
	                                    InfcBrowserIter* iter,
					    InfcExploreRequest* request,
	                                    gpointer user_data)
	{
		static_cast<DocumentInfoStorage*>(user_data)->
			on_begin_explore(browser, iter, request);
	}

	static void on_node_removed_static(InfcBrowser* browser,
	                                   InfcBrowserIter* iter,
	                                   gpointer user_data)
	{
		static_cast<DocumentInfoStorage*>(user_data)->
			on_node_removed(browser, iter);
	}

	void on_set_browser(GtkTreeIter* iter, InfcBrowser* browser);
	void on_begin_explore(InfcBrowser* browser, InfcBrowserIter* iter,
	                      InfcExploreRequest* request);
	void on_node_removed(InfcBrowser* browser, InfcBrowserIter* iter);

	typedef std::map<std::string, Info> InfoMap;
	InfoMap m_infos;

	class BrowserConn;
	typedef std::map<InfcBrowser*, BrowserConn*> BrowserMap;
	BrowserMap m_browsers;

	gulong m_set_browser_handler;
	InfGtkBrowserModel* m_model;

private:
	void init(xmlpp::Element* node);
};

}

#endif // _GOBBY_OPERATIONS_DOCUMENTINFO_STORAGE_HPP_
