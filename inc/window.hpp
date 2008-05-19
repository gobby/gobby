/* gobby - A GTKmm driven libobby client
 * Copyright (C) 2005 0x539 dev group
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
#include "dragdrop.hpp"

#include "commands/browser-commands.hpp"
#include "commands/file-commands.hpp"
#include "operations/operations.hpp"

/*#include "dialogs/finddialog.hpp"
#include "dialogs/gotodialog.hpp"
#include "dialogs/preferencesdialog.hpp"*/
#include "dialogs/initialdialog.hpp"

#include "core/preferences.hpp"
#include "core/iconmanager.hpp"
#include "core/header.hpp"
#include "core/docwindow.hpp"
#include "core/folder.hpp"
#include "core/browser.hpp"
#include "core/statusbar.hpp"

#include "util/config.hpp"

#include <gtkmm/window.h>
#include <gtkmm/paned.h>
#include <gtkmm/frame.h>
#include <gtkmm/messagedialog.h>

#include <queue>
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

	Gtk::Frame m_frame_browser;
	Gtk::Frame m_frame_text;

	Header m_header;
	Folder m_folder;
	StatusBar m_statusbar;
	Browser m_browser;

	// Functionality
	Operations m_operations;

	BrowserCommands m_commands_browser;
	FileCommands m_commands_file;

	// TODO: Can't we use this directly now that the session is
	// "always open"?
	std::auto_ptr<DragDrop> m_dnd;

	// Dialogs
	std::auto_ptr<InitialDialog> m_initial_dlg;
/*	std::auto_ptr<PreferencesDialog> m_preferences_dlg;
	std::auto_ptr<FindDialog> m_finddialog;
	std::auto_ptr<GotoDialog> m_gotodialog;*/
};

}

#endif // _GOBBY_WINDOW_HPP_
