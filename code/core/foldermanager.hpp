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

#ifndef _GOBBY_FOLDERMANAGER_HPP_
#define _GOBBY_FOLDERMANAGER_HPP_

#include "core/browser.hpp"
#include "core/folder.hpp"
#include "core/userjoin.hpp"
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
	typedef std::auto_ptr<UserJoin>* UserJoinRef;

	typedef sigc::signal<void, InfBrowser*, const InfBrowserIter*,
	                     InfSessionProxy*, Folder&, SessionView&,
	                     UserJoinRef>
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
	                  InfSessionProxy* proxy, UserJoinRef userjoin);
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
