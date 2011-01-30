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
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "core/textsessionuserview.hpp"

Gobby::TextSessionUserView::
	TextSessionUserView(TextSessionView& view,
                            bool show_disconnected,
                            Preferences::Option<bool>& opt_view,
                            Preferences::Option<unsigned int>& w):
	SessionUserView(view, show_disconnected, opt_view, w)
{
	m_userlist.signal_user_activated().connect(
		sigc::mem_fun(
			*this, &TextSessionUserView::on_user_activated));
}

void Gobby::TextSessionUserView::on_user_activated(InfUser* user)
{
	g_assert(INF_TEXT_IS_USER(user));
	InfTextUser* text_user = INF_TEXT_USER(user);

	// TODO: Instead, move this code to
	// TextSessionView::scroll_to_cursor_position which should take
	// an additional InfTextUser* argument

	GtkSourceBuffer* buffer = get_session_view().get_text_buffer();
	GtkSourceView* view = get_session_view().get_text_view();

	// Use a mark to make sure we scroll it onscreen
	GtkTextIter iter;
	gtk_text_buffer_get_iter_at_offset(
		GTK_TEXT_BUFFER(buffer), &iter,
		inf_text_user_get_caret_position(text_user));
	GtkTextMark* mark = gtk_text_buffer_create_mark(
		GTK_TEXT_BUFFER(buffer), NULL, &iter, FALSE);
	gtk_text_view_scroll_to_mark(
		GTK_TEXT_VIEW(view), mark, 0.0, TRUE, 0.5, 0.5);
	gtk_text_buffer_delete_mark(GTK_TEXT_BUFFER(buffer), mark);
}
