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

#include "commands/edit-commands.hpp"
#include "util/i18n.hpp"

namespace
{

} // anonymous namespace

Gobby::EditCommands::EditCommands(Gtk::Window& parent,
                                  WindowActions& actions,
                                  const Folder& folder,
                                  StatusBar& status_bar):
	m_parent(parent), m_actions(actions), m_folder(folder),
	m_status_bar(status_bar), m_current_view(NULL)
{
	actions.undo->signal_activate().connect(sigc::hide(
		sigc::mem_fun(*this, &EditCommands::on_undo)));
	actions.redo->signal_activate().connect(sigc::hide(
		sigc::mem_fun(*this, &EditCommands::on_redo)));
	actions.cut->signal_activate().connect(sigc::hide(
		sigc::mem_fun(*this, &EditCommands::on_cut)));
	actions.copy->signal_activate().connect(sigc::hide(
		sigc::mem_fun(*this, &EditCommands::on_copy)));
	actions.paste->signal_activate().connect(sigc::hide(
		sigc::mem_fun(*this, &EditCommands::on_paste)));
	actions.find->signal_activate().connect(sigc::hide(
		sigc::mem_fun(*this, &EditCommands::on_find)));
	actions.find_next->signal_activate().connect(sigc::hide(
		sigc::mem_fun(*this, &EditCommands::on_find_next)));
	actions.find_prev->signal_activate().connect(sigc::hide(
		sigc::mem_fun(*this, &EditCommands::on_find_prev)));
	actions.find_replace->signal_activate().connect(sigc::hide(
		sigc::mem_fun(*this, &EditCommands::on_find_replace)));
	actions.goto_line->signal_activate().connect(sigc::hide(
		sigc::mem_fun(*this, &EditCommands::on_goto_line)));

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

void Gobby::EditCommands::on_document_removed(SessionView& view)
{
	// TODO: Isn't this emitted by Folder already?
	if(&view == m_current_view)
		on_document_changed(NULL);
}

void Gobby::EditCommands::on_document_changed(SessionView* view)
{
	if(m_current_view != NULL)
	{
		InfTextSession* session = m_current_view->get_session();
		InfAdoptedAlgorithm* algorithm =
			inf_adopted_session_get_algorithm(
				INF_ADOPTED_SESSION(session));
		GtkTextBuffer* buffer = GTK_TEXT_BUFFER(
			m_current_view->get_text_buffer());

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

	m_current_view = dynamic_cast<TextSessionView*>(view);

	if(m_current_view != NULL)
	{
		InfTextSession* session = m_current_view->get_session();
		InfUser* active_user = m_current_view->get_active_user();
		GtkTextBuffer* buffer =
			GTK_TEXT_BUFFER(m_current_view->get_text_buffer());

		m_active_user_changed_connection =
			m_current_view->signal_active_user_changed().connect(
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
		m_actions.find->set_enabled(true);

		if(m_find_dialog.get())
		{
			on_find_text_changed();
		}
		else
		{
			m_actions.find_next->set_enabled(false);
			m_actions.find_prev->set_enabled(false);
		}

		m_actions.find_replace->set_enabled(true);
		m_actions.goto_line->set_enabled(true);
	}
	else
	{
		m_actions.undo->set_enabled(false);
		m_actions.redo->set_enabled(false);
		m_actions.cut->set_enabled(false);
		m_actions.copy->set_enabled(false);
		m_actions.paste->set_enabled(false);
		m_actions.find->set_enabled(false);
		m_actions.find_next->set_enabled(false);
		m_actions.find_prev->set_enabled(false);
		m_actions.find_replace->set_enabled(false);
		m_actions.goto_line->set_enabled(false);
	}
}

void Gobby::EditCommands::on_sync_complete()
{
	if (!m_current_view) {
		g_warning("No current view exists.");
		return;
	}

	InfTextSession* session = m_current_view->get_session();

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

void Gobby::EditCommands::on_active_user_changed(InfUser* active_user)
{
	if (!m_current_view) {
		g_warning("No current view exists.");
		return;
	}

	if(active_user != NULL)
	{
		InfTextSession* session = m_current_view->get_session();
		InfAdoptedAlgorithm* algorithm =
			inf_adopted_session_get_algorithm(
				INF_ADOPTED_SESSION(session));
		GtkTextBuffer* buffer = GTK_TEXT_BUFFER(
			m_current_view->get_text_buffer());

		m_actions.undo->set_enabled(
			inf_adopted_algorithm_can_undo(
				algorithm, INF_ADOPTED_USER(active_user)));
		m_actions.redo->set_enabled(
			inf_adopted_algorithm_can_redo(
				algorithm, INF_ADOPTED_USER(active_user)));

		m_actions.cut->set_enabled(
			gtk_text_buffer_get_has_selection(buffer));
		m_actions.paste->set_enabled(true);
	}
	else
	{
		m_actions.undo->set_enabled(false);
		m_actions.redo->set_enabled(false);
		m_actions.cut->set_enabled(false);
		m_actions.paste->set_enabled(false);
	}
}

void Gobby::EditCommands::on_mark_set()
{
	if (!m_current_view) {
		g_warning("No current view exists.");
		return;
	}

	GtkTextBuffer* buffer =
		GTK_TEXT_BUFFER(m_current_view->get_text_buffer());

	m_actions.copy->set_enabled(
		gtk_text_buffer_get_has_selection(buffer));

	if(m_current_view->get_active_user() != NULL)
	{
		m_actions.cut->set_enabled(
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
	if (!m_current_view) {
		g_warning("No current view exists.");
		return;
	}

	if(INF_ADOPTED_USER(m_current_view->get_active_user()) == user)
		m_actions.undo->set_enabled(can_undo);
}

void Gobby::EditCommands::on_can_redo_changed(InfAdoptedUser* user,
                                              bool can_redo)
{
	if (!m_current_view) {
		g_warning("No current view exists.");
		return;
	}

	if(INF_ADOPTED_USER(m_current_view->get_active_user()) == user)
		m_actions.redo->set_enabled(can_redo);
}

void Gobby::EditCommands::on_find_text_changed()
{
	m_actions.find_next->set_enabled(
		!m_find_dialog->get_find_text().empty());
	m_actions.find_prev->set_enabled(
		!m_find_dialog->get_find_text().empty());
}

// TODO: The following is basically a hack to set the cursor to the position
// where a Undo/Redo has happened. This can be properly fixed as soon as
// libinfinity supports caret-aware requests, by generating undo-caret and
// redo-caret requests.
namespace {
	GtkTextMark* check = NULL;

	void recaret_i(GtkTextBuffer* buffer,
	               GtkTextIter* location,
	               gchar* text,
	               gint len,
	               gpointer user_data)
	{
		if(!check)
		{
			check = gtk_text_buffer_create_mark(buffer, NULL, location, FALSE);
		}
		else
		{
			GtkTextIter iter;
			gtk_text_buffer_get_iter_at_mark(buffer, &iter, check);
			if(gtk_text_iter_get_offset(&iter) < gtk_text_iter_get_offset(location))
				gtk_text_buffer_move_mark(buffer, check, location);
		}
	}

	void recaret_e(GtkTextBuffer* buffer,
	               GtkTextIter* start,
	               GtkTextIter* end,
	               gpointer user_data)
	{
		if(!check)
		{
			check = gtk_text_buffer_create_mark(buffer, NULL, start, FALSE);
		}
		else
		{
			GtkTextIter iter;
			gtk_text_buffer_get_iter_at_mark(buffer, &iter, check);
			if(gtk_text_iter_get_offset(&iter) < gtk_text_iter_get_offset(start))
				gtk_text_buffer_move_mark(buffer, check, start);
		}
	}
}

void Gobby::EditCommands::on_undo()
{
	if (!m_current_view) {
		g_warning("No current view exists.");
		return;
	}

	gulong i_ = g_signal_connect_after(m_current_view->get_text_buffer(), "insert-text", G_CALLBACK(recaret_i), NULL);
	gulong e_ = g_signal_connect_after(m_current_view->get_text_buffer(), "delete-range", G_CALLBACK(recaret_e), NULL);

	inf_adopted_session_undo(
		INF_ADOPTED_SESSION(m_current_view->get_session()),
		INF_ADOPTED_USER(m_current_view->get_active_user()),
		m_current_view->get_undo_grouping().get_undo_size()
	);

	g_signal_handler_disconnect(m_current_view->get_text_buffer(), i_);
	g_signal_handler_disconnect(m_current_view->get_text_buffer(), e_);

	if(check)
	{
		GtkTextIter check_iter;
		gtk_text_buffer_get_iter_at_mark(GTK_TEXT_BUFFER(m_current_view->get_text_buffer()), &check_iter, check);
		gtk_text_buffer_select_range(GTK_TEXT_BUFFER(m_current_view->get_text_buffer()), &check_iter, &check_iter);
		gtk_text_buffer_delete_mark(GTK_TEXT_BUFFER(m_current_view->get_text_buffer()), check);
		check = NULL;
	}

	m_current_view->scroll_to_cursor_position(0.0);
}

void Gobby::EditCommands::on_redo()
{
	if (!m_current_view) {
		g_warning("No current view exists.");
		return;
	}

	gulong i_ = g_signal_connect_after(m_current_view->get_text_buffer(), "insert-text", G_CALLBACK(recaret_i), NULL);
	gulong e_ = g_signal_connect_after(m_current_view->get_text_buffer(), "delete-range", G_CALLBACK(recaret_e), NULL);

	inf_adopted_session_redo(
		INF_ADOPTED_SESSION(m_current_view->get_session()),
		INF_ADOPTED_USER(m_current_view->get_active_user()),
		m_current_view->get_undo_grouping().get_redo_size()
	);

	g_signal_handler_disconnect(m_current_view->get_text_buffer(), i_);
	g_signal_handler_disconnect(m_current_view->get_text_buffer(), e_);

	if(check)
	{
		GtkTextIter check_iter;
		gtk_text_buffer_get_iter_at_mark(GTK_TEXT_BUFFER(m_current_view->get_text_buffer()), &check_iter, check);
		gtk_text_buffer_select_range(GTK_TEXT_BUFFER(m_current_view->get_text_buffer()), &check_iter, &check_iter);
		gtk_text_buffer_delete_mark(GTK_TEXT_BUFFER(m_current_view->get_text_buffer()), check);
		check = NULL;
	}

	m_current_view->scroll_to_cursor_position(0.0);
}

void Gobby::EditCommands::on_cut()
{
	if (!m_current_view) {
		g_warning("No current view exists.");
		return;
	}

	g_assert(m_current_view->get_active_user() != NULL);

	gtk_text_buffer_cut_clipboard(
		GTK_TEXT_BUFFER(m_current_view->get_text_buffer()),
		gtk_clipboard_get(GDK_SELECTION_CLIPBOARD),
		TRUE);

	m_current_view->scroll_to_cursor_position(0.0);
}

void Gobby::EditCommands::on_copy()
{
	if (!m_current_view) {
		g_warning("No current view exists.");
		return;
	}

	gtk_text_buffer_copy_clipboard(
		GTK_TEXT_BUFFER(m_current_view->get_text_buffer()),
		gtk_clipboard_get(GDK_SELECTION_CLIPBOARD));
}

void Gobby::EditCommands::on_paste()
{
	if (!m_current_view) {
		g_warning("No current view exists.");
		return;
	}

	g_assert(m_current_view->get_active_user() != NULL);

	gtk_text_buffer_paste_clipboard(
		GTK_TEXT_BUFFER(m_current_view->get_text_buffer()),
		gtk_clipboard_get(GDK_SELECTION_CLIPBOARD),
		NULL, TRUE);

	m_current_view->scroll_to_cursor_position(0.0);
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
		m_goto_dialog = GotoDialog::create(m_parent, m_folder);

	m_goto_dialog->present();
}

void Gobby::EditCommands::ensure_find_dialog()
{
	if(!m_find_dialog.get())
	{
		m_find_dialog = FindDialog::create(m_parent, m_folder,
		                                   m_status_bar);
		m_find_dialog->signal_find_text_changed().connect(
			sigc::mem_fun(
				*this, &EditCommands::on_find_text_changed));
	}
}
