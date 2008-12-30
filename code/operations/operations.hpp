/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008 Armin Burgmeier <armin@arbur.net>
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

#ifndef _GOBBY_OPERATIONS_OPERATIONS_HPP_
#define _GOBBY_OPERATIONS_OPERATIONS_HPP_

#include "core/documentinfostorage.hpp"
#include "core/statusbar.hpp"

#include <libinfinity/client/infc-browser.h>

#include <gtkmm/window.h>
#include <sigc++/trackable.h>

#include <set>

namespace Gobby
{

class Operations: public sigc::trackable
{
public:
	class Operation
	{
	public:
		typedef sigc::signal<void> SignalFinished;

		Operation(Operations& operations):
			m_operations(operations) {}
		virtual ~Operation() = 0;

		StatusBar& get_status_bar()
		{
			return m_operations.m_status_bar;
		}

		DocumentInfoStorage& get_info_storage()
		{
			return m_operations.m_info_storage;
		}

		void remove()
		{
			m_signal_finished.emit();
			m_operations.remove_operation(this);
		}

		SignalFinished signal_finished() const
		{
			return m_signal_finished;
		}
	private:
		SignalFinished m_signal_finished;
		Operations& m_operations;
	};

	Operations(DocumentInfoStorage& info_storage, StatusBar& status_bar);
	~Operations();

	Operation* create_directory(InfcBrowser* browser,
	                            InfcBrowserIter* parent,
	                            const Glib::ustring& name);

	Operation* create_document(InfcBrowser* browser,
	                           InfcBrowserIter* parent,
	                           const Glib::ustring& name);

	Operation* create_document(InfcBrowser* browser,
	                           InfcBrowserIter* parent,
	                           const Glib::ustring& name,
	                           const Preferences& preferences,
	                           const Glib::ustring& from_uri,
	                           const char* encoding);

	Operation* save_document(DocWindow& document,
	                         Folder& folder,
	                         const std::string& uri,
	                         const std::string& encoding,
	                         DocumentInfoStorage::EolStyle eol_style);

	Operation* delete_node(InfcBrowser* browser,
	                       InfcBrowserIter* iter);

protected:
	void remove_operation(Operation* operation);

	DocumentInfoStorage& m_info_storage;
	StatusBar& m_status_bar;

	typedef std::set<Operation*> OperationSet;
	OperationSet m_operations;
};

}
	
#endif // _GOBBY_OPERATIONS_OPERATIONS_HPP_
