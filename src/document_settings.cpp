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

	Gdk::Color COLOR_UNSUBSCRIBED(&COLOR_UNSUBSCRIBED_GDK, true);
	Gdk::Color COLOR_SUBSCRIBED(&COLOR_SUBSCRIBED_GDK, true);
}

Gobby::DocumentSettings::Columns::Columns()
{
	add(info);
	add(icon);
	add(color);
	add(title);
	add(original_encoding);
	add(path);
}

Gobby::DocumentSettings::DocumentSettings(Window& window):
	m_icon(window.render_icon(Gtk::Stock::EDIT, Gtk::ICON_SIZE_BUTTON))
{
	m_data = Gtk::ListStore::create(m_cols);
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
	buf.document_rename_event().connect(
		sigc::mem_fun(*this, &DocumentSettings::on_document_rename) );
}

void Gobby::DocumentSettings::obby_end()
{
	// Do not clear the list, one might want to save documents when
	// the session is closed and so needs access to the save path.
	//
	// Note that this does not work until obby keeps the document info
	// after the session has been closed!
}

Glib::ustring Gobby::DocumentSettings::
	get_original_encoding(const LocalDocumentInfo& info) const
{
	return (*get_iter(info))[m_cols.original_encoding];
}

void Gobby::DocumentSettings::
	set_original_encoding(const LocalDocumentInfo& info,
	                      const Glib::ustring& encoding)
{
	(*get_iter(info))[m_cols.original_encoding] = encoding;
}

Glib::ustring Gobby::DocumentSettings::
	get_path(const LocalDocumentInfo& info) const
{
	return (*get_iter(info))[m_cols.path];
}

void Gobby::DocumentSettings::set_path(const LocalDocumentInfo& info,
                                       const Glib::ustring& path)
{
	(*get_iter(info))[m_cols.path] = path;
}

Glib::RefPtr<Gtk::ListStore> Gobby::DocumentSettings::get_list()
{
	return m_data;
}

Glib::RefPtr<const Gtk::ListStore> Gobby::DocumentSettings::get_list() const
{
	return m_data;
}

void Gobby::DocumentSettings::on_document_insert(DocumentInfo& info)
{
	LocalDocumentInfo& local_info = dynamic_cast<LocalDocumentInfo&>(info);

	Gtk::TreeIter iter = m_data->append();
	(*iter)[m_cols.info] = &local_info;
	(*iter)[m_cols.icon] = m_icon;
	(*iter)[m_cols.color] = (local_info.is_subscribed() ?
		COLOR_SUBSCRIBED : COLOR_UNSUBSCRIBED);
	(*iter)[m_cols.title] = local_info.get_title();
	(*iter)[m_cols.original_encoding] = "UTF-8";

	m_map[&local_info] = iter;
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

	m_data->erase(iter->second);
	m_map.erase(iter);
}

void Gobby::DocumentSettings::on_document_rename(DocumentInfo& info)
{
	LocalDocumentInfo& local_info = dynamic_cast<LocalDocumentInfo&>(info);
	(*get_iter(local_info))[m_cols.title] = local_info.get_title();
}

void Gobby::DocumentSettings::on_subscribe(const obby::user& user,
                                           LocalDocumentInfo& info)
{
	if(&user == &info.get_buffer().get_self() )
		(*get_iter(info))[m_cols.color] = COLOR_SUBSCRIBED;
}

void Gobby::DocumentSettings::on_unsubscribe(const obby::user& user,
                                             LocalDocumentInfo& info)
{
	if(&user == &info.get_buffer().get_self() )
		(*get_iter(info))[m_cols.color] = COLOR_UNSUBSCRIBED;
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
