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

#ifndef _GOBBY_FOLDERMANAGER_HPP_
#define _GOBBY_FOLDERMANAGER_HPP_

#include "core/browser.hpp"
#include "core/folder.hpp"
#include "core/documentinfostorage.hpp"

#include <libinfinity/common/inf-browser.h>
#include <libinfinity/common/inf-session-proxy.h>

namespace Gobby
{

// The FolderManager class manages the two folders, for text documents and
// chat documents. On top of this, it also manages a relation of browser
// entries to sessions and session proxies.
class FolderManager: public sigc::trackable
{
public:
	typedef sigc::signal<void, InfBrowser*, const InfBrowserIter*,
	                     InfSessionProxy*, Folder&, SessionView&>
		SignalDocumentAdded;

	typedef sigc::signal<void, InfBrowser*, const InfBrowserIter*,
	                     InfSessionProxy*, Folder&, SessionView&>
		SignalDocumentRemoved;

	FolderManager(Browser& browser,
	              DocumentInfoStorage& info_storage,
	              Folder& text_folder,
	              Folder& chat_folder);
	~FolderManager();

	const Folder& get_text_folder() const { return m_text_folder; }
	const Folder& get_chat_folder() const { return m_chat_folder; }

	// Add a SessionView for the given session
	void add_document(InfBrowser* browser, const InfBrowserIter* iter,
	                  InfSessionProxy* proxy);
	void remove_document(SessionView& view);

	SessionView* lookup_document(InfSession* session) const;
	void switch_to_document(SessionView& view);

	// Emitted whenever a document has been added, either for text or
	// for chat sessions.
	SignalDocumentAdded signal_document_added() const
	{
		return m_signal_document_added;
	}

	SignalDocumentRemoved signal_document_removed() const
	{
		return m_signal_document_removed;
	}

protected:
	static void on_set_browser_static(InfGtkBrowserModel* model,
	                                  GtkTreePath* path,
	                                  GtkTreeIter* iter,
	                                  InfBrowser* old_browser,
	                                  InfBrowser* new_browser,
	                                  gpointer user_data)
	{
		static_cast<FolderManager*>(user_data)->on_set_browser(
			model, iter, old_browser, new_browser);
	}

	static void on_unsubscribe_session_static(InfBrowser* browser,
	                                          const InfBrowserIter* iter,
	                                          InfSessionProxy* proxy,
	                                          InfRequest* request,
	                                          gpointer user_data)
	{
		static_cast<FolderManager*>(user_data)->
			on_unsubscribe_session(browser, iter, proxy, request);
	}

	void on_set_browser(InfGtkBrowserModel* model, GtkTreeIter* iter,
	                    InfBrowser* old_browser, InfBrowser* new_browser);

	void on_unsubscribe_session(InfBrowser* browser,
	                            const InfBrowserIter* iter,
	                            InfSessionProxy* proxy,
	                            InfRequest* request);

	void on_text_document_added(SessionView& view);
	void on_chat_document_added(SessionView& view);
	void on_document_removed(SessionView& view);

	Browser& m_browser;
	DocumentInfoStorage& m_info_storage;
	Folder& m_text_folder;
	Folder& m_chat_folder;

	sigc::connection m_text_document_added_connection;
	sigc::connection m_chat_document_added_connection;
	gulong m_set_browser_handler;

	class BrowserInfo;
	typedef std::map<InfBrowser*, BrowserInfo*> BrowserMap;
	BrowserMap m_browser_map;

	class SessionInfo;
	typedef std::map<InfSession*, SessionInfo*> SessionMap;
	SessionMap m_session_map;

	SignalDocumentAdded m_signal_document_added;
	SignalDocumentRemoved m_signal_document_removed;
};

}

#endif // _GOBBY_FOLDERMANAGER_HPP_
