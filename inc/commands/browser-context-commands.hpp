/* gobby - A GTKmm driven libobby client
 * Copyright (C) 2005 - 2008 0x539 dev group
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

#ifndef _GOBBY_BROWSER_CONTEXT_COMMANDS_HPP_
#define _GOBBY_BROWSER_CONTEXT_COMMANDS_HPP_

#include "operations/operations.hpp"

#include "dialogs/entrydialog.hpp"

#include "core/nodewatch.hpp"
#include "core/browser.hpp"
#include "core/filechooser.hpp"

#include <sigc++/trackable.h>

namespace Gobby
{

class BrowserContextCommands: public sigc::trackable
{
public:
	BrowserContextCommands(Gtk::Window& parent,
	                       Browser& browser, FileChooser& chooser,
	                       Operations& operations,
	                       const Preferences& prf);
	~BrowserContextCommands();

protected:
	static void on_populate_popup_static(InfGtkBrowserView* view,
	                                     GtkMenu* menu,
					     gpointer user_data)
	{
		static_cast<BrowserContextCommands*>(user_data)->
			on_populate_popup(Glib::wrap(menu));
	}

	void on_node_removed();
	void on_menu_deactivate();

	void on_populate_popup(Gtk::Menu* menu);

	// Context commands
	void on_new(InfcBrowser* browser, InfcBrowserIter iter,
	            bool directory);
	void on_open(InfcBrowser* browser, InfcBrowserIter iter);
	void on_delete(InfcBrowser* browser, InfcBrowserIter iter);

	// on_new handlers
	void on_new_node_removed();

	void on_new_response(int response_id, InfcBrowser* browser,
	                     InfcBrowserIter iter, bool directory);

	// on_open handlers
	void on_open_node_removed();
	void on_open_response(int response_id, InfcBrowser* browser,
	                      InfcBrowserIter iter);

	Gtk::Window& m_parent;
	Browser& m_browser;
	FileChooser& m_file_chooser;
	Operations& m_operations;
	const Preferences& m_preferences;

	// Browser item for which
	Gtk::Menu* m_popup_menu;
	std::auto_ptr<NodeWatch> m_watch;
	std::auto_ptr<EntryDialog> m_entry_dialog;
	std::auto_ptr<FileChooser::Dialog> m_file_dialog;

	gulong m_populate_popup_handler;
};

}

#endif // _GOBBY_BROWSER_CONTEXT_COMMANDS_HPP_
