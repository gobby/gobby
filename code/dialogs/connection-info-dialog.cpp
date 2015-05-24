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

Gobby::ConnectionInfoDialog::ConnectionInfoDialog(Gtk::Window& parent,
                                                  InfBrowser* browser):
	Gtk::Dialog(_("Connection Information"), parent), m_browser(browser),
	m_connection_store(Gtk::ListStore::create(m_columns)),
	m_connection_tree_view(m_connection_store),
	m_box(Gtk::ORIENTATION_HORIZONTAL, 12),
	m_connection_view(
		INF_GTK_CONNECTION_VIEW(inf_gtk_connection_view_new())),
	m_connection_added_handler(0),
	m_connection_removed_handler(0),
	m_empty(true)
{
	g_object_ref(m_browser);

	if(INFC_IS_BROWSER(browser))
	{
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
		m_image.set_from_icon_name(
			"network-idle", Gtk::ICON_SIZE_DIALOG);

		m_empty = false;
	}
	else if(INFD_IS_DIRECTORY(browser))
	{
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

		if(m_empty)
		{
			Gtk::TreeIter iter = m_connection_store->append();
			(*iter)[m_columns.connection] = NULL;
		}

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

		m_connection_tree_view.append_column(*column);

		Glib::RefPtr<Gtk::TreeSelection> selection =
			m_connection_tree_view.get_selection();
		if(m_empty)
			selection->set_mode(Gtk::SELECTION_NONE);
		else
			selection->set_mode(Gtk::SELECTION_BROWSE);
		selection->signal_changed().connect(
			sigc::mem_fun(*this,
				&ConnectionInfoDialog::on_selection_changed));
		m_connection_tree_view.show();

		m_connection_scroll.set_min_content_width(200);
		m_connection_scroll.set_min_content_height(80);
		m_connection_scroll.set_policy(
			Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
		m_connection_scroll.set_shadow_type(Gtk::SHADOW_IN);
		m_connection_scroll.add(m_connection_tree_view);
		m_connection_scroll.show();

		m_image.set_from_icon_name(
			"network-server", Gtk::ICON_SIZE_DIALOG);
	}

	m_image.set_valign(Gtk::ALIGN_START);
	m_image.show();
	if(!m_empty)
		gtk_widget_show(GTK_WIDGET(m_connection_view));

	m_box.pack_start(m_image, Gtk::PACK_SHRINK);
	m_box.pack_start(m_connection_scroll, Gtk::PACK_SHRINK);

	gtk_box_pack_start(
		m_box.gobj(), GTK_WIDGET(m_connection_view),
		FALSE, FALSE, 0);
	m_box.show();

	get_vbox()->set_spacing(6);
	get_vbox()->pack_start(m_box, Gtk::PACK_EXPAND_WIDGET);

	set_resizable(false);
	set_border_width(12);
}

Gobby::ConnectionInfoDialog::~ConnectionInfoDialog()
{
	if(m_connection_added_handler != 0)
	{
		g_signal_handler_disconnect(
			G_OBJECT(m_browser), m_connection_added_handler);
	}

	if(m_connection_removed_handler != 0)
	{
		g_signal_handler_disconnect(
			G_OBJECT(m_browser), m_connection_removed_handler);
	}

	g_object_unref(m_browser);
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
			m_connection_tree_view.get_selection()->
				set_mode(Gtk::SELECTION_BROWSE);
			m_connection_tree_view.get_selection()->select(iter);
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
			m_connection_tree_view.get_selection()->
				set_mode(Gtk::SELECTION_BROWSE);
			m_connection_tree_view.get_selection()->select(iter);
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
			m_connection_tree_view.get_selection()->
				set_mode(Gtk::SELECTION_NONE);
			m_empty = true;
		}
	}
}

void Gobby::ConnectionInfoDialog::on_selection_changed()
{
	if(m_connection_tree_view.get_selection()->count_selected_rows() > 0)
	{
		Gtk::TreeIter iter =
			m_connection_tree_view.get_selection()->
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
