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
