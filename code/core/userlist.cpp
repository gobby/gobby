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

#include "core/userlist.hpp"
#include "core/iconmanager.hpp"

#include "util/i18n.hpp"
#include "util/color.hpp"

#include <gtkmm/scrolledwindow.h>
#include <gtkmm/stock.h>

#include <cstring>

namespace
{
	typedef sigc::slot<void, InfUser*> ForeachUserFunc;

	void foreach_user_ctor_func(InfUser* user, gpointer user_data)
	{
		(*static_cast<ForeachUserFunc*>(user_data))(user);
	}

	Glib::RefPtr<Gdk::Pixbuf> generate_user_color_pixbuf(Gtk::Widget& w,
	                                                     gdouble hue)
	{
		Gtk::StockID id =
			Gobby::IconManager::STOCK_USER_COLOR_INDICATOR;

		Glib::RefPtr<Gdk::Pixbuf> pixbuf =
			w.render_icon_pixbuf(id, Gtk::ICON_SIZE_MENU);

		if(!pixbuf) // icon not found
		{
			pixbuf = w.render_icon_pixbuf(
				Gtk::Stock::MISSING_IMAGE,
				Gtk::ICON_SIZE_MENU);
		}

		// pixbuf is shared, though we want to mess with it here
		pixbuf = pixbuf->copy();

		for(int y = 0; y < pixbuf->get_height(); ++y)
		{
			for(int x = 0; x < pixbuf->get_width(); ++x)
			{
				guint8* pixels = pixbuf->get_pixels();
				guint8* pixel =
					pixels + y * pixbuf->get_rowstride() +
					         x * pixbuf->get_n_channels();

				double r = pixel[0]/255.0;
				double g = pixel[1]/255.0;
				double b = pixel[2]/255.0;

				Gobby::rgb_to_hsv(r,g,b);
				r = hue;
				Gobby::hsv_to_rgb(r,g,b);

				pixel[0] =
					static_cast<guint8>(r * 255.0 + 0.5);
				pixel[1] =
					static_cast<guint8>(g * 255.0 + 0.5);
				pixel[2] =
					static_cast<guint8>(b * 255.0 + 0.5);
			}
		}

		return pixbuf;
	}
}

Gobby::UserList::UserList(InfUserTable* table):
	m_table(table), m_store(Gtk::ListStore::create(m_columns)),
	m_view(m_store)
{
	m_store->set_sort_func(m_columns.user,
	                       sigc::mem_fun(*this, &UserList::sort_func));

	m_store->set_sort_column(m_columns.user, Gtk::SORT_ASCENDING);

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

	m_view.signal_row_activated().connect(
		sigc::mem_fun(*this, &UserList::on_row_activated));

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
	g_signal_handler_disconnect(G_OBJECT(m_table), m_add_user_handle);

	m_filter_model.reset();

	const Gtk::TreeModel::Children& children = m_store->children();
	for(Gtk::TreeIter iter = children.begin();
	    iter != children.end(); ++ iter)
	{
		InfUser* user = (*iter)[m_columns.user];
		gulong notify_hue_handle =
			(*iter)[m_columns.notify_hue_handle];
		gulong notify_status_handle =
			(*iter)[m_columns.notify_status_handle];

		if(notify_hue_handle > 0)
			g_signal_handler_disconnect(G_OBJECT(user),
			                            notify_hue_handle);
		g_signal_handler_disconnect(G_OBJECT(user),
		                            notify_status_handle);
	}
}

void Gobby::UserList::set_show_disconnected(bool show_disconnected)
{
	if(show_disconnected)
	{
		m_filter_model.reset();
		m_view.set_model(m_store);
	}
	else
	{
		m_filter_model = Gtk::TreeModelFilter::create(m_store);
		m_view.set_model(m_filter_model);

		m_filter_model->set_visible_func(
			sigc::mem_fun(*this, &UserList::visible_func));
	}
}

bool Gobby::UserList::visible_func(const Gtk::TreeIter& iter)
{
	InfUser* user = (*iter)[m_columns.user];

	// Can happen after creation of the node when the user object has
	// not yet been set
	if(user == NULL) return false;
	return inf_user_get_status(user) != INF_USER_UNAVAILABLE;
}

void Gobby::UserList::icon_cell_data_func(Gtk::CellRenderer* renderer,
                                          const Gtk::TreeIter& iter)
{
	Gtk::CellRendererPixbuf* pixbuf_renderer =
		dynamic_cast<Gtk::CellRendererPixbuf*>(renderer);
	g_assert(pixbuf_renderer);

	pixbuf_renderer->property_stock_size() = Gtk::ICON_SIZE_MENU;

	InfUser* user = (*iter)[m_columns.user];

	if(user == NULL)
	{
		// Can happen after creation of the node when the user
		// object has not yet been set
		pixbuf_renderer->property_visible() = false;
	}
	else
	{
		pixbuf_renderer->property_visible() = true;
		switch(inf_user_get_status(user))
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
}

void Gobby::UserList::color_cell_data_func(Gtk::CellRenderer* renderer,
                                           const Gtk::TreeIter& iter)
{
	Gtk::CellRendererPixbuf* pixbuf_renderer =
		dynamic_cast<Gtk::CellRendererPixbuf*>(renderer);
	g_assert(pixbuf_renderer);

	Glib::RefPtr<Gdk::Pixbuf> pixbuf = (*iter)[m_columns.color];
	if(pixbuf)
	{
		pixbuf_renderer->property_pixbuf() = pixbuf;
		pixbuf_renderer->property_visible() = true;
	}
	else
	{
		pixbuf_renderer->property_visible() = false;
	}
}

void Gobby::UserList::name_cell_data_func(Gtk::CellRenderer* renderer,
                                          const Gtk::TreeIter& iter)
{
	Gtk::CellRendererText* text_renderer =
		dynamic_cast<Gtk::CellRendererText*>(renderer);
	g_assert(text_renderer);

	InfUser* user = (*iter)[m_columns.user];
	if(user == NULL)
	{
		// Can happen after creation of the node when the user
		// object has not yet been set
		text_renderer->property_visible() = false;
	}
	else
	{
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

		text_renderer->property_visible() = true;
		text_renderer->property_text() = inf_user_get_name(user);
	}
}

int Gobby::UserList::sort_func(const Gtk::TreeIter& iter1,
                               const Gtk::TreeIter& iter2)
{
	InfUser* user1 = (*iter1)[m_columns.user];
	InfUser* user2 = (*iter2)[m_columns.user];

	bool available1 = inf_user_get_status(user1) != INF_USER_UNAVAILABLE;
	bool available2 = inf_user_get_status(user2) != INF_USER_UNAVAILABLE;

	if(available1 != available2)
	{
		if(!available1)
			return 1;
		return -1;
	}
	else
	{
		// We might want to cache collate keys in the ListStore if
		// this turns out to be a performance problem:
		return g_utf8_collate(inf_user_get_name(user1),
		                      inf_user_get_name(user2));
	}
}

void Gobby::UserList::on_add_user(InfUser* user)
{
	g_assert(find_user_iter(user) == m_store->children().end());

	Gtk::TreeIter iter = m_store->append();
	(*iter)[m_columns.user] = user;
	(*iter)[m_columns.notify_status_handle] = g_signal_connect(
		G_OBJECT(user), "notify::status",
		G_CALLBACK(on_notify_status_static), this);

	if(INF_TEXT_IS_USER(user))
	{
		Glib::RefPtr<Gdk::Pixbuf> color_pixbuf =
			generate_user_color_pixbuf(
				*this,
				inf_text_user_get_hue(INF_TEXT_USER(user)));

		(*iter)[m_columns.color] = color_pixbuf;
		(*iter)[m_columns.notify_hue_handle] = g_signal_connect(
			G_OBJECT(user), "notify::hue",
			G_CALLBACK(on_notify_hue_static), this);
	}
	else
	{
		// Should be 0 anyway, but let's be sure:
		(*iter)[m_columns.notify_hue_handle] = 0;
	}
}

void Gobby::UserList::on_notify_hue(InfTextUser* user)
{
	Gtk::TreeIter iter = find_user_iter(INF_USER(user));
	g_assert(iter != m_store->children().end());

	(*iter)[m_columns.color] = generate_user_color_pixbuf(
		*this, inf_text_user_get_hue(user));
}

void Gobby::UserList::on_notify_status(InfUser* user)
{
	Gtk::TreeIter iter = find_user_iter(user);
	g_assert(iter != m_store->children().end());

	// This does not cause a resort:
	//m_store->row_changed(m_store->get_path(iter), iter);

	// But this does:
	(*iter)[m_columns.user] = user;
}

void Gobby::UserList::on_row_activated(const Gtk::TreePath& path,
                                       Gtk::TreeViewColumn* column)
{
	Gtk::TreePath parent_path;
	if(m_filter_model)
		parent_path = m_filter_model->convert_path_to_child_path(path);
	else
		parent_path = path;

	const Gtk::TreeIter& iter = m_store->get_iter(path);
	InfUser* user = (*iter)[m_columns.user];

	if(inf_user_get_status(user) != INF_USER_UNAVAILABLE)
		m_signal_user_activated.emit(user);
}

Gtk::TreeIter Gobby::UserList::find_user_iter(InfUser* user)
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
