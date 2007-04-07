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

#include <stdexcept>
#include <gtkmm/stock.h>
#include "document_settings.hpp"
#include "window.hpp"

namespace
{
	GdkColor COLOR_UNSUBSCRIBED_GDK = { 0, 0xaaaa, 0xaaaa, 0xaaaa };
	GdkColor COLOR_SUBSCRIBED_GDK = { 0, 0x0000, 0x0000, 0x0000 };
	GdkColor COLOR_UNSUBSCRIBABLE_GDK = { 0, 0xaaaa, 0x0000, 0x0000 };

	Gdk::Color COLOR_UNSUBSCRIBED(&COLOR_UNSUBSCRIBED_GDK, true);
	Gdk::Color COLOR_SUBSCRIBED(&COLOR_SUBSCRIBED_GDK, true);
	Gdk::Color COLOR_UNSUBSCRIBABLE(&COLOR_UNSUBSCRIBABLE_GDK, true);

	Gdk::Color document_color(const Gobby::LocalDocumentInfo& info)
	{
		if(!Gobby::is_subscribable(info) )
			return COLOR_UNSUBSCRIBABLE;

		if(!info.is_subscribed() )
			return COLOR_UNSUBSCRIBED;

		return COLOR_SUBSCRIBED;
	}
}

Gobby::DocumentSettings::Columns::Columns()
{
	add(info);
	add(icon);
	add(color);
	add(title);
	add(original_encoding);
	add(path);
	add(auto_open);
}

Gobby::DocumentSettings::DocumentSettings(Window& window):
	m_icon(window.render_icon(Gtk::Stock::EDIT, Gtk::ICON_SIZE_BUTTON))
{
	m_data = Gtk::ListStore::create(columns);
}

void Gobby::DocumentSettings::obby_start(LocalBuffer& buf)
{
	m_data->clear();

	for(LocalBuffer::document_iterator iter = buf.document_begin();
	    iter != buf.document_end();
	    ++ iter)
	{
		on_document_insert(dynamic_cast<LocalDocumentInfo&>(*iter) );
	}

	buf.document_insert_event().connect(
		sigc::mem_fun(*this, &DocumentSettings::on_document_insert) );
	buf.document_remove_event().connect(
		sigc::mem_fun(*this, &DocumentSettings::on_document_remove) );
}

void Gobby::DocumentSettings::obby_end()
{
	// Do not clear the list, one might want to save documents when
	// the session is closed and so needs access to the save path.
}

Glib::ustring Gobby::DocumentSettings::
	get_original_encoding(const LocalDocumentInfo& info) const
{
	return (*get_iter(info))[columns.original_encoding];
}

void Gobby::DocumentSettings::
	set_original_encoding(const LocalDocumentInfo& info,
	                      const Glib::ustring& encoding)
{
	(*get_iter(info))[columns.original_encoding] = encoding;
}

Glib::ustring Gobby::DocumentSettings::
	get_path(const LocalDocumentInfo& info) const
{
	return (*get_iter(info))[columns.path];
}

void Gobby::DocumentSettings::set_path(const LocalDocumentInfo& info,
                                       const Glib::ustring& path)
{
	(*get_iter(info))[columns.path] = path;
}

bool Gobby::DocumentSettings::
	get_automatically_opened(const LocalDocumentInfo& info) const
{
	return (*get_iter(info))[columns.auto_open];
}

void Gobby::DocumentSettings::set_automatically_opened(
	const LocalDocumentInfo& info, bool value)
{
	(*get_iter(info))[columns.auto_open] = value;
}

/*Gobby::DocumentSettings::Columns&
Gobby::DocumentSettings::get_golumns()
{
	return m_columns;
}

const Gobby::DocumentSettings::Columns&
Gobby::DocumentSettings::get_columns() const
{
	return columns;
}*/

Glib::RefPtr<Gtk::ListStore> Gobby::DocumentSettings::get_list()
{
	return m_data;
}

Glib::RefPtr<const Gtk::ListStore> Gobby::DocumentSettings::get_list() const
{
	return m_data;
}

Gobby::DocumentSettings::signal_document_insert_type
Gobby::DocumentSettings::document_insert_event() const
{
	return m_signal_document_insert;
}

Gobby::DocumentSettings::signal_document_remove_type
Gobby::DocumentSettings::document_remove_event() const
{
	return m_signal_document_remove;
}

void Gobby::DocumentSettings::on_document_insert(DocumentInfo& info)
{
	LocalDocumentInfo& local_info = dynamic_cast<LocalDocumentInfo&>(info);

	local_info.rename_event().connect(
		sigc::hide(
			sigc::bind(
				sigc::mem_fun(
					*this,
					&DocumentSettings::on_document_rename
				),
				sigc::ref(local_info)
			)
		)
	);

	local_info.subscribe_event().connect(
		sigc::bind(
			sigc::mem_fun(
				*this,
				&DocumentSettings::on_subscribe
			),
			sigc::ref(local_info)
		)
	);

	local_info.unsubscribe_event().connect(
		sigc::bind(
			sigc::mem_fun(
				*this,
				&DocumentSettings::on_unsubscribe
			),
			sigc::ref(local_info)
		)
	);

	Gtk::TreeIter iter = m_data->append();
	(*iter)[columns.info] = &local_info;
	(*iter)[columns.icon] = m_icon;
	(*iter)[columns.color] = document_color(local_info);
	(*iter)[columns.title] = local_info.get_suffixed_title();
	(*iter)[columns.auto_open] = false;

	m_map[&local_info] = iter;

	m_signal_document_insert.emit(local_info);
}

void Gobby::DocumentSettings::on_document_remove(DocumentInfo& info)
{
	LocalDocumentInfo& local_info = dynamic_cast<LocalDocumentInfo&>(info);

	map_type::iterator iter = m_map.find(&local_info);
	if(iter == m_map.end() )
	{
		throw std::logic_error(
			"Gobby::DocumentSettings::on_document_remove:\n"
			"Document info not found in iterator map"
		);
	}

	m_signal_document_remove.emit(local_info);

	m_data->erase(iter->second);
	m_map.erase(iter);
}

void Gobby::DocumentSettings::on_document_rename(LocalDocumentInfo& info)
{
	(*get_iter(info))[columns.title] = info.get_suffixed_title();
}

void Gobby::DocumentSettings::on_subscribe(const obby::user& user,
                                           LocalDocumentInfo& info)
{
	if(&user == &info.get_buffer().get_self() )
		(*get_iter(info))[columns.color] = document_color(info);
}

void Gobby::DocumentSettings::on_unsubscribe(const obby::user& user,
                                             LocalDocumentInfo& info)
{
	if(&user == &info.get_buffer().get_self() )
		(*get_iter(info))[columns.color] = document_color(info);
}

Gtk::TreeIter Gobby::DocumentSettings::
	get_iter(const LocalDocumentInfo& info) const
{
	map_type::const_iterator iter = m_map.find(&info);
	if(iter == m_map.end() )
	{
		throw std::logic_error(
			"Gobby::DocumentSettings::get_iter:\n"
			"Document info not found in iterator map"
		);
	}

	return iter->second;
}
