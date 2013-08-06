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
		unsigned int num_uris):
	Operation(operations), m_preferences(prefs),
	m_parent(browser, parent), m_num_uris(num_uris),
	m_current(NULL)
{
	m_parent.signal_node_removed().connect(
		sigc::mem_fun(*this,
			&OperationOpenMultiple::on_node_removed));
}

void Gobby::OperationOpenMultiple::add_uri(const Glib::ustring& uri,
                                           const char* name,
                                           const char* encoding)
{
	g_assert(m_num_uris > 0);
	-- m_num_uris;

	Glib::RefPtr<Gio::File> file = Gio::File::create_for_uri(uri);

	info_list::iterator iter = m_infos.insert(m_infos.end(), Info());
	Info& info = *iter;

	info.uri = uri;
	info.encoding = encoding;

	if(name == NULL)
	{
		try
		{
			// Query file name
			file->query_info_async(
				sigc::bind(
					sigc::mem_fun(
						*this,
						&OperationOpenMultiple::
							on_query_info),
					file, iter),
				G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME);
		}
		catch(const Gio::Error& ex)
		{
			single_error(iter, ex.what());
		}
	}
	else
	{
		info.name = name;
		if(!m_current) load_info(iter);
	}
}

void Gobby::OperationOpenMultiple::on_node_removed()
{
	fatal_error(_("Parent folder was removed"));
}

void Gobby::OperationOpenMultiple::on_query_info(
		const Glib::RefPtr<Gio::AsyncResult>& result,
		const Glib::RefPtr<Gio::File>& file,
	const info_list::iterator& info)
{
	try
	{
		Glib::RefPtr<Gio::FileInfo> file_info =
			file->query_info_finish(result);

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

	if(!m_num_uris && m_infos.empty())
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
		// available, either by the user adding more uris via
		// add_uri(), or by query info results.
	}
}

void Gobby::OperationOpenMultiple::load_info(const info_list::iterator& iter)
{
	g_assert(m_current == NULL);
	g_assert(!iter->name.empty());

	m_current = m_operations.create_document(
		m_parent.get_browser(), m_parent.get_browser_iter(),
		iter->name, m_preferences, iter->uri, iter->encoding);

	m_current->signal_finished().connect(
		sigc::bind(
			sigc::mem_fun(
				*this, &OperationOpenMultiple::on_finished),
			iter));
}

void Gobby::OperationOpenMultiple::single_error(
		const info_list::iterator& iter,
		const Glib::ustring& message)
{
	get_status_bar().add_error_message(
		Glib::ustring::compose(
			_("Failed to open document \"%1\""), iter->uri),
		message);

	m_infos.erase(iter);

	// Finish operation if there are no more URIs to load
	if(!m_num_uris && m_infos.empty())
		finish();
}

void Gobby::OperationOpenMultiple::fatal_error(const Glib::ustring& message)
{
	get_status_bar().add_error_message(
		_("Failed to open multiple documents"),
		message);

	fail();
}
