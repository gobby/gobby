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

#ifndef _GOBBY_USER_JOIN_COMMANDS_HPP_
#define _GOBBY_USER_JOIN_COMMANDS_HPP_

#include "core/foldermanager.hpp"
#include "core/preferences.hpp"
#include "core/userjoin.hpp"

#include <sigc++/trackable.h>

namespace Gobby
{

class UserJoinCommands: public sigc::trackable
{
public:
	UserJoinCommands(FolderManager& folder_manager,
	                 const Preferences& preferences);
	~UserJoinCommands();

protected:
	void on_document_added(InfBrowser* browser,
	                       const InfBrowserIter* iter,
	                       InfSessionProxy* proxy,
	                       Folder& folder,
                               SessionView& view);
	void on_document_removed(InfBrowser* browser,
	                         const InfBrowserIter* iter,
	                         InfSessionProxy* proxy,
	                         Folder& folder,
	                         SessionView& view);
	void on_user_join_finished(InfSessionProxy* proxy,
	                           Folder& folder,
	                           SessionView& view,
	                           InfUser* user,
	                           const GError* error);

	const Preferences& m_preferences;

	class UserJoinInfo;
	typedef std::map<InfSessionProxy*, UserJoinInfo*> UserJoinMap;
	UserJoinMap m_user_join_map;
};

}

#endif // _GOBBY_USER_JOIN_COMMANDS_HPP_
