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

#include "commands/user-join-commands.hpp"
#include "core/nodewatch.hpp"
#include "util/i18n.hpp"

#include <glibmm/main.h>

#include <libinfinity/server/infd-session-proxy.h>
#include <libinfinity/common/inf-request-result.h>
#include <libinfinity/common/inf-error.h>

namespace
{
	inline const gchar* _(const gchar* msgid) { return Gobby::_(msgid); }

	void set_error_text(Gobby::SessionView& view,
	                    const Glib::ustring& initial_text)
	{
		using namespace Gobby;
		Glib::ustring type_text;

		// TODO: Adjust this for chat sessions
		type_text = _("You can still watch others editing "
		              "the document, but you cannot edit "
		              "it yourself.");

		const Glib::ustring info_text =
			_("If you have an idea what could have caused the "
			  "problem, then you may attempt to solve it and "
			  "try again (after having closed this document). "
			  "Otherwise it is most likely a bug in the "
			  "software. In that case, please file a bug report "
			  "at http://gobby.0x539.de/trac/newticket and "
			  "provide as much information as you can, including "
			  "what you did when the problem occurred and how to "
			  "reproduce the problem (if possible) so that we "
			  "can fix the problem in a later version. "
			  "Thank you.");

		view.set_info(
			Glib::ustring::compose(
				_("User Join failed: %1"), initial_text) +
			"\n\n" + type_text + "\n\n" + info_text, true);
	}

	void set_permission_denied_text(Gobby::SessionView& view)
	{
		view.set_info(
			_("Permissions are not granted to modify the document.") ,
			true);
	}

	void retr_local_user_func(InfUser* user, gpointer user_data)
	{
		(*static_cast<InfUser**>(user_data)) = user;
	}
}

class Gobby::UserJoinCommands::UserJoinInfo
{
public:
	UserJoinInfo(UserJoinCommands& commands,
	             InfBrowser* browser,
	             const InfBrowserIter* iter,
	             InfSessionProxy* proxy,
	             Folder& folder,
	             SessionView& view);
	~UserJoinInfo();

private:
	static void on_synchronization_complete_static(InfSession* session,
	                                               InfXmlConnection* conn,
	                                               gpointer user_data)
	{
		static_cast<UserJoinInfo*>(user_data)->
			on_synchronization_complete();
	}

	static void on_user_join_finished_static(InfRequest* request,
	                                         const InfRequestResult* res,
	                                         const GError* error,
	                                         gpointer user_data)
	{
		InfUser* user;
		inf_request_result_get_join_user(res, NULL, &user);

		static_cast<UserJoinInfo*>(user_data)->
			on_user_join_finished(user, error);
	}

	void on_synchronization_complete();
	void on_user_join_finished(InfUser* user, const GError* error);

	void attempt_user_join();
	void user_join_complete(InfUser* user);
	void finish();

	void add_text_user_properties(std::vector<GParameter>& params,
	                              TextSessionView& view);

	UserJoinCommands& m_commands;

	NodeWatch m_node;
	InfSessionProxy* m_proxy;
	Folder& m_folder;
	SessionView& m_view;

	InfRequest* m_request;

	gulong m_synchronization_complete_handler;

	guint m_retry_index;
};

Gobby::UserJoinCommands::UserJoinInfo::UserJoinInfo(UserJoinCommands& cmds,
                                                    InfBrowser* browser,
                                                    const InfBrowserIter* it,
                                                    InfSessionProxy* proxy,
                                                    Folder& folder,
                                                    SessionView& view):
	m_node(browser, it), m_commands(cmds), m_proxy(proxy),
	m_folder(folder), m_view(view), m_request(NULL),
	m_synchronization_complete_handler(0), m_retry_index(1)
{
	g_object_ref(m_proxy);

	InfSession* session;
	g_object_get(G_OBJECT(proxy), "session", &session, NULL);

	if(inf_session_get_status(session) == INF_SESSION_SYNCHRONIZING)
	{
		// If not yet synchronized, wait for synchronization until
		// attempting userjoin
		m_synchronization_complete_handler = g_signal_connect_after(
			G_OBJECT(session), "synchronization-complete",
			G_CALLBACK(on_synchronization_complete_static), this);
	}
	else
	{
		// Delay this call to make sure we don't call finish()
		// right inside the constructor.
		Glib::signal_idle().connect(
			sigc::bind_return(sigc::mem_fun(
				*this, &UserJoinInfo::attempt_user_join),
				false));
	}

	g_object_unref(session);
}

Gobby::UserJoinCommands::UserJoinInfo::~UserJoinInfo()
{
	if(m_synchronization_complete_handler)
	{
		InfSession* session;
		g_object_get(G_OBJECT(m_proxy), "session", &session, NULL);

		g_signal_handler_disconnect(
			session, m_synchronization_complete_handler);

		g_object_unref(session);
	}

	if(m_request)
	{
		g_signal_handlers_disconnect_by_func(
			G_OBJECT(m_request),
			(gpointer)G_CALLBACK(on_user_join_finished_static),
			this);

		g_object_unref(m_request);

		// TODO: Keep watching the request, and when it finishes, make
		// the user unavailable. This should typically not be
		// necessary, because on the server side user join requests
		// finish immediately, and on the client side the only thing
		// that leads to the UserJoinInfo being deleted is when the
		// document is removed and we are unsubscribed from the
		// session, in which case we do not care about the user join
		// anymore anyway.
		// However, it would be good to handle this, just in case.
	}

	g_object_unref(m_proxy);
}

void Gobby::UserJoinCommands::UserJoinInfo::on_synchronization_complete()
{
	// Disconnect signal handler, so that we don't get notified when
	// syncing this document in running state to another location
	// or server.
	InfSession* session;
	g_object_get(G_OBJECT(m_proxy), "session", &session, NULL);

	g_signal_handler_disconnect(session,
	                            m_synchronization_complete_handler);
	m_synchronization_complete_handler = 0;
	g_object_unref(session);

	// Attempt user join after synchronization
	attempt_user_join();
}

void Gobby::UserJoinCommands::UserJoinInfo::
	on_user_join_finished(InfUser* user, const GError* error)
{
	if(m_request != NULL)
	{
		g_object_unref(m_request);
		m_request = NULL;
	}

	if(error == NULL)
	{
		user_join_complete(user);
	}
	else if(error->domain == inf_user_error_quark() &&
	        error->code == INF_USER_ERROR_NAME_IN_USE)
	{
		// If name is in use retry with alternative user name
		++m_retry_index;
		attempt_user_join();
	}
	else if(error->domain == inf_request_error_quark() &&
	        error->code == INF_REQUEST_ERROR_NOT_AUTHORIZED)
	{
		set_permission_denied_text(m_view);
		finish();
	}
	else
	{
		set_error_text(m_view, error->message);
		finish();
	}
}

void Gobby::UserJoinCommands::UserJoinInfo::attempt_user_join()
{
	const Preferences& preferences = m_commands.m_preferences;

	// Check if there is already a local user, for example for a
	// synced-in document.
	InfSession* session;
	g_object_get(G_OBJECT(m_proxy), "session", &session, NULL);

	InfUserTable* user_table = inf_session_get_user_table(session);
	InfUser* user = NULL;
	inf_user_table_foreach_local_user(user_table,
	                                  retr_local_user_func, &user);
	g_object_unref(session);

	if(user != NULL)
	{
		user_join_complete(user);
		return;
	}

	// Next, check whether we are allowed to join a user
	if(m_node.get_browser() && m_node.get_browser_iter())
	{
		InfBrowser* browser = m_node.get_browser();
		const InfBrowserIter* iter = m_node.get_browser_iter();
		const InfAclAccount* account =
			inf_browser_get_acl_local_account(browser);
		InfAclMask msk;
		inf_acl_mask_set1(&msk, INF_ACL_CAN_JOIN_USER);
		if(!inf_browser_check_acl(browser, iter, account, &msk, NULL))
		{
			set_permission_denied_text(m_view);
			finish();
			return;
		}
	}

	// We are allowed, so attempt to join the user now.
	std::vector<GParameter> params;
	const GParameter name_param = { "name", { 0 } };
	params.push_back(name_param);
	const GParameter status_param = { "status", { 0 } };
	params.push_back(status_param);

	g_value_init(&params[0].value, G_TYPE_STRING);
	g_value_init(&params[1].value, INF_TYPE_USER_STATUS);

	const Glib::ustring& pref_name = preferences.user.name;
	if(m_retry_index > 1)
	{
		gchar* name = g_strdup_printf(
			"%s %u", pref_name.c_str(), m_retry_index);
		g_value_take_string(&params[0].value, name);
	}
	else
	{
		g_value_set_static_string(
			&params[0].value, pref_name.c_str());
	}

	if(m_folder.get_current_document() == &m_view)
		g_value_set_enum(&params[1].value, INF_USER_ACTIVE);
	else
		g_value_set_enum(&params[1].value, INF_USER_INACTIVE);

	// Extra properties for text session:
	TextSessionView* text_view =
		dynamic_cast<TextSessionView*>(&m_view);
	if(text_view) add_text_user_properties(params, *text_view);

	GError* error = NULL;
	InfRequest* request = inf_session_proxy_join_user(
		m_proxy, params.size(), &params[0],
		on_user_join_finished_static, this);

	for(unsigned int i = 0; i < params.size(); ++i)
		g_value_unset(&params[i].value);

	if(request != NULL)
	{
		m_request = request;
		g_object_ref(m_request);
		m_view.set_info(_("User Join in progress..."), false);
	}
}

void Gobby::UserJoinCommands::UserJoinInfo::user_join_complete(InfUser* user)
{
	// TODO: Notify the user about alternative user name if s/he uses any
	m_view.unset_info();

	// TODO: set_active_user should maybe go to SessionView base:
	TextSessionView* text_view = dynamic_cast<TextSessionView*>(&m_view);
	if(text_view)
		text_view->set_active_user(INF_TEXT_USER(user));
	ChatSessionView* chat_view = dynamic_cast<ChatSessionView*>(&m_view);
	if(chat_view)
		chat_view->set_active_user(user);

	finish();
}

void Gobby::UserJoinCommands::UserJoinInfo::finish()
{
	UserJoinCommands::UserJoinMap::iterator iter =
		m_commands.m_user_join_map.find(m_proxy);
	g_assert(iter != m_commands.m_user_join_map.end());

	m_commands.m_user_join_map.erase(iter);
	delete this;
}

void Gobby::UserJoinCommands::UserJoinInfo::
	add_text_user_properties(std::vector<GParameter>& params,
	                         TextSessionView& view)
{
	InfTextSession* session = view.get_session();

	GParameter hue_param = { "hue", { 0 } };
	g_value_init(&hue_param.value, G_TYPE_DOUBLE);
	g_value_set_double(&hue_param.value,
	                   m_commands.m_preferences.user.hue);
	params.push_back(hue_param);

	GParameter vector_param = { "vector", { 0 } };
	g_value_init(&vector_param.value, INF_ADOPTED_TYPE_STATE_VECTOR);

	g_value_take_boxed(&vector_param.value, inf_adopted_state_vector_copy(
		inf_adopted_algorithm_get_current(
			inf_adopted_session_get_algorithm(
				INF_ADOPTED_SESSION(session)))));
	params.push_back(vector_param);

	GParameter caret_param = { "caret-position", { 0 } };
	g_value_init(&caret_param.value, G_TYPE_UINT);

	GtkTextBuffer* buffer = GTK_TEXT_BUFFER(view.get_text_buffer());
	GtkTextMark* mark = gtk_text_buffer_get_insert(buffer);
	GtkTextIter caret_iter;

	gtk_text_buffer_get_iter_at_mark(buffer, &caret_iter, mark);
	g_value_set_uint(&caret_param.value,
	                 gtk_text_iter_get_offset(&caret_iter));
	params.push_back(caret_param);
}

Gobby::UserJoinCommands::UserJoinCommands(FolderManager& folder_manager,
	                                  const Preferences& preferences):
	m_preferences(preferences)
{
	folder_manager.signal_document_added().connect(
		sigc::mem_fun(
			*this, &UserJoinCommands::on_document_added));
	folder_manager.signal_document_removed().connect(
		sigc::mem_fun(
			*this, &UserJoinCommands::on_document_removed));
}

Gobby::UserJoinCommands::~UserJoinCommands()
{
	for(UserJoinMap::iterator iter = m_user_join_map.begin();
	    iter != m_user_join_map.end(); ++iter)
	{
		delete iter->second;
	}
}

void Gobby::UserJoinCommands::on_document_added(InfBrowser* browser,
                                                const InfBrowserIter* iter,
                                                InfSessionProxy* proxy,
                                                Folder& folder,
                                                SessionView& view)
{
	g_assert(proxy != NULL);

	g_assert(m_user_join_map.find(proxy) == m_user_join_map.end());
	m_user_join_map[proxy] =
		new UserJoinInfo(*this, browser, iter, proxy, folder, view);
}

void Gobby::UserJoinCommands::on_document_removed(InfBrowser* browser,
                                                  const InfBrowserIter* iter,
                                                  InfSessionProxy* proxy,
                                                  Folder& folder,
                                                  SessionView& view)
{
	g_assert(proxy != NULL);

	UserJoinMap::iterator user_iter = m_user_join_map.find(proxy);

	// If the user join was successful the session is no longer in the map
	if(user_iter != m_user_join_map.end())
	{
		delete user_iter->second;
		m_user_join_map.erase(user_iter);
	}
	else
	{
		// The user has removed the document. What we do now depends
		// on whether we are hosting the document or whether we are a
		// client. If we are a client we reset the connection of the
		// session proxy, which basically leads to us being
		// unsubscribed from the document. If we are hosting the
		// document, we do not want to unsubscribe from it, since
		// other users might still be connected. We therefore only
		// remove the local user from the session. If there are indeed
		// no other clients, then InfdDirectory will take care of
		// unsubscribing the session 60s after it became idle.
		if(INFD_IS_SESSION_PROXY(proxy))
		{
			InfUser* user = view.get_active_user();
			if(user != NULL)
			{
				InfSession* session;
				g_object_get(G_OBJECT(proxy), "session",
				             &session, NULL);
				inf_session_set_user_status(
					session, user, INF_USER_UNAVAILABLE);
				g_object_unref(session);

				// TODO: set_active_user should go to
				// SessionView base:
				// TODO: The libinftextgtk objects should
				// reset the active user automatically when it
				// becomes unavailable.
				TextSessionView* text_view =
					dynamic_cast<TextSessionView*>(&view);
				if(text_view)
					text_view->set_active_user(NULL);
				ChatSessionView* chat_view =
					dynamic_cast<ChatSessionView*>(&view);
				if(chat_view)
					chat_view->set_active_user(NULL);
			}
		}
		else if(INFC_IS_SESSION_PROXY(proxy))
		{
			infc_session_proxy_set_connection(
				INFC_SESSION_PROXY(proxy), NULL, NULL, 0);
		}
	}
}
