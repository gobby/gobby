/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008, 2009 Armin Burgmeier <armin@arbur.net>
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

class OperationNew;
class OperationOpen;
class OperationOpenMultiple;
class OperationSave;
class OperationDelete;
class OperationExportHtml;

class Operations: public sigc::trackable
{
public:
	class Operation
	{
	public:
		typedef sigc::signal<void, bool> SignalFinished;

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

		SignalFinished signal_finished() const
		{
			return m_signal_finished;
		}

	protected:
		void fail()
		{
			m_operations.fail_operation(this);
		}

		void finish()
		{
			m_operations.finish_operation(this);
		}

		Operations& m_operations;

	private:
		SignalFinished m_signal_finished;
	};

	typedef sigc::signal<void, OperationSave*> SignalBeginSaveOperation;

	Operations(DocumentInfoStorage& info_storage, StatusBar& status_bar);
	~Operations();

	OperationNew* create_directory(InfcBrowser* browser,
	                               const InfcBrowserIter* parent,
	                               const Glib::ustring& name);

	OperationNew* create_document(InfcBrowser* browser,
	                              const InfcBrowserIter* parent,
	                              const Glib::ustring& name);

	OperationOpen* create_document(InfcBrowser* browser,
	                               const InfcBrowserIter* parent,
	                               const Glib::ustring& name,
	                               const Preferences& preferences,
	                               const Glib::ustring& from_uri,
	                               const char* encoding);

	OperationOpenMultiple* create_documents(InfcBrowser* browser,
	                                        const InfcBrowserIter* parent,
	                                        const Preferences& prefs,
	                                        unsigned int num_uris);

	OperationSave* save_document(DocWindow& document,
	                             Folder& folder,
	                             const std::string& uri,
	                             const std::string& encoding,
	                             DocumentInfoStorage::EolStyle eol_style);

	OperationDelete* delete_node(InfcBrowser* browser,
	                             const InfcBrowserIter* iter);

	OperationExportHtml* export_html(DocWindow& document,
	                                 const std::string& uri);

	OperationSave* get_save_operation_for_document(DocWindow& window);

	SignalBeginSaveOperation signal_begin_save_operation() const
	{
		return m_signal_begin_save_operation;
	}

protected:
	void fail_operation(Operation* operation);
	void finish_operation(Operation* operation);

	DocumentInfoStorage& m_info_storage;
	StatusBar& m_status_bar;

	typedef std::set<Operation*> OperationSet;
	OperationSet m_operations;

	SignalBeginSaveOperation m_signal_begin_save_operation;
};

}
	
#endif // _GOBBY_OPERATIONS_OPERATIONS_HPP_
