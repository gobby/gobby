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

#include "commands/browser-commands.hpp"
#include "util/i18n.hpp"

#include <libinftext/inf-text-user.h>

namespace
{
	const char* GOBBY_BROWSER_COMMANDS_SESSION_PROXY =
		"GOBBY_BROWSER_COMMANDS_SESSION_PROXY";

	enum ErrorType {
		SYNC_ERROR,
		USER_JOIN_ERROR
	};

	void set_error_text(Gobby::DocWindow& window,
	                    const Glib::ustring& initial_text,
	                    ErrorType type)
	{
		using namespace Gobby;
		Glib::ustring type_text;

		switch(type)
		{
		case SYNC_ERROR:
			// Document cannot be used if an error happened
			// during synchronization.
			type_text = _("This document cannot be used.");
			break;
		case USER_JOIN_ERROR:
			type_text = _("You can still watch others editing "
			              "the document, but you cannot edit "
			              "it yourself.");
			break;
		}

		const Glib::ustring info_text =
			_("If you have an idea what could have caused the "
			  "problem, then you may attempt to solve it and try "
			  "try again (after having closed this document). "
			  "Otherwise it is most likely a bug in the "
			  "software. In that case, please file a bug report "
			  "at http://gobby.0x539.de/trac/newticket and "
			  "provide as much information as you can, including "
			  "what you did when the problem occurred and how to "
			  "reproduce the problem (if possible) so that we "
			  "can fix the problem in a later version. "
			  "Thank you.");

		window.set_info(
			initial_text + "\n\n" + type_text + "\n\n" +
			info_text);
	}
}

// These need default and copy constructors to satisfy the map properties.
// However, we only do copy them as long as they are unset.
Gobby::BrowserCommands::RequestNode::RequestNode():
	commands(NULL) {}

Gobby::BrowserCommands::RequestNode::RequestNode(const RequestNode& node):
	commands(node.commands)
{
	g_assert(commands == NULL);
}

Gobby::BrowserCommands::RequestNode::~RequestNode()
{
	if(commands != NULL)
	{
		commands->m_status_bar.remove_message(handle);
	}
}

Gobby::BrowserCommands::SessionNode::SessionNode():
	proxy(NULL) {}

Gobby::BrowserCommands::SessionNode::SessionNode(const SessionNode& node):
	proxy(node.proxy)
{
	g_assert(proxy == NULL);
}

Gobby::BrowserCommands::SessionNode::~SessionNode()
{
	if(proxy != NULL)
	{
		InfSession* session = infc_session_proxy_get_session(proxy);
		g_signal_handler_disconnect(session, failed_id);
		g_signal_handler_disconnect(session, complete_id);
		g_signal_handler_disconnect(session, progress_id);
		g_signal_handler_disconnect(session, close_id);

		g_object_unref(proxy);
	}
}

Gobby::BrowserCommands::BrowserCommands(Browser& browser, Folder& folder,
                                        StatusBar& status_bar,
                                        const Preferences& preferences):
	m_browser(browser), m_folder(folder), m_status_bar(status_bar),
	m_preferences(preferences)
{
	m_browser.signal_activate().connect(
		sigc::mem_fun(*this, &BrowserCommands::on_activate));
}

void Gobby::BrowserCommands::on_activate(InfcBrowser* browser,
                                         InfcBrowserIter* iter)
{
	InfcSessionProxy* proxy =
		infc_browser_iter_get_session(browser, iter);
	if(proxy != NULL)
	{
		InfSession* session = infc_session_proxy_get_session(proxy);
		InfTextSession* text_session = INF_TEXT_SESSION(session);
		DocWindow* window = m_folder.lookup_document(text_session);

		if(window != NULL)
		{
			m_folder.switch_to_document(*window);
		}
		else
		{
			// This should not happen: We insert every document
			// we subscribe to directly into the folder.
			g_assert_not_reached();
			/*folder.add_document(
				text_session,
				infc_browser_iter_get_name(browser, iter));*/
		}
	}
	else
	{
		InfcNodeRequest* request =
			infc_browser_iter_get_subscribe_request(browser,
			                                        iter);

		// If there is already a request don't re-request
		if(request == NULL)
		{
			request = infc_browser_iter_subscribe_session(browser,
			                                              iter);

			RequestNode& node = m_request_map[request];
			node.commands = this;
			node.browser = browser;
			node.handle = m_status_bar.add_message(
				StatusBar::INFO,
				Glib::ustring::compose(
					_("Subscribing to %1…"),
					Glib::ustring(
						infc_browser_iter_get_name(
							browser, iter))), 0);

			g_signal_connect(
				request, "finished",
				G_CALLBACK(&on_finished_static), this);

			g_signal_connect(
				request, "failed",
				G_CALLBACK(&on_failed_static), this);
		}
		else
		{
			m_status_bar.add_message(
				StatusBar::INFO,
				_("Subscription already in progress"), 2);
		}
	}
}

void Gobby::BrowserCommands::on_finished(InfcNodeRequest* request)
{
	RequestMap::iterator iter = m_request_map.find(request);
	g_assert(iter != m_request_map.end());

	InfcBrowser* browser = iter->second.browser;
	InfcBrowserIter browser_iter;
	if(infc_browser_iter_from_node_request(browser, request,
	                                       &browser_iter))
	{
		InfcSessionProxy* proxy = infc_browser_iter_get_session(
			iter->second.browser, &browser_iter);
		InfSession* session = infc_session_proxy_get_session(proxy);

		DocWindow& window = m_folder.add_document(
			INF_TEXT_SESSION(session),
			infc_browser_iter_get_name(browser, &browser_iter));

		SessionNode& node = m_session_map[session];
		node.proxy = proxy;
		g_object_ref(proxy);

		node.failed_id = g_signal_connect(
			session, "synchronization-failed",
			G_CALLBACK(on_synchronization_failed_static), this);
		// Connect _after here so that we can access the 
		// AdoptedAlgorithm the default handler created to perform
		// the user join.
		node.complete_id = g_signal_connect_after(
			session, "synchronization-complete",
			G_CALLBACK(on_synchronization_complete_static), this);
		node.progress_id = g_signal_connect(
			session, "synchronization-progress",
			G_CALLBACK(on_synchronization_progress_static), this);
		node.close_id = g_signal_connect(
			session, "close", G_CALLBACK(on_close_static), this);

		InfXmlConnection* connection;
		g_object_get(G_OBJECT(session),
		             "sync-connection", &connection, NULL);

		gdouble percentage =
			inf_session_get_synchronization_progress(session,
			                                         connection);
		g_object_unref(connection);

		window.set_info(
			Glib::ustring::compose(
				_("Synchronization in progress… %1%%"),
				static_cast<unsigned int>(percentage * 100)));

	}

	m_request_map.erase(iter);
}

void Gobby::BrowserCommands::on_failed(InfcNodeRequest* request,
                                       const GError* error)
{
	RequestMap::iterator iter = m_request_map.find(request);
	g_assert(iter != m_request_map.end());

	m_request_map.erase(iter);

	m_status_bar.add_message(
		StatusBar::ERROR,
		Glib::ustring::compose(_("Subscription failed: %1"),
			error->message), 5);
}

void Gobby::BrowserCommands::on_synchronization_failed(InfSession* session,
                                                       InfXmlConnection* conn,
                                                       const GError* error)
{
	SessionMap::iterator iter = m_session_map.find(session);
	g_assert(iter != m_session_map.end());

	DocWindow* window = m_folder.lookup_document(
		INF_TEXT_SESSION(session));
	g_assert(window != NULL);

	set_error_text(
		*window,
		Glib::ustring::compose("Synchronization failed: %1",
		                       error->message), SYNC_ERROR);
}

void Gobby::BrowserCommands::on_synchronization_complete(InfSession* session,
                                                         InfXmlConnection* c)
{
	SessionMap::iterator iter = m_session_map.find(session);
	g_assert(iter != m_session_map.end());

	DocWindow* window = m_folder.lookup_document(
		INF_TEXT_SESSION(session));
	g_assert(window != NULL);

	// TODO: Automatically join with a different name if there is
	// already a user with the preferred name.
	GParameter params[4] = {
		{ "name", { 0 } },
		{ "hue", { 0 } },
		{ "vector", { 0 } },
		{ "caret-position", { 0 } }
	};

	g_value_init(&params[0].value, G_TYPE_STRING);
	g_value_init(&params[1].value, G_TYPE_DOUBLE);
	g_value_init(&params[2].value, INF_ADOPTED_TYPE_STATE_VECTOR);
	g_value_init(&params[3].value, G_TYPE_UINT);

	g_value_set_static_string(
		&params[0].value,
		static_cast<const Glib::ustring&>(
			m_preferences.user.name).c_str());
	g_value_set_double(
		&params[1].value, m_preferences.user.hue);
	g_value_take_boxed(
		&params[2].value,inf_adopted_state_vector_copy(
			inf_adopted_algorithm_get_current(
				inf_adopted_session_get_algorithm(
					INF_ADOPTED_SESSION(session)))));
	GtkTextBuffer* buffer = GTK_TEXT_BUFFER(window->get_text_buffer());
	GtkTextMark* mark = gtk_text_buffer_get_insert(buffer);
	GtkTextIter caret_iter;
	gtk_text_buffer_get_iter_at_mark(buffer, &caret_iter, mark);
	g_value_set_uint(&params[3].value,
	                 gtk_text_iter_get_offset(&caret_iter));

	GError* error = NULL;
	InfcUserRequest* request = infc_session_proxy_join_user(
		iter->second.proxy, params, 4, &error);
	
	g_value_unset(&params[0].value);
	g_value_unset(&params[1].value);
	g_value_unset(&params[2].value);
	g_value_unset(&params[3].value);

	if(request == NULL)
	{
		set_error_text(
			*window,
			Glib::ustring::compose("User Join failed: %1",
			                       error->message),
			USER_JOIN_ERROR);
	}
	else
	{
		window->set_info(
			_("Synchronization complete."
			  "User Join in progress…"));

		g_signal_connect(
			request, "failed",
			G_CALLBACK(on_user_join_failed_static), this);
		g_signal_connect(
			request, "finished",
			G_CALLBACK(on_user_join_finished_static), this);

		g_object_set_data(G_OBJECT(request),
		                  GOBBY_BROWSER_COMMANDS_SESSION_PROXY,
		                  iter->second.proxy);
	}
}

void Gobby::BrowserCommands::on_synchronization_progress(InfSession* session,
                                                         InfXmlConnection* c,
                                                         gdouble percentage)
{
	SessionMap::iterator iter = m_session_map.find(session);
	g_assert(iter != m_session_map.end());

	DocWindow* window = m_folder.lookup_document(
		INF_TEXT_SESSION(session));

	g_assert(window != NULL);
	window->set_info(
		Glib::ustring::compose(
			_("Synchronization in progress… %1%%"),
			static_cast<unsigned int>(percentage * 100)));
}

void Gobby::BrowserCommands::on_close(InfSession* session)
{
	SessionMap::iterator iter = m_session_map.find(session);
	g_assert(iter != m_session_map.end());
	m_session_map.erase(iter);
}

void Gobby::BrowserCommands::on_user_join_failed(InfcUserRequest* request,
                                                 const GError* error)
{
	gpointer proxy_ptr =
		g_object_get_data(G_OBJECT(request),
	                          GOBBY_BROWSER_COMMANDS_SESSION_PROXY);

	InfcSessionProxy* proxy = static_cast<InfcSessionProxy*>(proxy_ptr);
	DocWindow* window = m_folder.lookup_document(
		INF_TEXT_SESSION(infc_session_proxy_get_session(proxy)));
	g_assert(window != NULL);

	// TODO: Try join with another name if the name is already in use
	set_error_text(
		*window,
		Glib::ustring::compose("User Join failed: %1",
		                       error->message),
		USER_JOIN_ERROR);
}

void Gobby::BrowserCommands::on_user_join_finished(InfcUserRequest* request,
                                                   InfUser* user)
{
	g_assert(INF_TEXT_IS_USER(user));

	gpointer proxy_ptr =
		g_object_get_data(G_OBJECT(request),
	                          GOBBY_BROWSER_COMMANDS_SESSION_PROXY);

	InfcSessionProxy* proxy = static_cast<InfcSessionProxy*>(proxy_ptr);
	DocWindow* window = m_folder.lookup_document(
		INF_TEXT_SESSION(infc_session_proxy_get_session(proxy)));
	g_assert(window != NULL);

	window->unset_info();

	window->set_active_user(INF_TEXT_USER(user));
}
