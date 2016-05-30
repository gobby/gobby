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

#ifndef _GOBBY_DOCUMENTLOCATIONDIALOG_HPP_
#define _GOBBY_DOCUMENTLOCATIONDIALOG_HPP_

#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>
#include <gtkmm/builder.h>

#include <libinfgtk/inf-gtk-browser-model.h>
#include <libinfgtk/inf-gtk-browser-model-filter.h>
#include <libinfgtk/inf-gtk-browser-view.h>

namespace Gobby
{

class DocumentLocationDialog : public Gtk::Dialog
{
private:
	friend class Gtk::Builder;
	DocumentLocationDialog(GtkDialog* cobject,
	                       const Glib::RefPtr<Gtk::Builder>& builder);
public:
	static std::unique_ptr<DocumentLocationDialog> create(
		Gtk::Window& parent,
		InfGtkBrowserModel* model);

	~DocumentLocationDialog();

	Glib::ustring get_document_name() const;
	void set_document_name(const Glib::ustring& document_name);

	InfBrowser* get_selected_directory(InfBrowserIter* iter) const;
	InfGtkBrowserModel* get_browser_model() const;

	void set_single_document_mode();
	void set_multiple_document_mode();
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

	Gtk::Label* m_name_label;
	Gtk::Entry* m_name_entry;
	Gtk::Label* m_location_label;

	InfGtkBrowserModelFilter* m_filter_model;
	InfGtkBrowserView* m_view;
};

}

#endif // _GOBBY_DOCUMENTLOCATIONDIALOG_HPP_

