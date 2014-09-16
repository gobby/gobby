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
                              FolderManager& folder_manager,
                              StatusBar& status_bar):
	m_info_storage(info_storage), m_browser(browser),
	m_folder_manager(folder_manager), m_status_bar(status_bar)
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
	OperationNew* op =
		new OperationNew(*this, browser, parent, name, true);
	m_operations.insert(op);
	op->start();
	return check_operation(op);
}

Gobby::OperationNew*
Gobby::Operations::create_document(InfBrowser* browser,
                                   const InfBrowserIter* parent,
                                   const Glib::ustring& name)
{
	OperationNew* op =
		new OperationNew(*this, browser, parent, name, false);
	m_operations.insert(op);
	op->start();
	return check_operation(op);
}

Gobby::OperationOpen*
Gobby::Operations::create_document(InfBrowser* browser,
                                   const InfBrowserIter* parent,
                                   const Glib::ustring& name,
                                   const Preferences& preferences,
                                   const Glib::RefPtr<Gio::File>& file,
                                   const char* encoding)
{
	OperationOpen* op = new OperationOpen(*this, preferences, browser,
	                                      parent, name, file, encoding);
	m_operations.insert(op);
	op->start();
	return check_operation(op);
}

Gobby::OperationOpenMultiple*
Gobby::Operations::create_documents(InfBrowser* browser,
                                    const InfBrowserIter* parent,
                                    const Preferences& prefs,
                                    const file_list& files)
{
	OperationOpenMultiple* op = new OperationOpenMultiple(*this, prefs,
	                                                      browser, parent,
	                                                      files);
	m_operations.insert(op);
	op->start();
	return check_operation(op);
}

Gobby::OperationSave*
Gobby::Operations::save_document(TextSessionView& view,
                                 const Glib::RefPtr<Gio::File>& file,
                                 const std::string& encoding,
                                 DocumentInfoStorage::EolStyle eol_style)
{
	OperationSave* prev_op = get_save_operation_for_document(view);

	// Cancel previous save operation:
	if(prev_op != NULL)
		fail_operation(prev_op);

	OperationSave* op = new OperationSave(*this, view, file,
	                                      encoding, eol_style);

	m_operations.insert(op);
	m_signal_begin_save_operation.emit(op);
	op->start();
	return check_operation(op);
}

Gobby::OperationDelete*
Gobby::Operations::delete_node(InfBrowser* browser,
                               const InfBrowserIter* iter)
{
	OperationDelete* op = new OperationDelete(*this, browser, iter);
	m_operations.insert(op);

	op->start();
	return check_operation(op);
}

Gobby::OperationSubscribePath*
Gobby::Operations::subscribe_path(const std::string& uri)
{
	OperationSubscribePath* op =
		new OperationSubscribePath(*this, uri);
	m_operations.insert(op);
	op->start();
	return check_operation(op);
}

Gobby::OperationSubscribePath*
Gobby::Operations::subscribe_path(InfBrowser* browser,
                                  const std::string& path)
{
	OperationSubscribePath* op =
		new OperationSubscribePath(*this, browser, path);
	m_operations.insert(op);
	op->start();
	return check_operation(op);
}

Gobby::OperationExportHtml*
Gobby::Operations::export_html(TextSessionView& view,
                               const Glib::RefPtr<Gio::File>& file)
{
	OperationExportHtml* op =
		new OperationExportHtml(*this, view, file);
	m_operations.insert(op);
	op->start();
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
