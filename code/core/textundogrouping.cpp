/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2010 Armin Burgmeier <armin@arbur.net>
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

#include "core/textundogrouping.hpp"

Gobby::TextUndoGrouping::TextUndoGrouping(InfAdoptedAlgorithm* algorithm,
                                          InfTextUser* user,
                                          GtkTextBuffer* buffer):
	m_buffer(buffer), m_grouping(inf_text_undo_grouping_new())
{
	g_object_ref(m_buffer);

	inf_adopted_undo_grouping_set_algorithm(
		INF_ADOPTED_UNDO_GROUPING(m_grouping),
		algorithm,
		INF_ADOPTED_USER(user));

	m_begin_user_action_handle = g_signal_connect(
		G_OBJECT(m_buffer), "begin-user-action",
		G_CALLBACK(on_begin_user_action_static), this);
	m_end_user_action_handle = g_signal_connect(
		G_OBJECT(m_buffer), "end-user-action",
		G_CALLBACK(on_end_user_action_static), this);
}

Gobby::TextUndoGrouping::~TextUndoGrouping()
{
	g_signal_handler_disconnect(m_buffer, m_begin_user_action_handle);
	g_signal_handler_disconnect(m_buffer, m_end_user_action_handle);

	g_object_unref(m_grouping);
	g_object_unref(m_buffer);
}

guint Gobby::TextUndoGrouping::get_undo_size() const
{
	return inf_adopted_undo_grouping_get_undo_size(
		INF_ADOPTED_UNDO_GROUPING(m_grouping));
}

guint Gobby::TextUndoGrouping::get_redo_size() const
{
	return inf_adopted_undo_grouping_get_redo_size(
		INF_ADOPTED_UNDO_GROUPING(m_grouping));
}

void Gobby::TextUndoGrouping::on_begin_user_action()
{
	// TODO: For paste actions we should not allow merging
	inf_adopted_undo_grouping_start_group(
		INF_ADOPTED_UNDO_GROUPING(m_grouping),
		TRUE);
}

void Gobby::TextUndoGrouping::on_end_user_action()
{
	// TODO: For paste actions we should not allow merging
	inf_adopted_undo_grouping_end_group(
		INF_ADOPTED_UNDO_GROUPING(m_grouping),
		TRUE);
}
