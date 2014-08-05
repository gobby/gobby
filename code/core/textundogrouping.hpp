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

#ifndef _GOBBY_TEXTUNDOGROUPING_HPP_
#define _GOBBY_TEXTUNDOGROUPING_HPP_

#include <libinftext/inf-text-undo-grouping.h>
#include <libinftext/inf-text-user.h>
#include <libinfinity/adopted/inf-adopted-algorithm.h>

#include <gtk/gtk.h>

namespace Gobby
{

class TextUndoGrouping
{
public:
	TextUndoGrouping(InfAdoptedAlgorithm* algorithm,
	                 InfTextUser* user,
	                 GtkTextBuffer* buffer);
	~TextUndoGrouping();

	guint get_undo_size() const;
	guint get_redo_size() const;

protected:
	static void
	on_begin_user_action_static(GtkTextBuffer* buffer,
	                            gpointer user_data)
	{
		static_cast<TextUndoGrouping*>(user_data)->
			on_begin_user_action();
	}

	static void
	on_end_user_action_static(GtkTextBuffer* buffer,
	                          gpointer user_data)
	{
		static_cast<TextUndoGrouping*>(user_data)->
			on_end_user_action();
	}

	void on_begin_user_action();
	void on_end_user_action();

	GtkTextBuffer* m_buffer;
	InfTextUndoGrouping* m_grouping;

	gulong m_begin_user_action_handle;
	gulong m_end_user_action_handle;
};

}

#endif // _GOBBY_TEXTSESSIONVIEW_HPP_
