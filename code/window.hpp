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

#ifndef _GOBBY_WINDOW_HPP_
#define _GOBBY_WINDOW_HPP_

#include "features.hpp"

#include "commands/autosave-commands.hpp"
#include "commands/browser-commands.hpp"
#include "commands/browser-context-commands.hpp"
#include "commands/folder-commands.hpp"
#include "commands/file-commands.hpp"
#include "commands/edit-commands.hpp"
#include "commands/view-commands.hpp"
#include "commands/help-commands.hpp"
#include "operations/operations.hpp"

#include "dialogs/initial-dialog.hpp"

#include "core/iconmanager.hpp"
#include "core/header.hpp"
#include "core/folder.hpp"
#include "core/browser.hpp"
#include "core/statusbar.hpp"
#include "core/preferences.hpp"
#include "core/filechooser.hpp"
#include "core/titlebar.hpp"

#include "util/config.hpp"

#include <gtkmm/window.h>
#include <gtkmm/paned.h>
#include <gtkmm/messagedialog.h>

#include <memory>

namespace Gobby
{

class Window : public Gtk::Window
{
public:
	Window(const IconManager& icon_mgr, Config& config);
	~Window();

	const Folder& get_folder() const { return m_folder; }
	Folder& get_folder() { return m_folder; }

protected:
	// Gtk::Window overrides
	virtual bool on_delete_event(GdkEventAny* event);
	virtual bool on_key_press_event(GdkEventKey* event);

	virtual void on_realize();
	virtual void on_show();

	void on_initial_dialog_hide();

	// Config
	Config& m_config;
	GtkSourceLanguageManager* m_lang_manager;
	Preferences m_preferences;
	const IconManager& m_icon_mgr;

	// GUI
	Gtk::VBox m_mainbox;
	Gtk::HPaned m_paned;

	Header m_header;
	Browser m_browser;
	Folder m_folder;
	StatusBar m_statusbar;

	// Functionality
	DocumentInfoStorage m_info_storage;
	FileChooser m_file_chooser;
	Operations m_operations;

	AutosaveCommands m_commands_autosave;
	BrowserCommands m_commands_browser;
	BrowserContextCommands m_commands_browser_context;
	FolderCommands m_commands_folder;
	FileCommands m_commands_file;
	EditCommands m_commands_edit;
	ViewCommands m_commands_view;
	HelpCommands m_commands_help;

	TitleBar m_title_bar;

	// Dialogs
	std::auto_ptr<InitialDialog> m_initial_dlg;
};

}

#endif // _GOBBY_WINDOW_HPP_
