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

#include "dialogs/connection-info-dialog.hpp"

#include "util/i18n.hpp"

#include <libinfinity/client/infc-browser.h>

Gobby::ConnectionInfoDialog::ConnectionInfoDialog(
	GtkDialog* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
:
	Gtk::Dialog(cobject), m_browser(NULL),
	m_connection_store(Gtk::ListStore::create(m_columns)),
	m_connection_added_handler(0),
	m_connection_removed_handler(0),
	m_empty(true)
{
	builder->get_widget("image", m_image);
	builder->get_widget("treeview", m_connection_tree_view);
	builder->get_widget("scrolled-window", m_connection_scroll);

	m_connection_view = INF_GTK_CONNECTION_VIEW(
		gtk_builder_get_object(builder->gobj(), "connection-info"));

	m_connection_tree_view->set_model(m_connection_store);

	Gtk::CellRendererPixbuf* icon_renderer =
		Gtk::manage(new Gtk::CellRendererPixbuf);

	Gtk::CellRendererText* name_renderer =
		Gtk::manage(new Gtk::CellRendererText);

	Gtk::TreeViewColumn* column =
		Gtk::manage(new Gtk::TreeViewColumn(_("Connections")));
	column->pack_start(*icon_renderer, false);
	column->pack_start(*name_renderer, true);

	column->set_cell_data_func(
		*icon_renderer,
		sigc::mem_fun(*this,
			&ConnectionInfoDialog::icon_cell_data_func));
	column->set_cell_data_func(
		*name_renderer,
		sigc::mem_fun(*this,
			&ConnectionInfoDialog::name_cell_data_func));

	m_connection_tree_view->append_column(*column);

	Glib::RefPtr<Gtk::TreeSelection> selection =
		m_connection_tree_view->get_selection();
	selection->signal_changed().connect(
		sigc::mem_fun(*this,
			&ConnectionInfoDialog::on_selection_changed));
}

Gobby::ConnectionInfoDialog::~ConnectionInfoDialog()
{
	set_browser(NULL);
}

std::auto_ptr<Gobby::ConnectionInfoDialog>
Gobby::ConnectionInfoDialog::create(Gtk::Window& parent, InfBrowser* browser)
{
	// Make sure the GType for InfGtkConnectionView is registered,
	// since the UI definition contains a widget of this kind, and
	// GtkBuilder will want to look it up.
	// Also, make sure the call does not get optimized out.
	GType type = inf_gtk_connection_view_get_type();
	if(type == 0)
		throw std::logic_error("inf_gtk_connection_view_get_type");

	Glib::RefPtr<Gtk::Builder> builder =
		Gtk::Builder::create_from_resource(
			"/de/0x539/gobby/ui/connection-info-dialog.ui");

	ConnectionInfoDialog* dialog_ptr;
	builder->get_widget_derived("ConnectionInfoDialog", dialog_ptr);
	std::auto_ptr<ConnectionInfoDialog> dialog(dialog_ptr);
	dialog->set_transient_for(parent);
	dialog->set_browser(browser);
	return dialog;
}

void Gobby::ConnectionInfoDialog::set_browser(InfBrowser* browser)
{
	if(m_browser != NULL)
	{
		if(m_connection_added_handler != 0)
		{
			g_signal_handler_disconnect(
				G_OBJECT(m_browser),
				m_connection_added_handler);
		}

		if(m_connection_removed_handler != 0)
		{
			g_signal_handler_disconnect(
				G_OBJECT(m_browser),
				m_connection_removed_handler);
		}

		g_object_unref(m_browser);
	}

	m_browser = browser;
	if(m_browser != NULL)
		g_object_ref(m_browser);

	if(INFC_IS_BROWSER(browser))
	{
		m_empty = false;

		InfXmlConnection* conn = infc_browser_get_connection(
			INFC_BROWSER(browser));
		if(INF_IS_XMPP_CONNECTION(conn))
		{
			inf_gtk_connection_view_set_connection(
				m_connection_view,
				INF_XMPP_CONNECTION(conn));
		}

		/* TODO: Show this corresponding to connection status, or
		 * network-server if we are a server. */
		m_image->set_from_icon_name(
			"network-idle", Gtk::ICON_SIZE_DIALOG);
		m_connection_scroll->hide();
	}
	else if(INFD_IS_DIRECTORY(browser))
	{
		m_connection_store->clear();
		m_empty = true;

		infd_directory_foreach_connection(
			INFD_DIRECTORY(browser),
			&ConnectionInfoDialog::foreach_connection_func_static,
			this);

		m_connection_added_handler = g_signal_connect(
			G_OBJECT(browser),
			"connection-added",
			G_CALLBACK(on_connection_added_static),
			this);

		m_connection_removed_handler = g_signal_connect(
			G_OBJECT(browser),
			"connection-removed",
			G_CALLBACK(on_connection_removed_static),
			this);

		Glib::RefPtr<Gtk::TreeSelection> selection =
			m_connection_tree_view->get_selection();
		if(m_empty)
		{
			selection->set_mode(Gtk::SELECTION_NONE);

			Gtk::TreeIter iter = m_connection_store->append();
			(*iter)[m_columns.connection] = NULL;
		}
		else
		{
			selection->set_mode(Gtk::SELECTION_BROWSE);
		}

		m_image->set_from_icon_name(
			"network-server", Gtk::ICON_SIZE_DIALOG);
	}

	if(!m_empty)
		gtk_widget_show(GTK_WIDGET(m_connection_view));
	else
		gtk_widget_hide(GTK_WIDGET(m_connection_view));
}

void Gobby::ConnectionInfoDialog::foreach_connection_func(
	InfXmlConnection* conn)
{
	if(INF_IS_XMPP_CONNECTION(conn))
	{
		g_assert(find_connection(INF_XMPP_CONNECTION(conn)) ==
		         m_connection_store->children().end());

		if(m_empty)
			m_connection_store->clear();

		Gtk::TreeIter iter = m_connection_store->append();
		(*iter)[m_columns.connection] = INF_XMPP_CONNECTION(conn);

		if(m_empty)
		{
			gtk_widget_show(GTK_WIDGET(m_connection_view));
			m_connection_tree_view->get_selection()->
				set_mode(Gtk::SELECTION_BROWSE);
			m_connection_tree_view->get_selection()->select(iter);
			m_empty = false;
		}
	}
}

void Gobby::ConnectionInfoDialog::on_connection_added(
	InfXmlConnection* conn)
{
	if(INF_IS_XMPP_CONNECTION(conn))
	{
		g_assert(find_connection(INF_XMPP_CONNECTION(conn)) ==
		         m_connection_store->children().end());

		if(m_empty)
			m_connection_store->clear();

		Gtk::TreeIter iter = m_connection_store->append();
		(*iter)[m_columns.connection] = INF_XMPP_CONNECTION(conn);

		if(m_empty)
		{
			gtk_widget_show(GTK_WIDGET(m_connection_view));
			m_connection_tree_view->get_selection()->
				set_mode(Gtk::SELECTION_BROWSE);
			m_connection_tree_view->get_selection()->select(iter);
			m_empty = false;
		}
	}
}

void Gobby::ConnectionInfoDialog::on_connection_removed(
	InfXmlConnection* conn)
{
	if(INF_IS_XMPP_CONNECTION(conn))
	{
		Gtk::TreeIter iter =
			find_connection(INF_XMPP_CONNECTION(conn));
		g_assert(iter != m_connection_store->children().end());
		m_connection_store->erase(iter);

		g_assert(!m_empty);
		if(m_connection_store->children().empty())
		{
			Gtk::TreeIter iter = m_connection_store->append();
			(*iter)[m_columns.connection] = NULL;

			gtk_widget_hide(GTK_WIDGET(m_connection_view));
			m_connection_tree_view->get_selection()->
				set_mode(Gtk::SELECTION_NONE);
			m_empty = true;
		}
	}
}

void Gobby::ConnectionInfoDialog::on_selection_changed()
{
	if(m_connection_tree_view->get_selection()->count_selected_rows() > 0)
	{
		Gtk::TreeIter iter =
			m_connection_tree_view->get_selection()->
				get_selected();
		inf_gtk_connection_view_set_connection(
			m_connection_view, (*iter)[m_columns.connection]);
	}
	else
	{
		inf_gtk_connection_view_set_connection(
			m_connection_view, NULL);
	}
}

void Gobby::ConnectionInfoDialog::icon_cell_data_func(
	Gtk::CellRenderer* renderer, const Gtk::TreeIter& iter)
{
	Gtk::CellRendererPixbuf* pixbuf_renderer =
		dynamic_cast<Gtk::CellRendererPixbuf*>(renderer);
	g_assert(pixbuf_renderer);

	InfXmppConnection* conn = (*iter)[m_columns.connection];

	// TODO: stock_size?
	pixbuf_renderer->property_icon_name() = "network-idle";
	pixbuf_renderer->property_visible() = (conn != NULL);
}

void Gobby::ConnectionInfoDialog::name_cell_data_func(
	Gtk::CellRenderer* renderer, const Gtk::TreeIter& iter)
{
	Gtk::CellRendererText* text_renderer =
		dynamic_cast<Gtk::CellRendererText*>(renderer);
	g_assert(text_renderer);

	gchar* hostname;
	InfXmppConnection* conn = (*iter)[m_columns.connection];

	if(conn != NULL)
	{
		g_object_get(
			G_OBJECT(conn), "remote-hostname", &hostname, NULL);
		text_renderer->property_text() = hostname;
		g_free(hostname);
	}
	else
	{
		text_renderer->property_text() =
			_("Nobody is connected to this computer");
		text_renderer->property_wrap_width() = 170;
	}

	text_renderer->property_visible() = true;
}

Gtk::TreeIter Gobby::ConnectionInfoDialog::find_connection(
	InfXmppConnection* conn)
{
	const Gtk::TreeModel::Children& children =
		m_connection_store->children();
	for(Gtk::TreeIter iter = children.begin();
	    iter != children.end(); ++iter)
	{
		if(conn == (*iter)[m_columns.connection])
			return iter;
	}

	return children.end();
}
