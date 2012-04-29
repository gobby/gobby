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

#include "commands/user-join-commands.hpp"
#include "util/i18n.hpp"

#include <glibmm/main.h>

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

	void retr_local_user_func(InfUser* user, gpointer user_data)
	{
		(*static_cast<InfUser**>(user_data)) = user;
	}
}

class Gobby::UserJoinCommands::UserJoinInfo
{
public:
	UserJoinInfo(UserJoinCommands& commands,
	             InfcSessionProxy* proxy,
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

	static void on_user_join_failed_static(InfcUserRequest* request,
	                                       const GError* error,
	                                       gpointer user_data)
	{
		static_cast<UserJoinInfo*>(user_data)->
			on_user_join_failed(error);
	}

	static void on_user_join_finished_static(InfcUserRequest* request,
	                                         InfUser* user,
	                                         gpointer user_data)
	{
		static_cast<UserJoinInfo*>(user_data)->
			on_user_join_finished(user);
	}

	void on_synchronization_complete();
	void on_user_join_failed(const GError* error);
	void on_user_join_finished(InfUser* user);

	void attempt_user_join();
	void user_join_complete(InfUser* user);
	void finish();

	void add_text_user_properties(std::vector<GParameter>& params,
	                              TextSessionView& view);

	UserJoinCommands& m_commands;

	InfcSessionProxy* m_proxy;
	Folder& m_folder;
	SessionView& m_view;

	InfcUserRequest* m_request;

	gulong m_synchronization_complete_handler;
	gulong m_user_join_failed_handler;
	gulong m_user_join_finished_handler;

	guint m_retry_index;
};

Gobby::UserJoinCommands::UserJoinInfo::UserJoinInfo(UserJoinCommands& cmds,
                                                    InfcSessionProxy* proxy,
                                                    Folder& folder,
                                                    SessionView& view):
	m_commands(cmds), m_proxy(proxy), m_folder(folder), m_view(view),
	m_request(NULL), m_synchronization_complete_handler(0),
	m_user_join_failed_handler(0), m_user_join_finished_handler(0),
	m_retry_index(1)
{
	g_object_ref(m_proxy);

	InfSession* session = infc_session_proxy_get_session(proxy);

	if(inf_session_get_status(session) == INF_SESSION_SYNCHRONIZING)
	{
		// If not yet synchronization wait for synchronization until
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
}

Gobby::UserJoinCommands::UserJoinInfo::~UserJoinInfo()
{
	if(m_synchronization_complete_handler)
	{
		InfSession* session = infc_session_proxy_get_session(m_proxy);
		g_signal_handler_disconnect(
			session, m_synchronization_complete_handler);
	}

	if(m_request)
	{
		g_signal_handler_disconnect(m_request,
		                            m_user_join_failed_handler);
		g_signal_handler_disconnect(m_request,
		                            m_user_join_finished_handler);

		g_object_unref(m_request);
	}

	g_object_unref(m_proxy);
}

void Gobby::UserJoinCommands::UserJoinInfo::on_synchronization_complete()
{
	// Disconnect signal handler, so that we don't get notified when
	// syncing this document in running state to another location
	// or server.
	InfSession* session = infc_session_proxy_get_session(m_proxy);
	g_signal_handler_disconnect(session,
	                            m_synchronization_complete_handler);
	m_synchronization_complete_handler = 0;

	// Attempt user join after synchronization
	attempt_user_join();
}

void Gobby::UserJoinCommands::UserJoinInfo::
	on_user_join_failed(const GError* error)
{
	if(error->domain == inf_user_error_quark() &&
	   error->code == INF_USER_ERROR_NAME_IN_USE)
	{
		// If name is in use retry with alternative user name
		++m_retry_index;

		attempt_user_join();
	}
	else
	{
		set_error_text(m_view, error->message);
		finish();
	}
}

void Gobby::UserJoinCommands::UserJoinInfo::
	on_user_join_finished(InfUser* user)
{
	user_join_complete(user);
}

void Gobby::UserJoinCommands::UserJoinInfo::attempt_user_join()
{
	const Preferences& preferences = m_commands.m_preferences;

	// Check if there is already a local user, for example for a
	// synced-in document.
	InfSession* session = infc_session_proxy_get_session(m_proxy);
	InfUserTable* user_table = inf_session_get_user_table(session);
	InfUser* user = NULL;
	inf_user_table_foreach_local_user(user_table,
	                                  retr_local_user_func, &user);

	if(user != NULL)
	{
		user_join_complete(user);
	}
	else
	{
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
		m_request = infc_session_proxy_join_user(
			m_proxy, &params[0], params.size(), &error);

		for(unsigned int i = 0; i < params.size(); ++i)
			g_value_unset(&params[i].value);

		if(m_request == NULL)
		{
			set_error_text(m_view, error->message);
			g_error_free(error);
		}
		else
		{
			g_object_ref(m_request);

			m_view.set_info(
				_("User Join in progress..."), false);

			m_user_join_failed_handler = g_signal_connect(
				m_request, "failed",
				G_CALLBACK(on_user_join_failed_static), this);
			m_user_join_finished_handler = g_signal_connect(
				m_request, "finished",
				G_CALLBACK(on_user_join_finished_static),
				this);
		}
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

Gobby::UserJoinCommands::
	UserJoinCommands(SubscriptionCommands& subscription_commands,
	                 const Preferences& preferences):
	m_preferences(preferences)
{
	subscription_commands.signal_subscribe_session().connect(
		sigc::mem_fun(
			*this, &UserJoinCommands::on_subscribe_session));
	subscription_commands.signal_unsubscribe_session().connect(
		sigc::mem_fun(
			*this, &UserJoinCommands::on_unsubscribe_session));
}

Gobby::UserJoinCommands::~UserJoinCommands()
{
	for(UserJoinMap::iterator iter = m_user_join_map.begin();
	    iter != m_user_join_map.end(); ++iter)
	{
		delete iter->second;
	}
}

void Gobby::UserJoinCommands::on_subscribe_session(InfcSessionProxy* proxy,
                                                   Folder& folder,
                                                   SessionView& view)
{
	g_assert(m_user_join_map.find(proxy) == m_user_join_map.end());
	m_user_join_map[proxy] = new UserJoinInfo(*this, proxy, folder, view);
}

void Gobby::UserJoinCommands::on_unsubscribe_session(InfcSessionProxy* proxy,
                                                     Folder& folder,
                                                     SessionView& view)
{
	UserJoinMap::iterator iter = m_user_join_map.find(proxy);

	// If the user join was successful the session is no longer in
	// our map, so don't assert here.
	if(iter != m_user_join_map.end())
	{
		delete iter->second;
		m_user_join_map.erase(iter);
	}
}
