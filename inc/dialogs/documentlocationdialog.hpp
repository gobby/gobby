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

#ifndef _GOBBY_DOCUMENTLOCATIONDIALOG_HPP_
#define _GOBBY_DOCUMENTLOCATIONDIALOG_HPP_

#include <gtkmm/dialog.h>
#include <gtkmm/table.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>
#include <gtkmm/scrolledwindow.h>

#include <libinfgtk/inf-gtk-browser-model.h>
#include <libinfgtk/inf-gtk-browser-model-filter.h>
#include <libinfgtk/inf-gtk-browser-view.h>

namespace Gobby
{

class DocumentLocationDialog : public Gtk::Dialog
{
public:
	DocumentLocationDialog(Gtk::Window& parent,
	                       InfGtkBrowserModel* model);
	~DocumentLocationDialog();

	Glib::ustring get_document_name() const;
	void set_document_name(const Glib::ustring& document_name);

	InfcBrowser* get_selected_directory(InfcBrowserIter* iter) const;
protected:
	virtual void on_show();

	static void on_selection_changed_static(InfGtkBrowserView* view,
	                                        GtkTreeIter* iter,
	                                        gpointer user_data)
	{
		static_cast<DocumentLocationDialog*>(
			user_data)->on_selection_changed(iter);
	}

	static void on_row_changed_static(GtkTreeModel* model,
	                                  GtkTreePath* path,
					  GtkTreeIter* iter,
	                                  gpointer user_data)
	{
		static_cast<DocumentLocationDialog*>(
			user_data)->on_row_changed(path, iter);
	}

	static gboolean filter_visible_func_static(GtkTreeModel* model,
	                                           GtkTreeIter* iter,
	                                           gpointer user_data)
	{
		return static_cast<DocumentLocationDialog*>(
			user_data)->filter_visible_func(model, iter);
	}

	void on_selection_changed(GtkTreeIter* iter);
	void on_row_changed(GtkTreePath* path, GtkTreeIter* iter);

	bool filter_visible_func(GtkTreeModel* model, GtkTreeIter* iter);

	Gtk::Table m_table;
	Gtk::Label m_name_label;
	Gtk::Entry m_name_entry;

	Gtk::Label m_location_label;
	InfGtkBrowserModelFilter* m_filter_model;
	Gtk::ScrolledWindow m_scroll;
	InfGtkBrowserView* m_view;
};

}

#endif // _GOBBY_DOCUMENTLOCATIONDIALOG_HPP_

