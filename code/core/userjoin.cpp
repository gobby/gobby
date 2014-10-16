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

#include "core/userjoin.hpp"

#include <glibmm/main.h>

#include <cstring>

#include <libinfinity/common/inf-error.h>

namespace
{
	void retr_local_user_func(InfUser* user, gpointer user_data)
	{
		(*static_cast<InfUser**>(user_data)) = user;
	}

	std::vector<GParameter>::iterator find_name_param(
		std::vector<GParameter>& params)
	{
		for(std::vector<GParameter>::iterator iter = params.begin();
		    iter != params.end(); ++iter)
		{
			if(std::strcmp(iter->name, "name") == 0)
				return iter;
		}

		g_assert_not_reached();
		return params.end();
	}
}

Gobby::UserJoin::UserJoin(InfBrowser* browser, const InfBrowserIter* iter,
                          InfSessionProxy* proxy,
                          std::auto_ptr<ParameterProvider> param_provider):
	m_node(browser, iter), m_proxy(proxy),
	m_param_provider(param_provider),
	m_synchronization_complete_handler(0), m_request(NULL),
	m_retry_index(1), m_user(NULL), m_error(NULL)
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
		// Delay this call to make sure we don't emit the
		// finished signal right inside the constructor.
		// TODO: This might not be a problem, since the caller
		// can just check for completion with the get_user()
		// and get_error() methods.
		Glib::signal_idle().connect(
			sigc::bind_return(sigc::mem_fun(
				*this, &UserJoin::attempt_user_join),
				false));
	}

	g_object_unref(session);
}

Gobby::UserJoin::~UserJoin()
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

	if(m_error != NULL) g_error_free(m_error);

	g_object_unref(m_proxy);
}

void Gobby::UserJoin::UserJoin::on_synchronization_complete()
{
	// Disconnect signal handler, so that we don't get notified when
	// syncing this document in running state to another location
	// or server.
	InfSession* session;
	g_object_get(G_OBJECT(m_proxy), "session", &session, NULL);

	g_signal_handler_disconnect(
		session, m_synchronization_complete_handler);
	m_synchronization_complete_handler = 0;
	g_object_unref(session);

	// Attempt user join after synchronization
	attempt_user_join();
}

void Gobby::UserJoin::on_user_join_finished(InfUser* user,
                                            const GError* error)
{
	if(m_request != NULL)
	{
		g_signal_handlers_disconnect_by_func(
			G_OBJECT(m_request),
			(gpointer)G_CALLBACK(on_user_join_finished_static),
			this);
		g_object_unref(m_request);
		m_request = NULL;
	}

	if(error == NULL)
	{
		user_join_complete(user, error);
	}
	else if(error->domain == inf_user_error_quark() &&
	        error->code == INF_USER_ERROR_NAME_IN_USE)
	{
		// If name is in use retry with alternative user name
		++m_retry_index;
		attempt_user_join();
	}
	else
	{
		user_join_complete(user, error);
	}
}

void Gobby::UserJoin::attempt_user_join()
{
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
		user_join_complete(user, NULL);
		return;
	}

	// Next, check whether we are allowed to join a user
	if(m_node.get_browser() && m_node.get_browser_iter())
	{
		InfBrowser* browser = m_node.get_browser();
		const InfBrowserIter* iter = m_node.get_browser_iter();
		const InfAclAccount* account =
			inf_browser_get_acl_local_account(browser);
		const InfAclAccountId acc_id =
			(account != NULL) ? account->id : 0;
		InfAclMask msk;
		inf_acl_mask_set1(&msk, INF_ACL_CAN_JOIN_USER);
		if(!inf_browser_check_acl(browser, iter, acc_id, &msk, NULL))
		{
			GError* error = NULL;
			g_set_error(
				&error, inf_request_error_quark(),
				INF_REQUEST_ERROR_NOT_AUTHORIZED,
				"%s", inf_request_strerror(
					INF_REQUEST_ERROR_NOT_AUTHORIZED));
			user_join_complete(NULL, error);
			g_error_free(error);
			return;
		}
	}

	// We are allowed, so attempt to join the user now.
	std::vector<GParameter> params =
		m_param_provider->get_user_join_parameters();
	std::vector<GParameter>::iterator name_index =
		find_name_param(params);
	const gchar* name = g_value_get_string(&name_index->value);

	if(m_retry_index > 1)
	{
		gchar* new_name = g_strdup_printf(
			"%s %u", name, m_retry_index);
		g_value_take_string(&name_index->value, new_name);
	}

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
	}
}

void Gobby::UserJoin::user_join_complete(InfUser* user, const GError* error)
{
	g_assert(m_request == NULL);
	g_assert(m_user == NULL && m_error == NULL);

	m_user = user;
	if(error) m_error = g_error_copy(error);

	m_signal_finished.emit(m_user, error);
}
