/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2014 Armin Burgmeier <armin@arbur.net>
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
 * Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#ifndef _GOBBY_OPERATIONS_OPERATIONS_HPP_
#define _GOBBY_OPERATIONS_OPERATIONS_HPP_

#include "core/browser.hpp"
#include "core/statusbar.hpp"
#include "core/documentinfostorage.hpp"
#include "core/foldermanager.hpp"
#include "core/textsessionview.hpp"

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
class OperationRename;
class OperationDelete;
class OperationSubscribePath;
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

		virtual void start() = 0;

		StatusBar& get_status_bar()
		{
			return m_operations.m_status_bar;
		}

		Browser& get_browser()
		{
			return m_operations.m_browser;
		}

		FolderManager& get_folder_manager()
		{
			return m_operations.m_folder_manager;
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
	typedef std::vector<Glib::ustring> uri_list;

	Operations(DocumentInfoStorage& info_storage,
	           Browser& browser,
	           FolderManager& folder_manager,
	           StatusBar& status_bar);
	~Operations();

	OperationNew* create_directory(InfBrowser* browser,
	                               const InfBrowserIter* parent,
	                               const Glib::ustring& name);

	OperationNew* create_document(InfBrowser* browser,
	                              const InfBrowserIter* parent,
	                              const Glib::ustring& name);

	OperationOpen* create_document(InfBrowser* browser,
	                               const InfBrowserIter* parent,
	                               const Glib::ustring& name,
	                               const Preferences& preferences,
	                               const Glib::ustring& from_uri,
	                               const char* encoding);

	OperationOpenMultiple* create_documents(InfBrowser* browser,
	                                        const InfBrowserIter* parent,
	                                        const Preferences& prefs,
	                                        const uri_list& uris);

	OperationSave* save_document(TextSessionView& view,
	                             const std::string& uri,
	                             const std::string& encoding,
	                             DocumentInfoStorage::EolStyle eol_style);

	OperationRename* rename_node(InfBrowser* browser,
	                             const InfBrowserIter* iter,
				     const Glib::ustring& name);

	OperationDelete* delete_node(InfBrowser* browser,
	                             const InfBrowserIter* iter);

	// uri must be of kind infinote://[hostname]/[path]
	OperationSubscribePath* subscribe_path(const std::string& uri);
	OperationSubscribePath* subscribe_path(InfBrowser* browser,
	                                       const std::string& path);

	OperationExportHtml* export_html(TextSessionView& view,
	                                 const std::string& uri);

	OperationSave* get_save_operation_for_document(TextSessionView& view);

	SignalBeginSaveOperation signal_begin_save_operation() const
	{
		return m_signal_begin_save_operation;
	}

protected:
	void fail_operation(Operation* operation);
	void finish_operation(Operation* operation);

	DocumentInfoStorage& m_info_storage;
	Browser& m_browser;
	FolderManager& m_folder_manager;
	StatusBar& m_status_bar;

	typedef std::set<Operation*> OperationSet;
	OperationSet m_operations;

	SignalBeginSaveOperation m_signal_begin_save_operation;
private:
	template<typename OperationType>
	OperationType* check_operation(OperationType* op)
	{
		if(m_operations.find(op) == m_operations.end())
			return NULL;
		return op;
	}
};

}
	
#endif // _GOBBY_OPERATIONS_OPERATIONS_HPP_
