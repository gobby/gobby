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

#include "commands/subscription-commands.hpp"
#include "util/i18n.hpp"

class Gobby::SubscriptionCommands::SessionInfo
{
public:
	SessionInfo(SubscriptionCommands& commands, const Folder& folder,
	            InfSession* session):
		m_folder(folder), m_session(session)
	{
		g_object_ref(session);

		m_notify_subscription_group_handler = g_signal_connect(
			G_OBJECT(session),
			"notify::subscription-group",
			G_CALLBACK(on_notify_subscription_group_static),
			&commands);
	}

	~SessionInfo()
	{
		g_signal_handler_disconnect(
			G_OBJECT(m_session),
			m_notify_subscription_group_handler);

		g_object_unref(m_session);
	}

	const Folder& get_folder() { return m_folder; }
	InfSession* get_session() { return m_session; }

private:
	static void on_notify_subscription_group_static(InfSession* session,
	                                                GParamSpec* pspec,
	                                                gpointer user_data)
	{
		static_cast<SubscriptionCommands*>(user_data)->
			on_notify_subscription_group(session);
	}

	const Folder& m_folder;
	InfSession* m_session;

	gulong m_notify_subscription_group_handler;
};

Gobby::SubscriptionCommands::SubscriptionCommands(const Folder& text_folder,
                                                  const Folder& chat_folder):
	m_text_folder(text_folder), m_chat_folder(chat_folder)
{
	m_text_folder.signal_document_added().connect(
		sigc::mem_fun(
			*this,
			&SubscriptionCommands::on_text_document_added));
	m_chat_folder.signal_document_added().connect(
		sigc::mem_fun(
			*this,
			&SubscriptionCommands::on_chat_document_added));

	m_text_folder.signal_document_removed().connect(
		sigc::mem_fun(
			*this, &SubscriptionCommands::on_document_removed));
	m_chat_folder.signal_document_removed().connect(
		sigc::mem_fun(
			*this, &SubscriptionCommands::on_document_removed));
}

Gobby::SubscriptionCommands::~SubscriptionCommands()
{
	for(SessionMap::iterator iter = m_session_map.begin();
	    iter != m_session_map.end(); ++iter)
	{
		delete iter->second;
	}
}

void Gobby::SubscriptionCommands::on_text_document_added(SessionView& view)
{
	InfSession* session = view.get_session();
	g_assert(m_session_map.find(session) == m_session_map.end());

	m_session_map[session] =
		new SessionInfo(*this, m_text_folder, session);
}

void Gobby::SubscriptionCommands::on_chat_document_added(SessionView& view)
{
	InfSession* session = view.get_session();
	g_assert(m_session_map.find(session) == m_session_map.end());

	m_session_map[session] =
		new SessionInfo(*this, m_chat_folder, session);
}

void Gobby::SubscriptionCommands::on_document_removed(SessionView& view)
{
	InfSession* session = view.get_session();
	SessionMap::iterator iter = m_session_map.find(session);
	g_assert(iter != m_session_map.end());

	delete iter->second;
	m_session_map.erase(iter);
}

void Gobby::SubscriptionCommands::
	on_notify_subscription_group(InfSession* session)
{
	SessionMap::iterator iter = m_session_map.find(session);
	g_assert(iter != m_session_map.end());

	if(inf_session_get_subscription_group(session) == NULL)
	{
		const Folder& folder = iter->second->get_folder();
		SessionView* view = folder.lookup_document(session);
		g_assert(view != NULL);

		TextSessionView* text_view =
			dynamic_cast<TextSessionView*>(view);
		ChatSessionView* chat_view =
			dynamic_cast<ChatSessionView*>(view);

		if(text_view)
		{
			/* If the session is in SYNCHRONIZING state then the
			 * session is closed due to a synchronization error.
			 * In that case synchronization-commands.cpp will set
			 * a more meaningful error message. */
			if(inf_session_get_status(session) ==
			   INF_SESSION_RUNNING)
			{
				view->set_info(_(
					"The connection to the publisher of "
					"this document has been lost. "
					"Further changes to the document "
					"could not be synchronized to others "
					"anymore, therefore the document "
					"cannot be edited anymore.\n\n"
					"Please note also that it is "
					"possible that not all of your "
					"latest changes have reached the "
					"publisher before the connection was "
					"lost."), true);
			}

			text_view->set_active_user(NULL);
		}
		else if(chat_view)
		{
			chat_view->set_active_user(NULL);
		}
	}
}
