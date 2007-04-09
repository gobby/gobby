/* gobby - A GTKmm driven libobby client
 * Copyright (C) 2005 0x539 dev group
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

#include <gtkmm/stock.h>
#include "common.hpp"
#include "userlist.hpp"

namespace
{
	/** Creates a pixbuf representing a user's colour.
	 */
	Glib::RefPtr<Gdk::Pixbuf>
	create_coloured_pixbuf(const obby::colour& colour)
	{
		Glib::RefPtr<Gdk::Pixbuf> pixbuf =
			Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, false, 8, 16,
				16);
		pixbuf->fill(
			(colour.get_red() << 24) |
		       	(colour.get_green() << 16) |
		       	(colour.get_blue() << 8)
			);

		// Border around the color
		guint8* pixels = pixbuf->get_pixels();
		for(unsigned int y = 0; y < 16; ++ y)
		{
			for(unsigned int x = 0; x < 16; ++ x)
			{
				if(x == 0 || y == 0 || x == 15 || y == 15)
				{
					pixels[0] = 0;
					pixels[1] = 0;
					pixels[2] = 0;
				}

				pixels += 3;
			}
		}

		return pixbuf;
	}
}


Gobby::UserList::Columns::Columns()
{
	add(icon);
	add(text);
	add(info);
}

Gobby::UserList::UserList(Gtk::Window& parent,
                          Header& header,
                          Folder& folder,
                          const Preferences& m_preferences,
			  Config::ParentEntry& config_entry):
	ToggleWindow(
		parent,
		header.action_window_userlist,
		m_preferences,
		config_entry["userlist"]
	),
	m_header(header),
	m_folder(folder)
{
	m_tree_data = Gtk::TreeStore::create(m_tree_cols);

	m_view_col.pack_start(m_tree_cols.icon, false);
	m_view_col.pack_start(m_tree_cols.text, false);
	m_view_col.set_spacing(5);

	m_tree_view.set_model(m_tree_data);
	m_tree_view.append_column(m_view_col);

	m_iter_online = m_tree_data->append();
	m_iter_offline = m_tree_data->append();

	(*m_iter_online)[m_tree_cols.text] = _("Online");
	(*m_iter_offline)[m_tree_cols.text] = _("Offline");

	(*m_iter_online)[m_tree_cols.icon] = render_icon(
		Gtk::Stock::CONNECT,
		Gtk::ICON_SIZE_BUTTON
	);

	(*m_iter_offline)[m_tree_cols.icon] = render_icon(
		Gtk::Stock::DISCONNECT,
		Gtk::ICON_SIZE_BUTTON
	);

	m_view_col.set_sort_column(m_tree_cols.text);

	m_tree_view.get_selection()->set_mode(Gtk::SELECTION_NONE);
	m_tree_view.set_headers_visible(false);
	m_tree_view.signal_row_activated().connect(
		sigc::mem_fun(*this, &UserList::on_row_activated) );

	m_scrolled_wnd.set_shadow_type(Gtk::SHADOW_IN);
	m_scrolled_wnd.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	m_scrolled_wnd.add(m_tree_view);
	m_scrolled_wnd.set_sensitive(false);

	add(m_scrolled_wnd);

	set_default_size(200, 400);
	set_title(_("User list") );
	set_border_width(10);

	show_all_children();
}

void Gobby::UserList::obby_start(LocalBuffer& buf)
{
	remove_children(m_iter_offline);
	remove_children(m_iter_online);

	m_scrolled_wnd.set_sensitive(true);
	m_buffer = &buf;
}

void Gobby::UserList::obby_end()
{
}

void Gobby::UserList::obby_user_join(const obby::user& user)
{
	// Verify that the user is not already joined
	if(find_iter(m_iter_online, user.get_name()) !=
	   m_iter_online->children().end() )
	{
		throw std::logic_error("Gobby::UserList::obby_user_join");
	}

	// Find user in offline list
	Gtk::TreeIter iter = find_iter(m_iter_offline, user.get_name() );
	if(iter != m_iter_offline->children().end() )
	{
		// Remove it, if the new user is connected
		if(user.get_flags() & obby::user::flags::CONNECTED)
			m_tree_data->erase(iter);
		else
			// Let the entry in the offline list if the new user is
			// not connected.
			return;
	}

	// Add it to correct list
	if(user.get_flags() & obby::user::flags::CONNECTED)
		iter = m_tree_data->append(m_iter_online->children());
	else
		iter = m_tree_data->append(m_iter_offline->children());

	(*iter)[m_tree_cols.icon] = create_coloured_pixbuf(user.get_colour() );
	(*iter)[m_tree_cols.text] = user.get_name();

	// New user may already be subscribed to documents (client initial)
	for(Buffer::document_iterator iter = m_buffer->document_begin();
	    iter != m_buffer->document_end();
	    ++ iter)
	{
		if(iter->is_subscribed(user) )
		{
			on_user_subscribe(
				user,
				dynamic_cast<LocalDocumentInfo&>(
					*iter
				)
			);
		}
	}

	if(&user == &m_buffer->get_self() )
	{
		// Open the "Online" node
		Gtk::TreePath online(*m_iter_online);
		m_tree_view.expand_row(online, false);
	}
}

void Gobby::UserList::obby_user_part(const obby::user& user)
{
	// Find user in online list
	Gtk::TreeIter iter = find_iter(m_iter_online, user.get_name() );
	if(iter == m_iter_online->children().end() )
		throw std::logic_error("Gobby::UserList::obby_user_part");

	// Remove it from there
	m_tree_data->erase(iter);

	// Insert into offline list
	iter = m_tree_data->append(m_iter_offline->children() );
	(*iter)[m_tree_cols.icon] = create_coloured_pixbuf(user.get_colour() );
	(*iter)[m_tree_cols.text] = user.get_name();
}

void Gobby::UserList::obby_user_colour(const obby::user& user)
{
	// Find user in list
	Gtk::TreeIter iter = find_iter(m_iter_online, user.get_name() );
	if(iter == m_iter_online->children().end() )
		throw std::logic_error("Gobby::UserList::obby_user_colour");

	// Recolour
	(*iter)[m_tree_cols.icon] = create_coloured_pixbuf(user.get_colour() );
}

void Gobby::UserList::obby_document_insert(LocalDocumentInfo& info)
{
	info.subscribe_event().connect(
		sigc::bind(
			sigc::mem_fun(*this, &UserList::on_user_subscribe),
			sigc::ref(info)
		)
	);

	info.unsubscribe_event().connect(
		sigc::bind(
			sigc::mem_fun(*this, &UserList::on_user_unsubscribe),
			sigc::ref(info)
		)
	);
}

void Gobby::UserList::obby_document_remove(LocalDocumentInfo& info)
{
	// Do nothing here because unsubscrption signal will be emitted for all
	// users?
}

void Gobby::UserList::on_user_subscribe(const obby::user& user,
                                        LocalDocumentInfo& info)
{
	Gtk::TreeIter iter = find_iter(m_iter_online, user.get_name() );
	if(iter == m_iter_online->children().end() )
		throw std::logic_error("Gobby::UserList::on_user_subscribe");

	Gtk::TreeIter doc = m_tree_data->append(iter->children() );
	(*doc)[m_tree_cols.icon] = render_icon(
		Gtk::Stock::EDIT,
		Gtk::ICON_SIZE_BUTTON
	);

	(*doc)[m_tree_cols.text] = info.get_title();

	(*doc)[m_tree_cols.info] = &info;
}

void Gobby::UserList::on_user_unsubscribe(const obby::user& user,
                                          const LocalDocumentInfo& info)
{
	Gtk::TreeIter user_iter = find_iter(m_iter_online, user.get_name() );
	if(user_iter == m_iter_online->children().end() )
		throw std::logic_error("Gobby::UserList::on_user_unsubscribe");

	Gtk::TreeIter doc_iter = find_iter(user_iter, info.get_title() );
	if(doc_iter == user_iter->children().end() )
		throw std::logic_error("Gobby::UserList::on_user_unsubscribe");

	m_tree_data->erase(doc_iter);
}

Gtk::TreeIter Gobby::UserList::find_iter(const Gtk::TreeIter& parent,
                                         const Glib::ustring& text) const
{
	const Gtk::TreeNodeChildren& children = parent->children();
	for(Gtk::TreeIter i = children.begin(); i != children.end(); ++ i)
		if( (*i)[m_tree_cols.text] == text)
			return i;

	return children.end();
}

void Gobby::UserList::remove_children(const Gtk::TreeIter& parent)
{
	const Gtk::TreeNodeChildren& list = parent->children();
	Gtk::TreeIter iter = list.begin();

	while(iter != list.end() )
		iter = m_tree_data->erase(iter);
}

void Gobby::UserList::on_row_activated(const Gtk::TreePath& path,
                                       Gtk::TreeViewColumn* columns)
{
	Gtk::TreeIter tree_iter = m_tree_data->get_iter(path);
	LocalDocumentInfo* info = (*tree_iter)[m_tree_cols.info];

	if(info == NULL)
		// The selected row is a user, not a document.
		return;

	if(Gobby::is_subscribable(*info) &&
	   info->get_subscription_state() == LocalDocumentInfo::UNSUBSCRIBED)
	{
		info->subscribe();
	}
	else if(info->get_subscription_state() == LocalDocumentInfo::SUBSCRIBED)
	{
		m_folder.select_document(*info);
	}
}

