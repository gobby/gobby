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
	inline bool can_subscribe(const Gobby::LocalDocumentInfo& info)
	{
		Gobby::LocalDocumentInfo::subscription_state state =
			info.get_subscription_state();

		return (Gobby::is_subscribable(info) &&
		        state == Gobby::LocalDocumentInfo::UNSUBSCRIBED);
	}
}

Gobby::DocumentList::DocumentList(Gtk::Window& parent,
                                  DocumentSettings& settings,
                                  Header& header,
                                  const Preferences& preferences,
				  Config::ParentEntry& config_entry):
	ToggleWindow(
		parent,
		header.action_window_documentlist,
		preferences,
		config_entry["documentlist"]
	),
	m_buffer(NULL),
	m_settings(settings),
	m_btn_subscribe(_("Subscribe") )
{
	m_view_col.pack_start(settings.columns.icon, false);
	m_view_col.pack_start(settings.columns.title, false);
	m_view_col.set_spacing(5);

	std::vector<Gtk::CellRenderer*> renderers =
		m_view_col.get_cell_renderers();

	Gtk::CellRendererText* renderer =
		dynamic_cast<Gtk::CellRendererText*>(renderers[1]);

	if(renderer == NULL)
	{
		throw std::logic_error(
			"Gobby::DocumentList::DocumentList:\n"
			"Second cellrenderer is not of type CellRendererText"
		);
	}

	m_view_col.add_attribute(
		renderer->property_foreground_gdk(),
		settings.columns.color
	);

	m_view_col.set_sort_column(settings.columns.title);

	m_tree_view.add_events(Gdk::BUTTON_PRESS_MASK);
	m_tree_view.signal_button_press_event().connect(
		sigc::mem_fun(*this, &DocumentList::on_tree_button_press) );

	m_tree_view.set_model(settings.get_list() );
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
	// Clear old data
	m_scrolled_wnd.set_sensitive(true);
	m_buffer = &buf;
}

void Gobby::DocumentList::obby_end()
{
	// Subscription is no more possible
	m_btn_subscribe.set_sensitive(false);
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
		sigc::mem_fun(
			*this,
			&DocumentList::on_user_subscribe
		)
	);

	info.unsubscribe_event().connect(
		sigc::mem_fun(
			*this,
			&DocumentList::on_user_unsubscribe
		)
	);
}

void Gobby::DocumentList::obby_document_remove(LocalDocumentInfo& info)
{
}

void Gobby::DocumentList::on_user_subscribe(const obby::user& user)
{
	if(&user == &m_buffer->get_self() )
		on_selection_changed();
}

void Gobby::DocumentList::on_user_unsubscribe(const obby::user& user)
{
	if(&user == &m_buffer->get_self() )
		on_selection_changed();
}

void Gobby::DocumentList::on_subscribe()
{
	std::list<Gtk::TreePath> selected_entries =
		m_tree_view.get_selection()->get_selected_rows();

	for(std::list<Gtk::TreePath>::iterator iter = selected_entries.begin();
	    iter != selected_entries.end();
	    ++ iter)
	{
		Gtk::TreeIter tree_iter =
			m_settings.get_list()->get_iter(*iter);

		LocalDocumentInfo* info =
			(*tree_iter)[m_settings.columns.info];

		if(can_subscribe(*info) )
			info->subscribe();
	}
}

void Gobby::DocumentList::on_selection_changed()
{
	// Cannot subscribe when session is closed
	if(m_buffer == NULL || !m_buffer->is_open() )
		return;

	std::list<Gtk::TreePath> selected_entries =
		m_tree_view.get_selection()->get_selected_rows();

	for(std::list<Gtk::TreePath>::iterator iter = selected_entries.begin();
	    iter != selected_entries.end();
	    ++ iter)
	{
		Gtk::TreeIter tree_iter =
			m_settings.get_list()->get_iter(*iter);

		LocalDocumentInfo* info =
			(*tree_iter)[m_settings.columns.info];

		if(can_subscribe(*info) )
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
