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

#ifndef _GOBBY_AUTOSAVE_COMMANDS_HPP_
#define _GOBBY_AUTOSAVE_COMMANDS_HPP_

#include <sigc++/trackable.h>

#include "operations/operations.hpp"

#include "core/folder.hpp"
#include "core/documentinfostorage.hpp"
#include "core/preferences.hpp"

namespace Gobby
{

class AutosaveCommands: public sigc::trackable
{
public:
	AutosaveCommands(const Folder& folder, Operations& operations,
	                 const DocumentInfoStorage& storage,
	                 const Preferences& preferences);
	~AutosaveCommands();

protected:
	void on_document_added(SessionView& view);
	void on_document_removed(SessionView& view);

	void on_begin_save_operation(OperationSave* operation);
	void on_autosave_enabled_changed();
	void on_autosave_interval_changed();

	const Folder& m_folder;
	Operations& m_operations;
	const DocumentInfoStorage& m_info_storage;
	const Preferences& m_preferences;

	class Info;
	typedef std::map<TextSessionView*, Info*> InfoMap;
	InfoMap m_info_map;
};

}
	
#endif // _GOBBY_AUTOSAVE_COMMANDS_HPP_
