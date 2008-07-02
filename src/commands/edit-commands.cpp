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

#include "commands/edit-commands.hpp"
#include "util/i18n.hpp"

namespace
{

} // anonymous namespace

Gobby::EditCommands::EditCommands(Gtk::Window& parent, Header& header,
                                  Folder& folder, Preferences& preferences):
	m_parent(parent), m_header(header), m_folder(folder),
	m_preferences(preferences), m_current_document(NULL)
{
	m_header.action_edit_undo->signal_activate().connect(
		sigc::mem_fun(*this, &EditCommands::on_undo));
	m_header.action_edit_redo->signal_activate().connect(
		sigc::mem_fun(*this, &EditCommands::on_redo));
	m_folder.signal_document_changed().connect(
		sigc::mem_fun(*this, &EditCommands::on_document_changed));

	// Setup initial sensitivity:
	on_document_changed(m_folder.get_current_document());
}

Gobby::EditCommands::~EditCommands()
{
	// Disconnect handlers from current document:
	on_document_changed(NULL);
}

void Gobby::EditCommands::on_document_changed(DocWindow* document)
{
	if(m_current_document != NULL)
	{
		InfTextSession* session = m_current_document->get_session();
		InfAdoptedAlgorithm* algorithm =
			inf_adopted_session_get_algorithm(
				INF_ADOPTED_SESSION(session));

		if(m_synchronization_complete_handler != 0)
		{
			g_signal_handler_disconnect(
				G_OBJECT(session),
				m_synchronization_complete_handler);
		}
		else
		{
			g_signal_handler_disconnect(
				G_OBJECT(algorithm),
				m_can_undo_changed_handler);
			g_signal_handler_disconnect(
				G_OBJECT(algorithm),
				m_can_redo_changed_handler);
		}

		m_active_user_changed_connection.disconnect();
	}

	m_current_document = document;

	if(document != NULL)
	{
		InfTextSession* session = document->get_session();
		InfTextUser* active_user = document->get_active_user();

		m_active_user_changed_connection =
			document->signal_active_user_changed().connect(
				sigc::mem_fun(
					*this,
					&EditCommands::
						on_active_user_changed));

		if(inf_session_get_status(INF_SESSION(session)) ==
		   INF_SESSION_RUNNING)
		{
			// This connects to can-undo-changed and
			// can-redo-changed of the algorithm. Set
			// m_synchronization_complete_handler to zero so that
			// the function does not try to disconnect from it.
			m_synchronization_complete_handler = 0;
			on_sync_complete();
		}
		else
		{
			// The InfAdoptedSession is created after
			// synchronization, so we wait until that finished.
			m_synchronization_complete_handler =
				g_signal_connect_after(
					G_OBJECT(session),
					"synchronization_complete",
					G_CALLBACK(&on_sync_complete_static),
					this);

			m_can_undo_changed_handler = 0;
			m_can_redo_changed_handler = 0;
		}

		// Set initial sensitivity for active user:
		on_active_user_changed(active_user);
	}
	else
	{
		m_header.action_edit_undo->set_sensitive(false);
		m_header.action_edit_redo->set_sensitive(false);
	}
}

void Gobby::EditCommands::on_sync_complete()
{
	g_assert(m_current_document != NULL);
	InfTextSession* session = m_current_document->get_session();

	InfAdoptedAlgorithm* algorithm = inf_adopted_session_get_algorithm(
		INF_ADOPTED_SESSION(session));

	m_can_undo_changed_handler = g_signal_connect(
		G_OBJECT(algorithm), "can-undo-changed",
		G_CALLBACK(&on_can_undo_changed_static), this);

	m_can_redo_changed_handler = g_signal_connect(
		G_OBJECT(algorithm), "can-redo-changed",
		G_CALLBACK(&on_can_redo_changed_static), this);

	if(m_synchronization_complete_handler != 0)
	{
		g_signal_handler_disconnect(
			G_OBJECT(session),
			m_synchronization_complete_handler);
		m_synchronization_complete_handler = 0;
	}
}

void Gobby::EditCommands::on_active_user_changed(InfTextUser* active_user)
{
	g_assert(m_current_document != NULL);
	InfTextSession* session = m_current_document->get_session();
	InfAdoptedAlgorithm* algorithm = inf_adopted_session_get_algorithm(
		INF_ADOPTED_SESSION(session));

	if(active_user != NULL)
	{
		m_header.action_edit_undo->set_sensitive(
			inf_adopted_algorithm_can_undo(
				algorithm, INF_ADOPTED_USER(active_user)));
		m_header.action_edit_redo->set_sensitive(
			inf_adopted_algorithm_can_redo(
				algorithm, INF_ADOPTED_USER(active_user)));
	}
	else
	{
		m_header.action_edit_undo->set_sensitive(false);
		m_header.action_edit_redo->set_sensitive(false);
	}
}

void Gobby::EditCommands::on_can_undo_changed(InfAdoptedUser* user,
                                              bool can_undo)
{
	g_assert(m_current_document != NULL);
	if(INF_ADOPTED_USER(m_current_document->get_active_user()) == user)
		m_header.action_edit_undo->set_sensitive(can_undo);
}

void Gobby::EditCommands::on_can_redo_changed(InfAdoptedUser* user,
                                              bool can_redo)
{
	g_assert(m_current_document != NULL);
	if(INF_ADOPTED_USER(m_current_document->get_active_user()) == user)
		m_header.action_edit_redo->set_sensitive(can_redo);
}

void Gobby::EditCommands::on_undo()
{
	g_assert(m_current_document != NULL);

	inf_adopted_session_undo(
		INF_ADOPTED_SESSION(m_current_document->get_session()),
		INF_ADOPTED_USER(m_current_document->get_active_user())
	);
}

void Gobby::EditCommands::on_redo()
{
	g_assert(m_current_document != NULL);

	inf_adopted_session_redo(
		INF_ADOPTED_SESSION(m_current_document->get_session()),
		INF_ADOPTED_USER(m_current_document->get_active_user())
	);
}
