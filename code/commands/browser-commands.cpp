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

#include "commands/browser-commands.hpp"
#include "util/i18n.hpp"

#include <libinfinity/common/inf-request-result.h>
#include <libinfinity/server/infd-directory.h>

namespace
{
	void count_available_users_func(InfUser* user, gpointer user_data)
	{
		if(inf_user_get_status(user) != INF_USER_UNAVAILABLE &&
		   ~inf_user_get_flags(user) & INF_USER_LOCAL)
		{
			++(*(unsigned int*)user_data);
		}
	}

	unsigned int count_available_users(InfUserTable* user_table)
	{
		// Count available remote users
		unsigned int n_users = 0;
		inf_user_table_foreach_user(
			user_table, count_available_users_func, &n_users);
		return n_users;
	}

	class ParameterProvider: public Gobby::UserJoin::ParameterProvider
	{
	public:
		ParameterProvider(const Gobby::Preferences& preferences):
			m_preferences(preferences)
		{
		}

		virtual std::vector<GParameter> get_user_join_parameters();

		const Gobby::Preferences& m_preferences;
	};

	std::vector<GParameter> ParameterProvider::get_user_join_parameters()
	{
		std::vector<GParameter> params;
		const GParameter name_param = { "name", { 0 } };
		params.push_back(name_param);
		const GParameter status_param = { "status", { 0 } };
		params.push_back(status_param);

		g_value_init(&params[0].value, G_TYPE_STRING);
		g_value_init(&params[1].value, INF_TYPE_USER_STATUS);

		const Glib::ustring& name = m_preferences.user.name;
		g_value_set_string(&params[0].value, name.c_str());
		g_value_set_enum(&params[1].value, INF_USER_INACTIVE);

		return params;
	}
}

class Gobby::BrowserCommands::BrowserInfo
{
public:
	BrowserInfo(BrowserCommands& commands,
	            InfBrowser* browser);

	~BrowserInfo();

	InfBrowser* get_browser() { return m_browser; }

	void set_pending_chat(InfSessionProxy* proxy);
	void clear_pending_chat();
	void check_pending_chat();
private:
	static void on_notify_status_static(GObject* object,
	                                    GParamSpec* pspec,
	                                    gpointer user_data)
	{
		static_cast<BrowserCommands*>(user_data)->on_notify_status(
			INF_BROWSER(object));
	}

	static void on_add_available_user_static(InfUserTable* user_table,
	                                         InfUser* user,
	                                         gpointer user_data)
	{
		static_cast<BrowserInfo*>(user_data)->check_pending_chat();
	}

	BrowserCommands& m_commands;
	std::auto_ptr<UserJoin> m_userjoin;

	InfBrowser* m_browser;
	gulong m_notify_status_handler;

	InfSessionProxy* m_pending_chat;
	gulong m_add_available_user_handler;
};

class Gobby::BrowserCommands::RequestInfo
{
	friend class BrowserCommands;
public:
	RequestInfo(BrowserCommands& commands,
	            InfBrowser* browser, InfBrowserIter* iter,
	            StatusBar& status_bar);
	~RequestInfo();

	InfBrowser* get_browser() { return m_browser; }

	void set_request(InfRequest* request);
private:
	static void on_node_finished_static(InfRequest* request,
	                                    const InfRequestResult* result,
	                                    const GError* error,
	                                    gpointer user_data)
	{
		const InfBrowserIter* iter = NULL;
		RequestInfo* info = static_cast<RequestInfo*>(user_data);

		if(error == NULL)
		{
			inf_request_result_get_subscribe_session(
				result, NULL, &iter, NULL);

			g_assert(iter == NULL ||
			         iter->node == info->m_iter.node);
			g_assert(iter == NULL ||
		        	 iter->node_id == info->m_iter.node_id);
		}

		info->m_commands.on_finished(
			INF_REQUEST(request),
			info->m_browser, iter, error);
	}
	
	static void on_chat_finished_static(InfRequest* request,
	                                    const InfRequestResult* result,
	                                    const GError* error,
	                                    gpointer user_data)
	{
		RequestInfo* info = static_cast<RequestInfo*>(user_data);

		info->m_commands.on_finished(
			INF_REQUEST(request), info->m_browser, NULL, error);
	}

	BrowserCommands& m_commands;
	InfBrowser* m_browser;
	InfBrowserIter m_iter;

	StatusBar& m_status_bar;
	StatusBar::MessageHandle m_handle;

	InfRequest* m_request;
	gulong m_finished_handler;
};

Gobby::BrowserCommands::BrowserInfo::BrowserInfo(BrowserCommands& cmds,
                                                 InfBrowser* browser):
	m_commands(cmds), m_browser(browser), m_pending_chat(NULL)
{
	m_notify_status_handler = g_signal_connect(
		m_browser, "notify::status",
		G_CALLBACK(on_notify_status_static), &cmds);

	g_object_ref(browser);
}

Gobby::BrowserCommands::BrowserInfo::~BrowserInfo()
{
	if(m_pending_chat != NULL)
	{
		InfSession* session;
		g_object_get(G_OBJECT(m_pending_chat), "session", &session, NULL);
		InfUserTable* table = inf_session_get_user_table(session);

		g_signal_handler_disconnect(
			table, m_add_available_user_handler);

		g_object_unref(session);
		g_object_unref(m_pending_chat);
	}

	g_signal_handler_disconnect(m_browser, m_notify_status_handler);
	g_object_unref(m_browser);
}

void
Gobby::BrowserCommands::BrowserInfo::set_pending_chat(InfSessionProxy* proxy)
{
	g_assert(m_pending_chat == NULL);

	m_pending_chat = proxy;
	g_object_ref(proxy);

	InfSession* session;
	g_object_get(G_OBJECT(proxy), "session", &session, NULL);
	InfUserTable* table = inf_session_get_user_table(session);

	m_add_available_user_handler = g_signal_connect(
		G_OBJECT(table), "add-available-user",
		G_CALLBACK(on_add_available_user_static), this);

	g_object_unref(session);

	// Attempt a user join already here before the document is added to
	// the folder manager so that we can correctly show the document when
	// enough users are available. Otherwise if no party would join a user
	// before other users show up, the chat would never be shown.
	std::auto_ptr<UserJoin::ParameterProvider> provider(
		new ParameterProvider(m_commands.m_preferences));
	m_userjoin.reset(new UserJoin(m_browser, NULL, proxy, provider));

	check_pending_chat();
}

void Gobby::BrowserCommands::BrowserInfo::clear_pending_chat()
{
	if(m_pending_chat != NULL)
	{
		InfSession* session;
		g_object_get(G_OBJECT(m_pending_chat),
		             "session", &session, NULL);
		InfUserTable* table = inf_session_get_user_table(session);

		g_signal_handler_disconnect(
			table, m_add_available_user_handler);

		m_userjoin.reset(NULL);
		g_object_unref(m_pending_chat);
		m_pending_chat = NULL;

		g_object_unref(session);
	}
}

void Gobby::BrowserCommands::BrowserInfo::check_pending_chat()
{
	g_assert(m_pending_chat != NULL);

	InfSession* session;
	g_object_get(G_OBJECT(m_pending_chat), "session", &session, NULL);
	InfUserTable* table = inf_session_get_user_table(session);

	if(count_available_users(table) > 0)
	{
		g_signal_handler_disconnect(
			table, m_add_available_user_handler);

		m_commands.m_folder_manager.add_document(
			m_browser, NULL, m_pending_chat, &m_userjoin);

		g_object_unref(m_pending_chat);
		m_pending_chat = NULL;
	}

	g_object_unref(session);
}

Gobby::BrowserCommands::RequestInfo::RequestInfo(BrowserCommands& commands,
                                                 InfBrowser* browser,
                                                 InfBrowserIter* iter,
                                                 StatusBar& status_bar):
	m_commands(commands), m_browser(browser), m_status_bar(status_bar),
	m_request(NULL), m_finished_handler(0)
{
	if(iter)
	{
		m_iter = *iter;

		m_handle = m_status_bar.add_info_message(
			Glib::ustring::compose(
				_("Subscribing to %1..."), Glib::ustring(
					inf_browser_get_node_name(
						browser, iter))));
	}
	else
	{
		InfXmlConnection* connection =
			infc_browser_get_connection(INFC_BROWSER(browser));
		gchar* remote_hostname;
		g_object_get(G_OBJECT(connection),
		             "remote-hostname", &remote_hostname, NULL);
		m_handle = m_status_bar.add_info_message(
			Glib::ustring::compose(
				_("Subscribing to chat on %1..."),
					remote_hostname));
		g_free(remote_hostname);
	}
}

Gobby::BrowserCommands::RequestInfo::~RequestInfo()
{
	m_status_bar.remove_message(m_handle);

	if(m_request != NULL)
	{
		g_signal_handler_disconnect(m_request, m_finished_handler);
		g_object_unref(m_request);
	}
}

void Gobby::BrowserCommands::RequestInfo::set_request(InfRequest* request)
{
	g_assert(m_request == NULL);

	m_finished_handler = g_signal_handler_find(
		request, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, this);
	g_assert(m_finished_handler != 0);
}

Gobby::BrowserCommands::BrowserCommands(Browser& browser,
                                        FolderManager& folder_manager,
                                        StatusBar& status_bar,
                                        Operations& operations,
                                        const Preferences& preferences):
	m_browser(browser), m_folder_manager(folder_manager),
	m_operations(operations), m_status_bar(status_bar),
	m_preferences(preferences)
{
	m_browser.signal_connect().connect(
		sigc::mem_fun(*this, &BrowserCommands::on_connect));
	m_browser.signal_activate().connect(
		sigc::mem_fun(*this, &BrowserCommands::on_activate));

	m_set_browser_handler = g_signal_connect(
		browser.get_store(), "set-browser",
		G_CALLBACK(on_set_browser_static), this);

	// Add already existing browsers
	GtkTreeIter iter;
	GtkTreeModel* treemodel = GTK_TREE_MODEL(browser.get_store());
	for(gboolean have_entry =
		gtk_tree_model_get_iter_first(treemodel, &iter);
	    have_entry == TRUE;
	    have_entry = gtk_tree_model_iter_next(treemodel, &iter))
	{
		InfBrowser* browser;

		gtk_tree_model_get(
			treemodel, &iter,
			INF_GTK_BROWSER_MODEL_COL_BROWSER, &browser,
			-1);

		InfBrowserStatus browser_status;
		g_object_get(
			G_OBJECT(browser), "status",
			&browser_status, NULL);

		m_browser_map[browser] =
			new BrowserInfo(*this, browser);
		if(browser_status == INF_BROWSER_OPEN)
			if(!create_chat_document(browser))
				subscribe_chat(browser);
		g_object_unref(browser);
	}
}

Gobby::BrowserCommands::~BrowserCommands()
{
	for(RequestMap::iterator iter = m_request_map.begin();
	    iter != m_request_map.end(); ++ iter)
	{
		delete iter->second;
	}

	for(BrowserMap::iterator iter = m_browser_map.begin();
	    iter != m_browser_map.end(); ++iter)
	{
		delete iter->second;
	}

	g_signal_handler_disconnect(m_browser.get_store(),
	                            m_set_browser_handler);
}

void Gobby::BrowserCommands::on_set_browser(InfGtkBrowserModel* model,
                                            GtkTreeIter* iter,
                                            InfBrowser* old_browser,
                                            InfBrowser* new_browser)
{
	if(old_browser != NULL)
	{
		// Find by browser in case old_browser has its connection
		// reset.
		BrowserMap::iterator iter = m_browser_map.find(old_browser);
		g_assert(iter != m_browser_map.end());

		delete iter->second;
		m_browser_map.erase(iter);
	}

	if(new_browser != NULL)
	{
		g_assert(m_browser_map.find(new_browser) ==
		         m_browser_map.end());

		InfBrowserStatus browser_status;
		g_object_get(
			G_OBJECT(new_browser), "status",
			&browser_status, NULL);

		m_browser_map[new_browser] =
			new BrowserInfo(*this, new_browser);
		if(browser_status == INF_BROWSER_OPEN)
			if(!create_chat_document(new_browser))
				subscribe_chat(new_browser);
	}
}

void Gobby::BrowserCommands::on_notify_status(InfBrowser* browser)
{
	InfXmlConnection* connection;
	InfXmlConnectionStatus status;
	InfBrowserStatus browser_status;

	const BrowserMap::iterator iter = m_browser_map.find(browser);
	g_assert(iter != m_browser_map.end());

	g_object_get(G_OBJECT(browser), "status", &browser_status, NULL);
	switch(browser_status)
	{
	case INF_BROWSER_CLOSED:
		iter->second->clear_pending_chat();

		// Close connection if browser got disconnected. This for
		// example happens when the server does not send an initial
		// welcome message.
		if(INFC_IS_BROWSER(browser))
		{
			connection = infc_browser_get_connection(
				INFC_BROWSER(browser));
			g_object_get(G_OBJECT(connection),
			             "status", &status, NULL);
			if(status != INF_XML_CONNECTION_CLOSED &&
			   status != INF_XML_CONNECTION_CLOSING)
			{
				inf_xml_connection_close(connection);
			}
		}

		break;
	case INF_BROWSER_OPENING:
		break;
	case INF_BROWSER_OPEN:
		if(!create_chat_document(browser))
			subscribe_chat(browser);
		break;
	default:
		g_assert_not_reached();
		break;
	}
}

void Gobby::BrowserCommands::subscribe_chat(InfBrowser* browser)
{
	if(INFC_IS_BROWSER(browser))
	{
		std::auto_ptr<RequestInfo> info(new RequestInfo(
			*this, INF_BROWSER(browser), NULL, m_status_bar));

		InfRequest* request = INF_REQUEST(
			infc_browser_subscribe_chat(
				INFC_BROWSER(browser),
				RequestInfo::on_chat_finished_static,
				info.get()));

		if(request != NULL)
		{
			info->set_request(request);
			g_assert(m_request_map.find(request) == m_request_map.end());
			m_request_map[request] = info.release();
		}
	}
	else if(INFD_IS_DIRECTORY(browser))
	{
		infd_directory_enable_chat(INFD_DIRECTORY(browser), TRUE);
		create_chat_document(browser);
	}
}

bool Gobby::BrowserCommands::create_chat_document(InfBrowser* browser)
{
	// Get the chat session from the browser
	InfSessionProxy* proxy = NULL;
	if(INFC_IS_BROWSER(browser))
	{
		proxy = INF_SESSION_PROXY(
			infc_browser_get_chat_session(INFC_BROWSER(browser)));
	}
	else if(INFD_IS_DIRECTORY(browser))
	{
		proxy = INF_SESSION_PROXY(
			infd_directory_get_chat_session(
				INFD_DIRECTORY(browser)));
	}

	// Chat session not available
	if(!proxy) return false;

	// First, check whether there is already a document for this
	// chat session. If there is, then don't create another one.
	InfSession* session;
	g_object_get(G_OBJECT(proxy), "session", &session, NULL);

	SessionView* view =
		m_folder_manager.get_chat_folder().lookup_document(session);
	g_object_unref(session);

	g_assert(view == NULL); // Can it happen?
	if(view) return true; // Document exists already

	if(INFC_IS_BROWSER(browser))
	{
		// For clients, just add the document
		m_folder_manager.add_document(
			browser, NULL, proxy, NULL);
		return true;
	}
	else if(INFD_IS_DIRECTORY(browser))
	{
		// For our local chat, only show it if there is at least
		// one remote user, otherwise wait until users show up.
		BrowserMap::iterator iter =
			m_browser_map.find(browser);
		g_assert(iter != m_browser_map.end());

		iter->second->set_pending_chat(proxy);
		return true;
	}

	// This cannot actually happen, but return to silence compiler
	// warnings
	g_assert_not_reached();
	return false;
}

void Gobby::BrowserCommands::on_connect(const Glib::ustring& hostname)
{
	m_operations.subscribe_path(hostname);
}

void Gobby::BrowserCommands::on_activate(InfBrowser* browser,
                                         InfBrowserIter* iter)
{
	InfSessionProxy* proxy = inf_browser_get_session(browser, iter);

	if(proxy != NULL)
	{
		InfSession* session;
		g_object_get(G_OBJECT(proxy), "session", &session, NULL);
		SessionView* view = m_folder_manager.lookup_document(session);
		g_object_unref(session);

		if(view != NULL)
		{
			m_folder_manager.switch_to_document(*view);
		}
		else
		{
			m_folder_manager.add_document(browser, iter,
			                              proxy, NULL);
		}
	}
	else
	{
		InfRequest* request =
			inf_browser_get_pending_request(
				browser, iter, "subscribe-session");

		// If there is already a request don't re-request
		if(request == NULL)
		{
			std::auto_ptr<RequestInfo> info(new RequestInfo(
				*this, browser, iter, m_status_bar));

			request = INF_REQUEST(
				inf_browser_subscribe(
					browser, iter,
					RequestInfo::on_node_finished_static,
					info.get()));

			if(request != NULL)
			{
				info->set_request(request);
				g_assert(m_request_map.find(request) ==
				         m_request_map.end());
				m_request_map[request] = info.release();
			}
		}
	}
}

void Gobby::BrowserCommands::on_finished(InfRequest* request,
                                         InfBrowser* browser,
                                         const InfBrowserIter* iter,
                                         const GError* error)
{
	RequestMap::iterator map_iter = m_request_map.find(request);
	if(map_iter != m_request_map.end())
	{
		delete map_iter->second;
		m_request_map.erase(map_iter);
	}

	if(error != NULL)
	{
		m_status_bar.add_error_message(
			_("Subscription failed"),
			error->message);
	}
	else if(iter != NULL)
	{
		InfSessionProxy* proxy =
			inf_browser_get_session(browser, iter);
		g_assert(proxy != NULL);

		m_folder_manager.add_document(browser, iter, proxy, NULL);
	}
	else
	{
		// For InfdDirectory we do not make a request for subscribing
		// to the chat.
		const bool created = create_chat_document(browser);
		g_assert(created); // Must work now
	}
}
