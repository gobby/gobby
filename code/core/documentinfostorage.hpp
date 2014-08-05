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
	                                  InfBrowser* old_browser,
	                                  InfBrowser* new_browser,
	                                  gpointer user_data)
	{
		static_cast<DocumentInfoStorage*>(user_data)->
			on_set_browser(iter, old_browser, new_browser);
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
	                                   InfRequest* request,
	                                   gpointer user_data)
	{
		static_cast<DocumentInfoStorage*>(user_data)->
			on_node_removed(browser, iter, request);
	}

	void on_set_browser(GtkTreeIter* iter, InfBrowser* old_browser,
	                    InfBrowser* new_browser);
	void on_begin_request_explore_node(InfBrowser* browser,
	                                   InfBrowserIter* iter,
	                                   InfRequest* request);
	void on_node_removed(InfBrowser* browser, InfBrowserIter* iter,
	                     InfRequest* request);

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
