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
#include "documentlist.hpp"

namespace
{
	GdkColor COLOR_UNSUBSCRIBED_GDK = { 0, 0xaaaa, 0xaaaa, 0xaaaa };
	GdkColor COLOR_SUBSCRIBED_GDK = { 0, 0x0000, 0x0000, 0x0000 };

	Gdk::Color COLOR_UNSUBSCRIBED(&COLOR_UNSUBSCRIBED_GDK, true);
	Gdk::Color COLOR_SUBSCRIBED(&COLOR_SUBSCRIBED_GDK, true);
}

Gobby::DocumentList::Columns::Columns()
{
	add(icon);
	add(text);
	add(color);
	add(data);
}

Gobby::DocumentList::DocumentList(Gtk::Window& parent,
                                  Header& header,
                                  const Preferences& preferences,
				  Config::Entry& config_entry):
	ToggleWindow(
		parent,
		header.action_window_documentlist,
		preferences,
		config_entry["documentlist"]
	),
	m_btn_subscribe(_("Subscribe") ),
	m_header(header)
{
	m_tree_data = Gtk::TreeStore::create(m_tree_cols);

	m_view_col.pack_start(m_tree_cols.icon, false);
	m_view_col.pack_start(m_tree_cols.text, false);
	m_view_col.set_spacing(5);

	std::vector<Gtk::CellRenderer*> renderers =
		m_view_col.get_cell_renderers();
	Gtk::CellRendererText* renderer =
		dynamic_cast<Gtk::CellRendererText*>(renderers[1]);
	if(renderer == NULL)
		throw std::logic_error("Gobby::DocumentList::DocumentList");

	m_view_col.add_attribute(
		renderer->property_foreground_gdk(),
		m_tree_cols.color
	);

	m_view_col.set_sort_column(m_tree_cols.text);

	m_tree_view.add_events(Gdk::BUTTON_PRESS_MASK);
	m_tree_view.signal_button_press_event().connect(
		sigc::mem_fun(*this, &DocumentList::on_tree_button_press) );

	m_tree_view.set_model(m_tree_data);
	m_tree_view.append_column(m_view_col);
	m_tree_view.set_headers_visible(false);

	m_tree_view.get_selection()->set_mode(Gtk::SELECTION_MULTIPLE);
	m_tree_view.get_selection()->signal_changed().connect(
		sigc::mem_fun(*this, &DocumentList::on_selection_changed) );

	m_scrolled_wnd.set_shadow_type(Gtk::SHADOW_IN);
	m_scrolled_wnd.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	m_scrolled_wnd.add(m_tree_view);
	m_scrolled_wnd.set_sensitive(false);

	m_btn_subscribe.set_sensitive(false);
	m_btn_subscribe.signal_clicked().connect(
		sigc::mem_fun(*this, &DocumentList::on_subscribe) );

	m_mainbox.pack_start(m_scrolled_wnd, Gtk::PACK_EXPAND_WIDGET);
	m_mainbox.pack_start(m_btn_subscribe, Gtk::PACK_SHRINK);
	m_mainbox.set_spacing(10);

	add(m_mainbox);

	set_default_size(200, 400);
	set_title(_("Document list") );
	set_border_width(10);

	show_all_children();
}

void Gobby::DocumentList::obby_start(LocalBuffer& buf)
{
	m_scrolled_wnd.set_sensitive(true);
}

void Gobby::DocumentList::obby_end()
{
	m_scrolled_wnd.set_sensitive(false);
	m_tree_data->clear();
}

void Gobby::DocumentList::obby_user_join(const obby::user& user)
{
}

void Gobby::DocumentList::obby_user_part(const obby::user& user)
{
}

void Gobby::DocumentList::obby_user_colour(const obby::user& user)
{
}

void Gobby::DocumentList::obby_document_insert(LocalDocumentInfo& info)
{
	info.subscribe_event().connect(
		sigc::bind(
			sigc::mem_fun(*this, &DocumentList::on_user_subscribe),
			sigc::ref(info)
		)
	);

	info.unsubscribe_event().connect(
		sigc::bind(
			sigc::mem_fun(
				*this,
				&DocumentList::on_user_unsubscribe
			),
			sigc::ref(info)
		)
	);

	Gtk::TreeIter new_doc = m_tree_data->append();
	(*new_doc)[m_tree_cols.icon] = render_icon(
		Gtk::Stock::EDIT,
		Gtk::ICON_SIZE_BUTTON
	);

	(*new_doc)[m_tree_cols.text] = info.get_title();
	(*new_doc)[m_tree_cols.color] =
		info.is_subscribed() ? COLOR_SUBSCRIBED : COLOR_UNSUBSCRIBED;
	(*new_doc)[m_tree_cols.data] = static_cast<void*>(&info);
}

void Gobby::DocumentList::obby_document_remove(LocalDocumentInfo& info)
{
	Gtk::TreeIter iter = find_iter(info);
	if(iter == m_tree_data->children().end() )
	{
		throw std::logic_error(
			"Gobby::DocumentList:obby_document_remove"
		);
	}

	m_tree_data->erase(iter);
}

void
Gobby::DocumentList::on_user_subscribe(const obby::user& user,
                                       const LocalDocumentInfo& info)
{
	if(&user == &info.get_buffer().get_self() )
	{
		Gtk::TreeIter iter = find_iter(info);
		(*iter)[m_tree_cols.color] = COLOR_SUBSCRIBED;

		// Own subscription to a document has changed: Recheck
		// sensitivity of subscribe button
		on_selection_changed();
	}
}

void
Gobby::DocumentList::on_user_unsubscribe(const obby::user& user,
                                         const LocalDocumentInfo& info)
{
	if(&user == &info.get_buffer().get_self() )
	{
		Gtk::TreeIter iter = find_iter(info);
		(*iter)[m_tree_cols.color] = COLOR_UNSUBSCRIBED;

		// Own subscription to a document has changed: Recheck
		// sensitivity of subscribe button
		on_selection_changed();
	}
}

Gtk::TreeIter
Gobby::DocumentList::find_iter(const LocalDocumentInfo& info) const
{
	const Gtk::TreeNodeChildren& list = m_tree_data->children();
	for(Gtk::TreeIter iter = list.begin(); iter != list.end(); ++ iter)
	{
		if( (*iter)[m_tree_cols.data] ==
		   static_cast<const void*>(&info) )
		{
			return iter;
		}
	}

	return list.end();
}

void Gobby::DocumentList::on_subscribe()
{
	std::list<Gtk::TreePath> selected_entries =
		m_tree_view.get_selection()->get_selected_rows();

	for(std::list<Gtk::TreePath>::iterator iter = selected_entries.begin();
	    iter != selected_entries.end();
	    ++ iter)
	{
		Gtk::TreeIter tree_iter = m_tree_data->get_iter(*iter);
		LocalDocumentInfo* info =
			static_cast<LocalDocumentInfo*>(
				static_cast<void*>(
					(*tree_iter)[m_tree_cols.data]
				)
			);

		if(!info->is_subscribed() )
			info->subscribe();
	}
}

void Gobby::DocumentList::on_selection_changed()
{
	std::list<Gtk::TreePath> selected_entries =
		m_tree_view.get_selection()->get_selected_rows();
	for(std::list<Gtk::TreePath>::iterator iter = selected_entries.begin();
	    iter != selected_entries.end();
	    ++ iter)
	{
		Gtk::TreeIter tree_iter = m_tree_data->get_iter(*iter);
		const LocalDocumentInfo* info =
			static_cast<const LocalDocumentInfo*>(
				static_cast<const void*>(
					(*tree_iter)[m_tree_cols.data]
				)
			);

		if(!info->is_subscribed() )
		{
			m_btn_subscribe.set_sensitive(true);
			return;
		}
	}

	m_btn_subscribe.set_sensitive(false);
}

#include <iostream>
bool Gobby::DocumentList::on_tree_button_press(GdkEventButton* event)
{
	std::cout << event->button << std::endl;

	// Double-clicking on a entry is like clicking on the subscribe
	// button
	if(event->type == GDK_2BUTTON_PRESS && event->button == 1)
	{
		on_subscribe();
		return true;
	}

	return false;
}
