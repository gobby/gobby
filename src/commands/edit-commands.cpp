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
                                  Folder& folder, StatusBar& status_bar,
                                  Preferences& preferences):
	m_parent(parent), m_header(header), m_folder(folder),
	m_preferences(preferences), m_status_bar(status_bar),
	m_current_document(NULL)
{
	m_header.action_edit_undo->signal_activate().connect(
		sigc::mem_fun(*this, &EditCommands::on_undo));
	m_header.action_edit_redo->signal_activate().connect(
		sigc::mem_fun(*this, &EditCommands::on_redo));
	m_header.action_edit_cut->signal_activate().connect(
		sigc::mem_fun(*this, &EditCommands::on_cut));
	m_header.action_edit_copy->signal_activate().connect(
		sigc::mem_fun(*this, &EditCommands::on_copy));
	m_header.action_edit_paste->signal_activate().connect(
		sigc::mem_fun(*this, &EditCommands::on_paste));
	m_header.action_edit_find->signal_activate().connect(
		sigc::mem_fun(*this, &EditCommands::on_find));
	m_header.action_edit_find_next->signal_activate().connect(
		sigc::mem_fun(*this, &EditCommands::on_find_next));
	m_header.action_edit_find_prev->signal_activate().connect(
		sigc::mem_fun(*this, &EditCommands::on_find_prev));
	m_header.action_edit_find_replace->signal_activate().connect(
		sigc::mem_fun(*this, &EditCommands::on_find_replace));
	m_header.action_edit_goto_line->signal_activate().connect(
		sigc::mem_fun(*this, &EditCommands::on_goto_line));
	m_header.action_edit_preferences->signal_activate().connect(
		sigc::mem_fun(*this, &EditCommands::on_preferences));
	m_folder.signal_document_removed().connect(
		sigc::mem_fun(*this, &EditCommands::on_document_removed));
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

void Gobby::EditCommands::on_document_removed(DocWindow& document)
{
	if(&document == m_current_document)
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
		GtkTextBuffer* buffer = GTK_TEXT_BUFFER(
			m_current_document->get_text_buffer());

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

		g_signal_handler_disconnect(G_OBJECT(buffer),
		                            m_mark_set_handler);
		g_signal_handler_disconnect(G_OBJECT(buffer),
		                            m_changed_handler);

		m_active_user_changed_connection.disconnect();
	}

	m_current_document = document;

	if(document != NULL)
	{
		InfTextSession* session = document->get_session();
		InfTextUser* active_user = document->get_active_user();
		GtkTextBuffer* buffer =
			GTK_TEXT_BUFFER(document->get_text_buffer());

		m_active_user_changed_connection =
			document->signal_active_user_changed().connect(
				sigc::mem_fun(
					*this,
					&EditCommands::
						on_active_user_changed));

		m_mark_set_handler = g_signal_connect_after(
			G_OBJECT(buffer), "mark-set",
			G_CALLBACK(&on_mark_set_static), this);
		// The selection might change without mark-set being emitted
		// when the document changes, for example when all
		// currently selected text is deleted.
		m_changed_handler = g_signal_connect_after(
			G_OBJECT(buffer), "changed",
			G_CALLBACK(&on_changed_static), this);

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
		// Set initial sensitivity for cut/copy/paste:
		on_mark_set();

		// Set initial sensitivity for find/replace/goto:
		m_header.action_edit_find->set_sensitive(true);

		if(m_find_dialog.get())
		{
			on_find_text_changed();
		}
		else
		{
			m_header.action_edit_find_next->set_sensitive(false);
			m_header.action_edit_find_prev->set_sensitive(false);
		}

		m_header.action_edit_find_replace->set_sensitive(true);
		m_header.action_edit_goto_line->set_sensitive(true);
	}
	else
	{
		m_header.action_edit_undo->set_sensitive(false);
		m_header.action_edit_redo->set_sensitive(false);
		m_header.action_edit_cut->set_sensitive(false);
		m_header.action_edit_copy->set_sensitive(false);
		m_header.action_edit_paste->set_sensitive(false);
		m_header.action_edit_find->set_sensitive(false);
		m_header.action_edit_find_next->set_sensitive(false);
		m_header.action_edit_find_prev->set_sensitive(false);
		m_header.action_edit_find_replace->set_sensitive(false);
		m_header.action_edit_goto_line->set_sensitive(false);
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

	if(active_user != NULL)
	{
		InfTextSession* session = m_current_document->get_session();
		InfAdoptedAlgorithm* algorithm =
			inf_adopted_session_get_algorithm(
				INF_ADOPTED_SESSION(session));
		GtkTextBuffer* buffer = GTK_TEXT_BUFFER(
			m_current_document->get_text_buffer());

		m_header.action_edit_undo->set_sensitive(
			inf_adopted_algorithm_can_undo(
				algorithm, INF_ADOPTED_USER(active_user)));
		m_header.action_edit_redo->set_sensitive(
			inf_adopted_algorithm_can_redo(
				algorithm, INF_ADOPTED_USER(active_user)));

		m_header.action_edit_cut->set_sensitive(
			gtk_text_buffer_get_has_selection(buffer));
		m_header.action_edit_paste->set_sensitive(true);
	}
	else
	{
		m_header.action_edit_undo->set_sensitive(false);
		m_header.action_edit_redo->set_sensitive(false);
		m_header.action_edit_cut->set_sensitive(false);
		m_header.action_edit_paste->set_sensitive(false);
	}
}

void Gobby::EditCommands::on_mark_set()
{
	g_assert(m_current_document != NULL);
	GtkTextBuffer* buffer =
		GTK_TEXT_BUFFER(m_current_document->get_text_buffer());

	m_header.action_edit_copy->set_sensitive(
		gtk_text_buffer_get_has_selection(buffer));

	if(m_current_document->get_active_user() != NULL)
	{
		m_header.action_edit_cut->set_sensitive(
			gtk_text_buffer_get_has_selection(buffer));
	}
}

void Gobby::EditCommands::on_changed()
{
	on_mark_set();
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

void Gobby::EditCommands::on_find_text_changed()
{
	m_header.action_edit_find_next->set_sensitive(
		!m_find_dialog->get_find_text().empty());
	m_header.action_edit_find_prev->set_sensitive(
		!m_find_dialog->get_find_text().empty());
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

void Gobby::EditCommands::on_cut()
{
	g_assert(m_current_document != NULL);
	g_assert(m_current_document->get_active_user() != NULL);

	gtk_text_buffer_cut_clipboard(
		GTK_TEXT_BUFFER(m_current_document->get_text_buffer()),
		gtk_clipboard_get(GDK_SELECTION_CLIPBOARD),
		TRUE);
}

void Gobby::EditCommands::on_copy()
{
	g_assert(m_current_document != NULL);

	gtk_text_buffer_copy_clipboard(
		GTK_TEXT_BUFFER(m_current_document->get_text_buffer()),
		gtk_clipboard_get(GDK_SELECTION_CLIPBOARD));
}

void Gobby::EditCommands::on_paste()
{
	g_assert(m_current_document != NULL);
	g_assert(m_current_document->get_active_user() != NULL);

	gtk_text_buffer_paste_clipboard(
		GTK_TEXT_BUFFER(m_current_document->get_text_buffer()),
		gtk_clipboard_get(GDK_SELECTION_CLIPBOARD),
		NULL, TRUE);
}

void Gobby::EditCommands::on_find()
{
	ensure_find_dialog();
	m_find_dialog->set_search_only(true);
	m_find_dialog->present();
}

void Gobby::EditCommands::on_find_next()
{
	g_assert(m_find_dialog.get() != NULL);
	m_find_dialog->find_next();
}

void Gobby::EditCommands::on_find_prev()
{
	g_assert(m_find_dialog.get() != NULL);
	m_find_dialog->find_previous();
}

void Gobby::EditCommands::on_find_replace()
{
	ensure_find_dialog();
	m_find_dialog->set_search_only(false);
	m_find_dialog->present();
}

void Gobby::EditCommands::on_goto_line()
{
	if(!m_goto_dialog.get())
		m_goto_dialog.reset(new GotoDialog(m_parent, m_folder));

	m_goto_dialog->present();
}

void Gobby::EditCommands::on_preferences()
{
	if(!m_preferences_dialog.get())
	{
		m_preferences_dialog.reset(
			new PreferencesDialog(m_parent, m_preferences));
	}

	m_preferences_dialog->present();
}

void Gobby::EditCommands::ensure_find_dialog()
{
	if(!m_find_dialog.get())
	{
		m_find_dialog.reset(new FindDialog(m_parent, m_folder,
		                                   m_status_bar));
		m_find_dialog->signal_find_text_changed().connect(
			sigc::mem_fun(
				*this, &EditCommands::on_find_text_changed));
	}
}
