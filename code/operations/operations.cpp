/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2013 Armin Burgmeier <armin@arbur.net>
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

#include "operations/operation-new.hpp"
#include "operations/operation-open.hpp"
#include "operations/operation-open-multiple.hpp"
#include "operations/operation-save.hpp"
#include "operations/operation-delete.hpp"
#include "operations/operation-subscribe-path.hpp"
#include "operations/operation-export-html.hpp"

#include "operations/operations.hpp"

#include "core/noteplugin.hpp"
#include "util/i18n.hpp"

Gobby::Operations::Operation::~Operation() {}

Gobby::Operations::Operations(DocumentInfoStorage& info_storage,
                              Browser& browser,
                              StatusBar& status_bar):
	m_info_storage(info_storage), m_browser(browser),
	m_status_bar(status_bar)
{
}

Gobby::Operations::~Operations()
{
	for(OperationSet::iterator iter = m_operations.begin();
	    iter != m_operations.end(); ++ iter)
	{
		delete *iter;
	}
}

Gobby::OperationNew*
Gobby::Operations::create_directory(InfBrowser* browser,
                                    const InfBrowserIter* parent,
                                    const Glib::ustring& name)
{
	OperationNew* op = new OperationNew(*this, browser, parent,
	                                    name, true);

	m_operations.insert(op);
	return op;
}

Gobby::OperationNew*
Gobby::Operations::create_document(InfBrowser* browser,
                                   const InfBrowserIter* parent,
                                   const Glib::ustring& name)
{
	OperationNew* op = new OperationNew(*this, browser, parent,
	                                    name, false);

	m_operations.insert(op);
	return op;
}

Gobby::OperationOpen*
Gobby::Operations::create_document(InfBrowser* browser,
                                   const InfBrowserIter* parent,
                                   const Glib::ustring& name,
                                   const Preferences& preferences,
                                   const Glib::ustring& from_uri,
                                   const char* encoding)
{
	OperationOpen* op = new OperationOpen(*this, preferences, browser,
	                                      parent, name, from_uri,
	                                      encoding);

	m_operations.insert(op);
	return op;
}

Gobby::OperationOpenMultiple*
Gobby::Operations::create_documents(InfBrowser* browser,
                                    const InfBrowserIter* parent,
                                    const Preferences& prefs,
                                    unsigned int num_uris)
{
	OperationOpenMultiple* op = new OperationOpenMultiple(*this, prefs,
	                                                      browser, parent,
	                                                      num_uris);

	m_operations.insert(op);
	return op;
}

Gobby::OperationSave*
Gobby::Operations::save_document(TextSessionView& view,
                                 Folder& folder,
                                 const std::string& uri,
                                 const std::string& encoding,
                                 DocumentInfoStorage::EolStyle eol_style)
{
	OperationSave* prev_op = get_save_operation_for_document(view);

	// Cancel previous save operation:
	if(prev_op != NULL)
		fail_operation(prev_op);

	OperationSave* op = new OperationSave(*this, view, folder, uri,
	                                      encoding, eol_style);

	m_operations.insert(op);
	m_signal_begin_save_operation.emit(op);
	return op;
}

Gobby::OperationDelete*
Gobby::Operations::delete_node(InfBrowser* browser,
                               const InfBrowserIter* iter)
{
	OperationDelete* op = new OperationDelete(*this, browser, iter);
	m_operations.insert(op);
	return op;
}

Gobby::OperationSubscribePath*
Gobby::Operations::subscribe_path(Folder& folder,
                                  const std::string& uri)
{
	try
	{
		OperationSubscribePath* op =
			new OperationSubscribePath(*this, folder, uri);
		m_operations.insert(op);
		return op;
	}
	catch(const std::exception& ex)
	{
		m_status_bar.add_error_message(
		Glib::ustring::compose(
			_("Failed to connect to \"%1\""), uri),
		ex.what());

		return NULL;
	}
}

Gobby::OperationSubscribePath*
Gobby::Operations::subscribe_path(Folder& folder,
                                  InfBrowser* browser,
                                  const std::string& path)
{
	OperationSubscribePath* op =
		new OperationSubscribePath(*this, folder, browser, path);
	m_operations.insert(op);
	return op;
}

Gobby::OperationExportHtml*
Gobby::Operations::export_html(TextSessionView& view,
                               const std::string& uri)
{
	OperationExportHtml* op =
		new OperationExportHtml(*this, view, uri);
	m_operations.insert(op);
	return op;
}

Gobby::OperationSave*
Gobby::Operations::get_save_operation_for_document(TextSessionView& view)
{
	for(OperationSet::iterator iter = m_operations.begin();
	    iter != m_operations.end(); ++ iter)
	{
		Operation* op = *iter;
		OperationSave* save_op = dynamic_cast<OperationSave*>(op);
		if(save_op != NULL)
		{
			if(save_op->get_view() == &view)
				return save_op;
		}
	}

	return NULL;
}

void Gobby::Operations::finish_operation(Operation* operation)
{
	m_operations.erase(operation);
	operation->signal_finished().emit(true);
	delete operation;
}

void Gobby::Operations::fail_operation(Operation* operation)
{
	m_operations.erase(operation);
	operation->signal_finished().emit(false);
	delete operation;
}
