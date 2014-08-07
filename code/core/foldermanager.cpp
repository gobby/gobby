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

#include "core/foldermanager.hpp"
#include "util/i18n.hpp"

#include <libinfinity/client/infc-browser.h>
#include <libinfinity/server/infd-directory.h>

class Gobby::FolderManager::BrowserInfo
{
public:
	BrowserInfo(FolderManager& manager, InfBrowser* browser):
		m_browser(browser)
	{
		g_object_ref(m_browser);

		m_unsubscribe_session_handler = g_signal_connect(
			G_OBJECT(browser),
			"unsubscribe-session",
			G_CALLBACK(&on_unsubscribe_session_static),
			&manager);
	}

	~BrowserInfo()
	{
		g_signal_handler_disconnect(G_OBJECT(m_browser),
		                            m_unsubscribe_session_handler);

		g_object_unref(m_browser);
	}

private:
	InfBrowser* m_browser;
	gulong m_unsubscribe_session_handler;
};

class Gobby::FolderManager::SessionInfo
{
public:
	SessionInfo(Folder& folder, InfBrowser* browser,
	            const InfBrowserIter* iter, InfSessionProxy* proxy):
		m_folder(folder), m_browser(browser), m_proxy(proxy)
	{
		if(browser != NULL && iter != NULL)
			m_browser_iter = *iter;
		else
			m_browser = NULL;

		if(proxy != NULL)
			g_object_ref(proxy);
	}

	~SessionInfo()
	{
		if(m_proxy != NULL)
			g_object_unref(m_proxy);
	}

	void reset_browser()
	{
		m_browser = NULL;
	}

	InfBrowser* get_browser() { return m_browser; }
	const InfBrowserIter* get_browser_iter()
		{ return m_browser == NULL ? NULL : &m_browser_iter; }
	Folder& get_folder() { return m_folder; }
	InfSessionProxy* get_proxy() { return m_proxy; }

private:
	Folder& m_folder;
	InfBrowser* m_browser;
	InfBrowserIter m_browser_iter;
	InfSessionProxy* m_proxy;
};

Gobby::FolderManager::FolderManager(Browser& browser,
                                    DocumentInfoStorage& info_storage,
                                    Folder& text_folder,
                                    Folder& chat_folder):
	m_browser(browser), m_info_storage(info_storage),
	m_text_folder(text_folder), //false, preferences, m_lang_manager),
	m_chat_folder(chat_folder) //true, preferences, m_lang_manager)
{
	InfGtkBrowserModel* model =
		INF_GTK_BROWSER_MODEL(browser.get_store());

	m_set_browser_handler = g_signal_connect(
		G_OBJECT(model), "set-browser",
		G_CALLBACK(&on_set_browser_static), this);

	m_text_document_added_connection = 
		m_text_folder.signal_document_added().connect(
			sigc::mem_fun(
				*this,
				&FolderManager::on_text_document_added));
	m_chat_document_added_connection =
		m_chat_folder.signal_document_added().connect(
			sigc::mem_fun(
				*this,
				&FolderManager::on_chat_document_added));

	m_text_folder.signal_document_removed().connect(
		sigc::mem_fun(
			*this, &FolderManager::on_document_removed));
	m_chat_folder.signal_document_removed().connect(
		sigc::mem_fun(
			*this, &FolderManager::on_document_removed));
}

Gobby::FolderManager::~FolderManager()
{
	for(BrowserMap::iterator iter = m_browser_map.begin();
	    iter != m_browser_map.end(); ++ iter)
	{
		delete iter->second;
	}

	for(SessionMap::iterator iter = m_session_map.begin();
	    iter != m_session_map.end(); ++iter)
	{
		delete iter->second;
	}

	g_signal_handler_disconnect(
		INF_GTK_BROWSER_MODEL(m_browser.get_store()),
		m_set_browser_handler);
}

void Gobby::FolderManager::add_document(InfBrowser* browser,
                                        const InfBrowserIter* iter,
                                        InfSessionProxy* proxy,
                                        UserJoinRef userjoin)
{
	gchar* hostname;

	if(INFC_IS_BROWSER(browser))
	{
		InfXmlConnection* connection =
			infc_browser_get_connection(INFC_BROWSER(browser));
		g_object_get(
			G_OBJECT(connection),
			"remote-hostname", &hostname, NULL);
	}
	else if(INFD_IS_DIRECTORY(browser))
	{
		hostname = g_strdup(g_get_host_name());
	}
	else
	{
		g_assert_not_reached();
		hostname = NULL;
	}

	InfSession* session;
	g_object_get(G_OBJECT(proxy), "session", &session, NULL);

	TextSessionView* text_view = NULL;

	Folder* folder;
	SessionView* view;

	if(INF_TEXT_IS_SESSION(session))
	{
		gchar* path = inf_browser_get_path(browser, iter);

		m_text_document_added_connection.block();
		text_view = &m_text_folder.add_text_session(
			INF_TEXT_SESSION(session),
			inf_browser_get_node_name(browser, iter),
			path, hostname,
			m_info_storage.get_key(browser, iter));
		m_text_document_added_connection.unblock();

		folder = &m_text_folder;
		view = text_view;

		g_free(path);
	}
	else if(INF_IS_CHAT_SESSION(session))
	{
		if(iter == NULL)
		{
			m_chat_document_added_connection.block();
			view = &m_chat_folder.add_chat_session(
				INF_CHAT_SESSION(session),
				hostname, "", hostname);
			m_chat_document_added_connection.unblock();
		}
		else
		{
			gchar* path = inf_browser_get_path(browser, iter);

			m_chat_document_added_connection.block();
			view = &m_chat_folder.add_chat_session(
				INF_CHAT_SESSION(session),
				inf_browser_get_node_name(browser, iter),
				path,
				hostname);
			m_chat_document_added_connection.unblock();

			g_free(path);
		}

		folder = &m_chat_folder;
	}
	else
	{
		// Cannot happen, because we don't have any other note plugins
		// installed in the browser.
		g_assert_not_reached();
	}

	g_free(hostname);

	// Highlight the newly created session
	folder->switch_to_document(*view);
	if(text_view)
		gtk_widget_grab_focus(GTK_WIDGET(text_view->get_text_view()));
	if(iter) m_browser.set_selected(browser, iter);

	g_assert(m_session_map.find(session) == m_session_map.end());
	m_session_map[session] =
		new SessionInfo(*folder, browser, iter, proxy);
	g_object_unref(session);

	m_signal_document_added.emit(
		browser, iter, proxy, *folder, *view, userjoin);
}

void Gobby::FolderManager::remove_document(SessionView& view)
{
	SessionMap::const_iterator iter = m_session_map.find(view.get_session());
	g_assert(iter != m_session_map.end());

	iter->second->get_folder().remove_document(view);
}

Gobby::SessionView*
Gobby::FolderManager::lookup_document(InfSession* session) const
{
	SessionMap::const_iterator iter = m_session_map.find(session);
	if(iter == m_session_map.end()) return NULL;

	return iter->second->get_folder().lookup_document(session);
}

void Gobby::FolderManager::switch_to_document(SessionView& view)
{
	SessionMap::iterator iter = m_session_map.find(view.get_session());
	g_assert(iter != m_session_map.end());
	iter->second->get_folder().switch_to_document(view);
}

void Gobby::FolderManager::on_set_browser(InfGtkBrowserModel* model,
                                          GtkTreeIter* iter,
                                          InfBrowser* old_browser,
                                          InfBrowser* new_browser)
{
	if(old_browser != NULL)
	{
		BrowserMap::iterator iter = m_browser_map.find(old_browser);
		g_assert(iter != m_browser_map.end());
		delete iter->second;
		m_browser_map.erase(iter);
	}

	if(new_browser != NULL)
	{
		g_assert(m_browser_map.find(new_browser) ==
		         m_browser_map.end());
		m_browser_map[new_browser] =
			new BrowserInfo(*this, new_browser);
	}
}

void Gobby::FolderManager::on_unsubscribe_session(InfBrowser* browser,
                                                  const InfBrowserIter* iter,
                                                  InfSessionProxy* proxy,
                                                  InfRequest* request)
{
	InfSession* session;
	g_object_get(G_OBJECT(proxy), "session", &session, NULL);
	SessionMap::iterator session_iter = m_session_map.find(session);

	// Note that the session might not be in the session map, for example
	// if we are subscribed but do not show a document for the session.
	if(session_iter != m_session_map.end())
	{
		// Browser entry disappeared for the session, so invalidate
		// our iterator. The session is now "floating", without any
		// browser entry.
		session_iter->second->reset_browser();

		// When the subscription group of the session is still
		// available, then close the session. This can essentially
		// only happen on the server side, and it closes the session
		// when it is removed from the document tree. In principle
		// the session could go on until the document is closed by
		// the server, but this probably causes more confusion than
		// anything else. So we close it.
		// TODO: It would be nice to keep the session itself alive,
		// and to only unsubscribe all clients and set the local users
		// to unavailable.
		if(inf_session_get_subscription_group(session) != NULL)
		{
			lookup_document(session)->set_info(
				_("The document has been removed from the server."),
				true);

			inf_session_close(session);
		}
	}

	g_object_unref(session);
}

void Gobby::FolderManager::on_text_document_added(SessionView& view)
{
	// This should not happen, because we only hand out const folders.
	g_assert_not_reached();

	InfSession* session = view.get_session();

	// Do as if the document was added "normally", but we cannot associate
	// either a browser entry or a session proxy.
	g_assert(m_session_map.find(session) == m_session_map.end());
	m_session_map[session] =
		new SessionInfo(m_text_folder, NULL, NULL, NULL);
	m_signal_document_added.emit(NULL, NULL, NULL,
	                             m_text_folder, view, NULL);
}

void Gobby::FolderManager::on_chat_document_added(SessionView& view)
{
	// This should not happen, because we only hand out const folders.
	g_assert_not_reached();

	InfSession* session = view.get_session();

	// Do as if the document was added "normally", but we cannot associate
	// either a browser entry or a session proxy.
	g_assert(m_session_map.find(session) == m_session_map.end());
	m_session_map[session] =
		new SessionInfo(m_chat_folder, NULL, NULL, NULL);
	m_signal_document_added.emit(NULL, NULL, NULL,
	                             m_chat_folder, view, NULL);
}

void Gobby::FolderManager::on_document_removed(SessionView& view)
{
	InfSession* session = view.get_session();
	SessionMap::iterator session_iter = m_session_map.find(session);

	g_assert(session_iter != m_session_map.end());

	Folder& folder = session_iter->second->get_folder();
	InfBrowser* browser = session_iter->second->get_browser();
	InfSessionProxy* proxy = session_iter->second->get_proxy();
	const InfBrowserIter* iter = session_iter->second->get_browser_iter();

	InfBrowserIter local_iter;
	if(iter != NULL) local_iter = *iter;

	g_object_ref(proxy);
	delete session_iter->second;
	m_session_map.erase(session_iter);

	m_signal_document_removed.emit(
		browser, browser != NULL ? &local_iter : NULL,
		proxy, folder, view);

	g_object_unref(proxy);
}
