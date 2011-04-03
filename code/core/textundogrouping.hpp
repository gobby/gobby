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
 * Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02110-1301, USA.
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
