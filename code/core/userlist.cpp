/* gobby - A GTKmm driven libobby client
 * Copyright (C) 2005, 2006 0x539 dev group
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

#include "core/userlist.hpp"

#include "util/i18n.hpp"
#include "util/color.hpp"

#include <gtkmm/scrolledwindow.h>
#include <gtkmm/stock.h>

#include <cstring>

namespace
{
	typedef sigc::slot<void, InfTextUser*> ForeachUserFunc;

	void foreach_user_ctor_func(InfUser* user, gpointer user_data)
	{
		(*static_cast<ForeachUserFunc*>(user_data))
			(INF_TEXT_USER(user));
	}

	void draw_pixel(const Glib::RefPtr<Gdk::Pixbuf>& pixbuf, int x, int y,
	                const Gdk::Color& color)
	{
		guint8* pixels = pixbuf->get_pixels();
		guint8* pixel = pixels + y * pixbuf->get_rowstride() +
		                         x * pixbuf->get_n_channels();
		*pixel++ = (color.get_red() >> 8);
		*pixel++ = (color.get_green() >> 8);
		*pixel++ = (color.get_blue() >> 8);
	}

	void draw_vline(const Glib::RefPtr<Gdk::Pixbuf>& pixbuf, int x,
	                int y1, int y2, const Gdk::Color& color)
	{
		for(int y = y1; y < y2; ++ y)
			draw_pixel(pixbuf, x, y, color);
	}

	void draw_hline(const Glib::RefPtr<Gdk::Pixbuf>& pixbuf, int x1,
	                int x2, int y, const Gdk::Color& color)
	{
		for(int x = x1; x < x2; ++ x)
			draw_pixel(pixbuf, x, y, color);
	}

	void draw_rectangle(const Glib::RefPtr<Gdk::Pixbuf>& pixbuf, int x1,
	                    int y1, int x2, int y2, const Gdk::Color& color)
	{
		for(int y = y1; y < y2; ++ y)
			for(int x = x1; x < x2; ++ x)
				draw_pixel(pixbuf, x, y, color);
	}

	Glib::RefPtr<Gdk::Pixbuf> generate_user_color_pixbuf(gdouble hue)
	{
		// TODO: Play around with hue, saturation and value to get
		// a cool effect instead of a monochromatic icon.
		Gdk::Color color = Gobby::hue_to_gdk_color(hue, 0.35, 1.0);
		Gdk::Color black("#000000");

		int width, height;
		Gtk::IconSize::lookup(Gtk::ICON_SIZE_MENU, width, height);

		Glib::RefPtr<Gdk::Pixbuf> pixbuf(
			Gdk::Pixbuf::create(
				Gdk::COLORSPACE_RGB, false, 8,
				width, height));

		draw_hline(pixbuf, 0, width, 0, black);
		draw_hline(pixbuf, 0, width, height - 1, black);
		draw_vline(pixbuf, 0, 1, height - 1, black);
		draw_vline(pixbuf, width-1, 1, height - 1, black);
		draw_rectangle(pixbuf, 1, 1, width - 1, height - 1, color);

		return pixbuf;
	}
}

Gobby::UserList::UserList(InfTextSession* session):
	m_session(session), m_store(Gtk::ListStore::create(m_columns)),
	m_view(m_store)
{
	m_store->set_sort_func(m_columns.user,
	                       sigc::mem_fun(*this, &UserList::sort_func));

	m_store->set_sort_column(m_columns.user, Gtk::SORT_ASCENDING);

	InfUserTable* table =
		inf_session_get_user_table(INF_SESSION(session));

	m_add_user_handle = g_signal_connect(
		G_OBJECT(table), "add-user",
		G_CALLBACK(on_add_user_static), this);

	ForeachUserFunc slot(sigc::mem_fun(*this, &UserList::on_add_user));
	inf_user_table_foreach_user(table, foreach_user_ctor_func, &slot);

	Gtk::CellRendererPixbuf* icon_renderer =
		Gtk::manage(new Gtk::CellRendererPixbuf);

	Gtk::CellRendererPixbuf* color_renderer =
		Gtk::manage(new Gtk::CellRendererPixbuf);

	Gtk::CellRendererText* name_renderer =
		Gtk::manage(new Gtk::CellRendererText);

	Gtk::TreeViewColumn* column =
		Gtk::manage(new Gtk::TreeViewColumn(_("Users")));
	column->pack_start(*icon_renderer, false);
	column->pack_start(*color_renderer, false);
	column->pack_start(*name_renderer, true);

	column->set_cell_data_func(
		*icon_renderer,
		sigc::mem_fun(*this, &UserList::icon_cell_data_func));
	column->set_cell_data_func(
		*color_renderer,
		sigc::mem_fun(*this, &UserList::color_cell_data_func));
	column->set_cell_data_func(
		*name_renderer,
		sigc::mem_fun(*this, &UserList::name_cell_data_func));

	column->set_spacing(6);
	m_view.append_column(*column);
	m_view.get_selection()->set_mode(Gtk::SELECTION_NONE);
	m_view.set_headers_visible(false);
	m_view.show();

	Gtk::ScrolledWindow* scroll = Gtk::manage(new Gtk::ScrolledWindow);
	scroll->set_shadow_type(Gtk::SHADOW_IN);
	scroll->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	scroll->add(m_view);
	scroll->show();

	pack_start(*scroll, Gtk::PACK_EXPAND_WIDGET);
}

Gobby::UserList::~UserList()
{
	InfUserTable* table =
		inf_session_get_user_table(INF_SESSION(m_session));
	g_signal_handler_disconnect(G_OBJECT(table), m_add_user_handle);

	const Gtk::TreeModel::Children& children = m_store->children();
	for(Gtk::TreeIter iter = children.begin();
	    iter != children.end(); ++ iter)
	{
		InfTextUser* user = (*iter)[m_columns.user];
		gulong notify_hue_handle =
			(*iter)[m_columns.notify_hue_handle];
		gulong notify_status_handle =
			(*iter)[m_columns.notify_status_handle];

		g_signal_handler_disconnect(G_OBJECT(user),
		                            notify_hue_handle);
		g_signal_handler_disconnect(G_OBJECT(user),
		                            notify_status_handle);
	}
}

void Gobby::UserList::icon_cell_data_func(Gtk::CellRenderer* renderer,
                                          const Gtk::TreeIter& iter)
{
	Gtk::CellRendererPixbuf* pixbuf_renderer =
		dynamic_cast<Gtk::CellRendererPixbuf*>(renderer);
	g_assert(pixbuf_renderer);

	pixbuf_renderer->property_stock_size() = Gtk::ICON_SIZE_MENU;

	InfTextUser* user = (*iter)[m_columns.user];
	switch(inf_user_get_status(INF_USER(user)))
	{
	case INF_USER_ACTIVE:
	case INF_USER_INACTIVE:
		pixbuf_renderer->property_stock_id() =
			Gtk::Stock::CONNECT.id;
		break;
	case INF_USER_UNAVAILABLE:
		pixbuf_renderer->property_stock_id() =
			Gtk::Stock::DISCONNECT.id;
		break;
	default:
		g_assert_not_reached();
		break;
	}
}

void Gobby::UserList::color_cell_data_func(Gtk::CellRenderer* renderer,
                                           const Gtk::TreeIter& iter)
{
	Gtk::CellRendererPixbuf* pixbuf_renderer =
		dynamic_cast<Gtk::CellRendererPixbuf*>(renderer);
	g_assert(pixbuf_renderer);

	pixbuf_renderer->property_pixbuf() = (*iter)[m_columns.color];
}

void Gobby::UserList::name_cell_data_func(Gtk::CellRenderer* renderer,
                                          const Gtk::TreeIter& iter)
{
	Gtk::CellRendererText* text_renderer =
		dynamic_cast<Gtk::CellRendererText*>(renderer);
	g_assert(text_renderer);

	InfTextUser* user = (*iter)[m_columns.user];
	switch(inf_user_get_status(INF_USER(user)))
	{
	case INF_USER_ACTIVE:
		text_renderer->property_foreground_set() = false;
		break;
	case INF_USER_INACTIVE:
		text_renderer->property_foreground() = "#606060";
		break;
	case INF_USER_UNAVAILABLE:
		text_renderer->property_foreground() = "#a0a0a0";
		break;
	}

	text_renderer->property_text() = inf_user_get_name(INF_USER(user));
}

int Gobby::UserList::sort_func(const Gtk::TreeIter& iter1,
                               const Gtk::TreeIter& iter2)
{
	InfTextUser* user1 = (*iter1)[m_columns.user];
	InfTextUser* user2 = (*iter2)[m_columns.user];

	bool available1 =
		inf_user_get_status(INF_USER(user1)) != INF_USER_UNAVAILABLE;
	bool available2 =
		inf_user_get_status(INF_USER(user2)) != INF_USER_UNAVAILABLE;

	if(available1 != available2)
	{
		if(!available1)
			return 1;
		return -1;
	}
	else
	{
		return std::strcmp(inf_user_get_name(INF_USER(user1)),
		                   inf_user_get_name(INF_USER(user2)));
	}
}

void Gobby::UserList::on_add_user(InfTextUser* user)
{
	g_assert(find_user_iter(user) == m_store->children().end());

	Glib::RefPtr<Gdk::Pixbuf> color_pixbuf =
		generate_user_color_pixbuf(inf_text_user_get_hue(user));

	Gtk::TreeIter iter = m_store->append();
	(*iter)[m_columns.user] = user;
	(*iter)[m_columns.color] = color_pixbuf;
	(*iter)[m_columns.notify_hue_handle] = g_signal_connect(
		G_OBJECT(user), "notify::hue",
		G_CALLBACK(on_notify_hue_static), this);
	(*iter)[m_columns.notify_status_handle] = g_signal_connect(
		G_OBJECT(user), "notify::status",
		G_CALLBACK(on_notify_status_static), this);
}

void Gobby::UserList::on_notify_hue(InfTextUser* user)
{
	Gtk::TreeIter iter = find_user_iter(user);
	g_assert(iter != m_store->children().end());

	(*iter)[m_columns.color] =
		generate_user_color_pixbuf(inf_text_user_get_hue(user));
}

void Gobby::UserList::on_notify_status(InfTextUser* user)
{
	Gtk::TreeIter iter = find_user_iter(user);
	g_assert(iter != m_store->children().end());

	// This does not cause a resort:
	//m_store->row_changed(m_store->get_path(iter), iter);

	// But this does:
	(*iter)[m_columns.user] = user;
}

Gtk::TreeIter Gobby::UserList::find_user_iter(InfTextUser* user)
{
	const Gtk::TreeModel::Children& children = m_store->children();
	for(Gtk::TreeIter iter = children.begin();
	    iter != children.end(); ++ iter)
	{
		if(user == (*iter)[m_columns.user])
			return iter;
	}

	return children.end();
}

