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

#ifndef _GOBBY_GOTODIALOG_HPP_
#define _GOBBY_GOTODIALOG_HPP_

#include "core/folder.hpp"
#include "core/sessionview.hpp"

#include <gtkmm/dialog.h>
#include <gtkmm/table.h>
#include <gtkmm/label.h>
#include <gtkmm/spinbutton.h>

namespace Gobby
{

class GotoDialog: public Gtk::Dialog
{
public:
	GotoDialog(Gtk::Window& parent, Folder& m_folder);
	~GotoDialog();

protected:
	static void on_changed_static(GtkTextBuffer* buffer,
	                              gpointer user_data)
	{
		static_cast<GotoDialog*>(user_data)->on_changed();
	}

	virtual void on_show();
	virtual void on_response(int id);

	void on_document_changed(SessionView* view);
	void on_changed();

	Folder& m_folder;

	Gtk::Table m_table;

	Gtk::Label m_label_line;
	Gtk::SpinButton m_entry_line;

	TextSessionView* m_current_view;
	gulong m_changed_handler;
};

}

#endif // _GOBBY_GOTODIALOG_HPP_
