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

#ifndef _GOBBY_BROWSER_CONTEXT_COMMANDS_HPP_
#define _GOBBY_BROWSER_CONTEXT_COMMANDS_HPP_

#include "operations/operations.hpp"

#include "dialogs/entry-dialog.hpp"

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

	void on_menu_node_removed();
	void on_menu_deactivate();

	void on_populate_popup(Gtk::Menu* menu);
	void on_popdown();

	// Context commands
	void on_disconnect(InfcBrowser* browser);

	void on_new(InfBrowser* browser, InfBrowserIter iter,
	            bool directory);
	void on_open(InfBrowser* browser, InfBrowserIter iter);
	void on_permissions(InfBrowser* browser, InfBrowserIter iter);
	void on_delete(InfBrowser* browser, InfBrowserIter iter);

	void on_dialog_node_removed();
	void on_new_response(int response_id, InfBrowser* browser,
	                     InfBrowserIter iter, bool directory);
	void on_open_response(int response_id, InfBrowser* browser,
	                      InfBrowserIter iter);
	void on_permissions_response(int response_id);

	Gtk::Window& m_parent;
	Browser& m_browser;
	FileChooser& m_file_chooser;
	Operations& m_operations;
	const Preferences& m_preferences;

	// Browser item for which
	Gtk::Menu* m_popup_menu;
	std::auto_ptr<NodeWatch> m_watch;
	std::auto_ptr<Gtk::Dialog> m_dialog;

	gulong m_populate_popup_handler;
};

}

#endif // _GOBBY_BROWSER_CONTEXT_COMMANDS_HPP_
