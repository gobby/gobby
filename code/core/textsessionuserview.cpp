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
