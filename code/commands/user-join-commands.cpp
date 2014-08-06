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

	class ParameterProvider: public Gobby::UserJoin::ParameterProvider
	{
	public:
		ParameterProvider(Gobby::SessionView& view,
		                  Gobby::Folder& folder,
		                  const Gobby::Preferences& preferences):
			m_view(view), m_folder(folder),
			m_preferences(preferences)
		{
		}
	
		virtual std::vector<GParameter> get_user_join_parameters();
	protected:
		void add_text_parameters(std::vector<GParameter>& parameters,
		                         Gobby::TextSessionView& view);

		Gobby::SessionView& m_view;
		Gobby::Folder& m_folder;
		const Gobby::Preferences& m_preferences;
	};

	std::vector<GParameter> ParameterProvider::get_user_join_parameters()
	{
		// Otherwise join a new user.
		std::vector<GParameter> params;
		const GParameter name_param = { "name", { 0 } };
		params.push_back(name_param);
		const GParameter status_param = { "status", { 0 } };
		params.push_back(status_param);

		g_value_init(&params[0].value, G_TYPE_STRING);
		g_value_init(&params[1].value, INF_TYPE_USER_STATUS);

		const Glib::ustring& pref_name = m_preferences.user.name;
		g_value_set_string(&params[0].value, pref_name.c_str());

		if(m_folder.get_current_document() == &m_view)
			g_value_set_enum(&params[1].value, INF_USER_ACTIVE);
		else
			g_value_set_enum(&params[1].value, INF_USER_INACTIVE);

		Gobby::TextSessionView* text_view =
			dynamic_cast<Gobby::TextSessionView*>(&m_view);
		if(text_view) add_text_parameters(params, *text_view);

		return params;
	}

	void ParameterProvider::add_text_parameters(
		std::vector<GParameter>& params,
		Gobby::TextSessionView& view)
	{
		InfTextSession* session = view.get_session();

		GParameter hue_param = { "hue", { 0 } };
		g_value_init(&hue_param.value, G_TYPE_DOUBLE);
		g_value_set_double(&hue_param.value, m_preferences.user.hue);
		params.push_back(hue_param);

		GParameter vector_param = { "vector", { 0 } };
		g_value_init(&vector_param.value,
		             INF_ADOPTED_TYPE_STATE_VECTOR);

		g_value_take_boxed(&vector_param.value,
			inf_adopted_state_vector_copy(
				inf_adopted_algorithm_get_current(
					inf_adopted_session_get_algorithm(
						INF_ADOPTED_SESSION(
							session)))));
		params.push_back(vector_param);

		GParameter caret_param = { "caret-position", { 0 } };
		g_value_init(&caret_param.value, G_TYPE_UINT);

		GtkTextBuffer* buffer =
			GTK_TEXT_BUFFER(view.get_text_buffer());
		GtkTextMark* mark = gtk_text_buffer_get_insert(buffer);
		GtkTextIter caret_iter;

		gtk_text_buffer_get_iter_at_mark(buffer, &caret_iter, mark);
		g_value_set_uint(&caret_param.value,
		                 gtk_text_iter_get_offset(&caret_iter));
		params.push_back(caret_param);
	}
}

class Gobby::UserJoinCommands::UserJoinInfo
{
public:
	UserJoinInfo(UserJoinCommands& commands,
	             std::auto_ptr<UserJoin> userjoin,
	             Folder& folder,
	             SessionView& view);

private:
	void on_user_join_finished(InfUser* user, const GError* error);

	UserJoinCommands& m_commands;
	std::auto_ptr<UserJoin> m_userjoin;
	Folder& m_folder;
	SessionView& m_view;
};

Gobby::UserJoinCommands::UserJoinInfo::UserJoinInfo(UserJoinCommands& cmds,
                                                    std::auto_ptr<UserJoin> j,
                                                    Folder& folder,
                                                    SessionView& view):
	m_commands(cmds), m_userjoin(j), m_folder(folder), m_view(view)
{
	// Only if userjoin is still running:
	g_assert(m_userjoin->get_user() == NULL);
	g_assert(m_userjoin->get_error() == NULL);

	// Wait for userjoin to finish
	m_userjoin->signal_finished().connect(
		sigc::mem_fun(
			*this, &UserJoinInfo::on_user_join_finished));
}

void Gobby::UserJoinCommands::UserJoinInfo::
	on_user_join_finished(InfUser* user, const GError* error)
{
	m_commands.on_user_join_finished(
		m_userjoin->get_proxy(), m_folder, m_view, user, error);
	// Note that the above call deletes this object!
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

	std::auto_ptr<UserJoin::ParameterProvider> provider(
		new ParameterProvider(view, folder, m_preferences));
	std::auto_ptr<UserJoin> userjoin(
		new UserJoin(browser, iter, proxy, provider));
	m_user_join_map[proxy] =
		new UserJoinInfo(*this, userjoin, folder, view);
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

void Gobby::UserJoinCommands::on_user_join_finished(InfSessionProxy* proxy,
                                                    Folder& folder,
                                                    SessionView& view,
                                                    InfUser* user,
                                                    const GError* error)
{
	g_assert(user != NULL || error != NULL);

	// Remove userjoin object. It might not exist if
	// on_document_added() was passed a user immediately.
	UserJoinMap::iterator user_iter = m_user_join_map.find(proxy);
	if(user_iter != m_user_join_map.end())
	{
		delete user_iter->second;
		m_user_join_map.erase(user_iter);
	}

	if(error == NULL)
	{
		// TODO: Notify the user about alternative user name if s/he uses any
		view.unset_info();

		// TODO: set_active_user should maybe go to SessionView base:
		TextSessionView* text_view =
			dynamic_cast<TextSessionView*>(&view);
		if(text_view)
			text_view->set_active_user(INF_TEXT_USER(user));
		ChatSessionView* chat_view =
			dynamic_cast<ChatSessionView*>(&view);
		if(chat_view)
			chat_view->set_active_user(user);
	}
	else if(error->domain == inf_request_error_quark() &&
	        error->code == INF_REQUEST_ERROR_NOT_AUTHORIZED)
	{
		set_permission_denied_text(view);
	}
	else
	{
		set_error_text(view, error->message);
	}
}
