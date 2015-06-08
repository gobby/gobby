/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2015 Armin Burgmeier <armin@arbur.net>
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

#include "commands/synchronization-commands.hpp"

#include "util/i18n.hpp"

namespace
{
	inline const gchar* _(const gchar* msgid) { return Gobby::_(msgid); }

	void set_progress_text(Gobby::SessionView& view,
	                       gdouble percentage)
	{
		view.set_info(
			Glib::ustring::compose(
				_("Synchronization in progress... %1%%"),
				static_cast<unsigned int>(percentage * 100)),
			false);
	}

	void set_error_text(Gobby::SessionView& view,
			    const Glib::ustring& initial_text)
	{
		Glib::ustring type_text;

		// Document cannot be used if an error happened
		// during synchronization.
		type_text = _("This document cannot be used.");

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
				_("Synchronization failed: %1"),
				initial_text) +
			"\n\n" + type_text + "\n\n" + info_text, true);
	}
}

class Gobby::SynchronizationCommands::SyncInfo
{
public:
	SyncInfo(SynchronizationCommands& sync_commands,
	         SessionView& view);
	~SyncInfo();

	SessionView& get_session_view() { return m_view; }

private:
	static void
	on_synchronization_failed_static(InfSession* session,
	                                 InfXmlConnection* conn,
	                                 const GError* error,
	                                 gpointer user_data)
	{
		static_cast<SynchronizationCommands*>(user_data)->
			on_synchronization_failed(session, conn, error);
	}

	static void
	on_synchronization_complete_static(InfSession* session,
	                                   InfXmlConnection* conn,
	                                   gpointer user_data)
	{
		static_cast<SynchronizationCommands*>(user_data)->
			on_synchronization_complete(session, conn);
	}

	static void
	on_synchronization_progress_static(InfSession* session,
	                                   InfXmlConnection* conn,
	                                   gdouble percentage,
	                                   gpointer user_data)
	{
		static_cast<SyncInfo*>(user_data)->
			on_synchronization_progress(conn, percentage);
	}

	void on_synchronization_progress(InfXmlConnection* conn,
	                                 gdouble percentage);

	SessionView& m_view;

	gulong m_synchronization_complete_handler;
	gulong m_synchronization_failed_handler;
	gulong m_synchronization_progress_handler;
};

Gobby::SynchronizationCommands::SyncInfo::
	SyncInfo(SynchronizationCommands& commands, SessionView& view):
	m_view(view)
{
	InfSession* session = m_view.get_session();

	m_synchronization_complete_handler = g_signal_connect(
		G_OBJECT(session), "synchronization-complete",
		G_CALLBACK(on_synchronization_complete_static), &commands);
	m_synchronization_failed_handler = g_signal_connect(
		G_OBJECT(session), "synchronization-failed",
		G_CALLBACK(on_synchronization_failed_static), &commands);
	m_synchronization_progress_handler = g_signal_connect(
		G_OBJECT(session), "synchronization-progress",
		G_CALLBACK(on_synchronization_progress_static), this);
}

Gobby::SynchronizationCommands::SyncInfo::~SyncInfo()
{
	InfSession* session = m_view.get_session();

	g_signal_handler_disconnect(G_OBJECT(session),
	                            m_synchronization_complete_handler);
	g_signal_handler_disconnect(G_OBJECT(session),
	                            m_synchronization_failed_handler);
	g_signal_handler_disconnect(G_OBJECT(session),
	                            m_synchronization_progress_handler);
}

void Gobby::SynchronizationCommands::SyncInfo::
	on_synchronization_progress(InfXmlConnection* conn,
	                            gdouble percentage)
{
	set_progress_text(m_view, percentage);
}

Gobby::SynchronizationCommands::
	SynchronizationCommands(const Folder& text_folder,
	                        const Folder& chat_folder)
{
	text_folder.signal_document_added().connect(
		sigc::mem_fun(
			*this,
			&SynchronizationCommands::on_document_added));
	chat_folder.signal_document_added().connect(
		sigc::mem_fun(
			*this,
			&SynchronizationCommands::on_document_added));

	text_folder.signal_document_removed().connect(
		sigc::mem_fun(
			*this,
			&SynchronizationCommands::on_document_removed));

	text_folder.signal_document_removed().connect(
		sigc::mem_fun(
			*this,
			&SynchronizationCommands::on_document_removed));
}

Gobby::SynchronizationCommands::~SynchronizationCommands()
{
	for(SyncMap::iterator iter = m_sync_map.begin();
	    iter != m_sync_map.end(); ++iter)
	{
		delete iter->second;
	}
}

void
Gobby::SynchronizationCommands::on_document_added(SessionView& view)
{
	InfSession* session = view.get_session();
	if(inf_session_get_status(session) == INF_SESSION_SYNCHRONIZING)
	{
		InfXmlConnection* connection;
		g_object_get(G_OBJECT(session),
		             "sync-connection", &connection, NULL);
		gdouble percentage = inf_session_get_synchronization_progress(
			session, connection);
		g_object_unref(connection);

		g_assert(m_sync_map.find(session) == m_sync_map.end());
		m_sync_map[session] = new SyncInfo(*this, view);
		set_progress_text(view, percentage);
	}
}

void
Gobby::SynchronizationCommands::on_document_removed(SessionView& view)
{
	InfSession* session = view.get_session();
	SyncMap::iterator iter = m_sync_map.find(session);
	if(iter != m_sync_map.end())
	{
		delete iter->second;
		m_sync_map.erase(iter);
	}
}

void
Gobby::SynchronizationCommands::
	on_synchronization_failed(InfSession* session,
                                  InfXmlConnection* c,
                                  const GError* error)
{
	SyncMap::iterator iter = m_sync_map.find(session);
	g_assert(iter != m_sync_map.end());

	set_error_text(iter->second->get_session_view(), error->message);

	// The document will be of no use anyway, so consider it as not
	// being modified.
	InfBuffer* buffer = inf_session_get_buffer(session);
	inf_buffer_set_modified(buffer, FALSE);

	delete iter->second;
	m_sync_map.erase(iter);
}

void
Gobby::SynchronizationCommands::
	on_synchronization_complete(InfSession* session,
                                    InfXmlConnection* c)
{
	SyncMap::iterator iter = m_sync_map.find(session);
	g_assert(iter != m_sync_map.end());

	// TODO: Actually we should always set the modified flag, except the
	// document is either empty, or known in the document info storage
	// and the version on disk is the same as the one we got
	// synchronized. We could store a hash and modification time in the
	// documentinfo storage for this.
	InfBuffer* buffer = inf_session_get_buffer(session);
	inf_buffer_set_modified(buffer, FALSE);

	delete iter->second;
	m_sync_map.erase(iter);
}
