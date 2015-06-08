/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2015 Armin Burgmeier <armin@arbur.net>
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

// TODO: Show "Querying file name(s)..." in statusbar when querying file
// names without actually opening a file because of waiting for the file name.

#include "operations/operation-open-multiple.hpp"
#include "operations/operation-open.hpp"
#include "util/i18n.hpp"

Gobby::OperationOpenMultiple::OperationOpenMultiple(
		Operations& operations,
		const Preferences& prefs,
		InfBrowser* browser,
		const InfBrowserIter* parent,
		const file_list& files):
	Operation(operations), m_preferences(prefs),
	m_parent(browser, parent), m_current(NULL)
{
	m_parent.signal_node_removed().connect(
		sigc::mem_fun(*this,
			&OperationOpenMultiple::on_node_removed));

	for(file_list::const_iterator iter = files.begin();
	    iter != files.end(); ++iter)
	{
		info_list::iterator info_iter =
			m_infos.insert(m_infos.end(), Info());
		Info& info = *info_iter;

		info.file = *iter;
		info.encoding = NULL; /* auto-detect... */
	}
}

void Gobby::OperationOpenMultiple::start()
{
	for(info_list::iterator iter = m_infos.begin();
	    iter != m_infos.end(); ++iter)
	{
		query(iter);
	}
}

void Gobby::OperationOpenMultiple::query(const info_list::iterator& info)
{
	if(info->name.empty())
	{
		try
		{
			// Query file name
			info->file->query_info_async(
				sigc::bind(
					sigc::mem_fun(
						*this,
						&OperationOpenMultiple::
							on_query_info),
					info),
					G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME);
		}
		catch(const Gio::Error& ex)
		{
			single_error(info, ex.what());
		}
	}
	else
	{
		if(!m_current)
			load_info(info);
	}
}

void Gobby::OperationOpenMultiple::on_node_removed()
{
	fatal_error(_("Parent folder was removed"));
}

void Gobby::OperationOpenMultiple::on_query_info(
		const Glib::RefPtr<Gio::AsyncResult>& result,
		const info_list::iterator& info)
{
	try
	{
		Glib::RefPtr<Gio::FileInfo> file_info =
			info->file->query_info_finish(result);

		info->name = file_info->get_display_name();
		if(!m_current) load_info(info);
	}
	catch(const Gio::Error& ex)
	{
		single_error(info, ex.what());
	}
}

void Gobby::OperationOpenMultiple::on_finished(
		bool success,
		const info_list::iterator& info)
{
	m_infos.erase(info);
	m_current = NULL;

	if(m_infos.empty())
	{
		// All documents loaded
		finish();
	}
	else
	{
		// Find the next info with name set
		for(info_list::iterator iter = m_infos.begin();
		    iter != m_infos.end(); ++iter)
		{
			if(!iter->name.empty())
			{
				load_info(iter);
				break;
			}
		}

		// If no info was found, then wait for names to become
		// available, by query info results
	}
}

void Gobby::OperationOpenMultiple::load_info(const info_list::iterator& iter)
{
	g_assert(m_current == NULL);
	g_assert(!iter->name.empty());

	m_current = m_operations.create_document(
		m_parent.get_browser(), m_parent.get_browser_iter(),
		iter->name, m_preferences, iter->file, iter->encoding);

	// TODO: In principle this could be NULL if the whole operation
	// finished synchrounously. In this case with the current API we
	// cannot find out whether the operation was successful or not. What
	// we do does not depend on whether the operation is successful,
	// so it does not matter at this point. But in principle we should
	// change the API so that we can find it out here. Note also that
	// currently, OperationOpen can never finish synchrounously.
	if(m_current == NULL)
	{
		on_finished(true, iter);
	}
	else
	{
		m_current->signal_finished().connect(
			sigc::bind(
				sigc::mem_fun(
					*this,
					&OperationOpenMultiple::on_finished),
				iter));
	}
}

void Gobby::OperationOpenMultiple::single_error(
		const info_list::iterator& iter,
		const Glib::ustring& message)
{
	get_status_bar().add_error_message(
		Glib::ustring::compose(
			_("Failed to open document \"%1\""), iter->file->get_uri()),
		message);

	m_infos.erase(iter);

	// Finish operation if there are no more files to load
	if(m_infos.empty())
		finish();
}

void Gobby::OperationOpenMultiple::fatal_error(const Glib::ustring& message)
{
	get_status_bar().add_error_message(
		_("Failed to open multiple documents"),
		message);

	fail();
}
