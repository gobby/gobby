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

#include "dialogs/document-location-dialog.hpp"
#include "util/i18n.hpp"

Gobby::DocumentLocationDialog::DocumentLocationDialog(
	GtkDialog* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
:
	Gtk::Dialog(cobject), m_filter_model(NULL)
{
	builder->get_widget("document-name-label", m_name_label);
	builder->get_widget("document-name", m_name_entry);
	builder->get_widget("location-label", m_location_label);

	m_view = INF_GTK_BROWSER_VIEW(
		gtk_builder_get_object(GTK_BUILDER(builder->gobj()), "view"));

	g_signal_connect(m_view, "selection-changed",
	                 G_CALLBACK(on_selection_changed_static), this);

	add_button(_("_Cancel"), Gtk::RESPONSE_CANCEL);
	add_button(_("_Open"), Gtk::RESPONSE_ACCEPT);

	set_response_sensitive(Gtk::RESPONSE_ACCEPT, false);
	set_default_response(Gtk::RESPONSE_ACCEPT);
}

std::auto_ptr<Gobby::DocumentLocationDialog>
Gobby::DocumentLocationDialog::create(Gtk::Window& parent,
                                      InfGtkBrowserModel* model)
{
	Glib::RefPtr<Gtk::Builder> builder =
		Gtk::Builder::create_from_resource(
			"/de/0x539/gobby/ui/document-location-dialog.ui");

	DocumentLocationDialog* dialog_ptr = NULL;
	builder->get_widget_derived("DocumentLocationDialog", dialog_ptr);
	std::auto_ptr<DocumentLocationDialog> dialog(dialog_ptr);

	dialog->set_transient_for(parent);

	dialog->m_filter_model = inf_gtk_browser_model_filter_new(model);
	gtk_tree_view_set_model(
		GTK_TREE_VIEW(dialog->m_view),
		GTK_TREE_MODEL(dialog->m_filter_model));

	g_signal_connect(dialog->m_filter_model, "row-changed",
	                 G_CALLBACK(on_row_changed_static), dialog.get());

	// Hide non-subdirectories:
	gtk_tree_model_filter_set_visible_func(
		GTK_TREE_MODEL_FILTER(dialog->m_filter_model),
		filter_visible_func_static, dialog.get(), NULL);

	// Required to filter initial content:
	gtk_tree_model_filter_refilter(
		GTK_TREE_MODEL_FILTER(dialog->m_filter_model));

	// Default is single document mode:
	dialog->set_single_document_mode();

	return dialog;
}

Gobby::DocumentLocationDialog::~DocumentLocationDialog()
{
	g_object_unref(m_filter_model);
	m_filter_model = NULL;
}

Glib::ustring Gobby::DocumentLocationDialog::get_document_name() const
{
	return m_name_entry->get_text();
}

void Gobby::DocumentLocationDialog::set_document_name(
	const Glib::ustring& document_name)
{
	m_name_entry->set_text(document_name);
}

InfBrowser* Gobby::DocumentLocationDialog::get_selected_directory(
	InfBrowserIter* iter) const
{
	GtkTreeIter tree_iter;
	if(inf_gtk_browser_view_get_selected(m_view, &tree_iter))
	{
		InfGtkBrowserModelStatus status =
			INF_GTK_BROWSER_MODEL_CONNECTED;

		GtkTreeIter dummy_iter;
		if(!gtk_tree_model_iter_parent(GTK_TREE_MODEL(m_filter_model),
		                               &dummy_iter, &tree_iter))
		{
			gtk_tree_model_get(
				GTK_TREE_MODEL(m_filter_model),
				&tree_iter,
				INF_GTK_BROWSER_MODEL_COL_STATUS, &status,
				-1);
		}

		if(status == INF_GTK_BROWSER_MODEL_CONNECTED)
		{
			InfBrowserIter* browser_iter;
			InfBrowser* browser;

			gtk_tree_model_get(
				GTK_TREE_MODEL(m_filter_model),
				&tree_iter,
				INF_GTK_BROWSER_MODEL_COL_BROWSER, &browser,
				INF_GTK_BROWSER_MODEL_COL_NODE, &browser_iter,
				-1);

			*iter = *browser_iter;
			inf_browser_iter_free(browser_iter);
			g_object_unref(browser);
			return browser;
		}
		else
		{
			return NULL;
		}
	}
	else
	{
		return NULL;
	}
}

InfGtkBrowserModel* Gobby::DocumentLocationDialog::get_browser_model() const
{
	return INF_GTK_BROWSER_MODEL(
		gtk_tree_model_filter_get_model(
			GTK_TREE_MODEL_FILTER(m_filter_model)));
}

void Gobby::DocumentLocationDialog::set_single_document_mode()
{
	set_title(_("Select document's target location"));
	m_location_label->set_text(
		_("Choose a directory to create the document into:"));
	m_name_label->show();
	m_name_entry->show();
}

void Gobby::DocumentLocationDialog::set_multiple_document_mode()
{
	set_title(_("Select documents' target location"));
	m_location_label->set_text(
		_("Choose a directory to create the documents into:"));
	m_name_label->hide();
	m_name_entry->hide();
}

void Gobby::DocumentLocationDialog::on_show()
{
	Gtk::Dialog::on_show();

	if(m_name_entry->get_visible())
	{
		m_name_entry->select_region(
			0, m_name_entry->get_text_length());
		m_name_entry->grab_focus();
	}
	else
	{
		gtk_widget_grab_focus(GTK_WIDGET(m_view));
	}
}

void Gobby::DocumentLocationDialog::on_selection_changed(GtkTreeIter* iter)
{
	gboolean accept_sensitive = false;
	if(iter != NULL)
	{
		GtkTreeIter dummy_iter;
		if(gtk_tree_model_iter_parent(GTK_TREE_MODEL(m_filter_model),
		                              &dummy_iter, iter))
		{
			// Child: Always OK
			accept_sensitive = true;
		}
		else
		{
			// Top-level node: Check status
			InfGtkBrowserModelStatus status;
			gtk_tree_model_get(GTK_TREE_MODEL(m_filter_model),
			                   iter,
			                   INF_GTK_BROWSER_MODEL_COL_STATUS,
			                   &status,
			                   -1);

			accept_sensitive =
				(status == INF_GTK_BROWSER_MODEL_CONNECTED);
		}
	}

	set_response_sensitive(Gtk::RESPONSE_ACCEPT, accept_sensitive);
}

void Gobby::DocumentLocationDialog::on_row_changed(GtkTreePath* path,
                                                   GtkTreeIter* iter)
{
	// If a toplevel entry is selected, and the status changed to
	// connected, then make the Accept button sensitive. On the other
	// hand, if status changed to something else, then
	// make it insensitive.
	GtkTreeIter sel_iter;

	if(inf_gtk_browser_view_get_selected(m_view, &sel_iter))
	{
		// We cannot compare the iterators directly here, since
		// GtkTreeModelFilter seems to allow two iterators pointing to
		// the same row but having a different user_data3 field. We
		// therefore compare the paths instead.
		GtkTreePath* sel_path = gtk_tree_model_get_path(
			GTK_TREE_MODEL(m_filter_model), &sel_iter);
		if(gtk_tree_path_compare(path, sel_path) == 0)
		{
			GtkTreeIter dummy_iter;
			if(!gtk_tree_model_iter_parent(
				GTK_TREE_MODEL(m_filter_model), &dummy_iter,
				iter))
			{
				InfGtkBrowserModelStatus s;
				gtk_tree_model_get(
					GTK_TREE_MODEL(m_filter_model), iter,
					INF_GTK_BROWSER_MODEL_COL_STATUS, &s,
					-1);

				set_response_sensitive(
					Gtk::RESPONSE_ACCEPT,
					s == INF_GTK_BROWSER_MODEL_CONNECTED
				);
			}
		}

		gtk_tree_path_free(sel_path);
	}
}

bool Gobby::DocumentLocationDialog::filter_visible_func(GtkTreeModel* model,
                                                        GtkTreeIter* iter)
{
	GtkTreeIter dummy_iter;
	if(!gtk_tree_model_iter_parent(model, &dummy_iter, iter))
	{
		return true;
	}
	else
	{
		InfBrowserIter* browser_iter;
		InfBrowser* browser;

		gtk_tree_model_get(
			model, iter,
			INF_GTK_BROWSER_MODEL_COL_BROWSER, &browser,
			INF_GTK_BROWSER_MODEL_COL_NODE, &browser_iter,
			-1);

		bool result =
			inf_browser_is_subdirectory(browser, browser_iter);

		inf_browser_iter_free(browser_iter);
		g_object_unref(browser);

		return result;
	}
}
