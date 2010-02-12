/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2010 Armin Burgmeier <armin@arbur.net>
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
#include "commands/subscription-commands.hpp"
#include "commands/synchronization-commands.hpp"
#include "commands/user-join-commands.hpp"
#include "commands/browser-context-commands.hpp"
#include "commands/auth-commands.hpp"
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
#include "core/closableframe.hpp"
#include "core/titlebar.hpp"

#include "util/config.hpp"

#include <gtkmm/window.h>
#include <gtkmm/paned.h>
#include <gtkmm/messagedialog.h>

#ifdef WITH_UNIQUE
# include <unique/unique.h>
#endif

#include <memory>

namespace Gobby
{

#ifdef WITH_UNIQUE
const int UNIQUE_GOBBY_CONNECT = 1;
#endif

class Window : public Gtk::Window
{
public:
	Window(unsigned int argc, const char* const argv[],
	       const IconManager& icon_mgr, Config& config
#ifdef WITH_UNIQUE
	       , UniqueApp* app
#endif
	       );
	~Window();

	const Folder& get_text_folder() const { return m_text_folder; }
	Folder& get_text_folder() { return m_text_folder; }

	void connect_to_host(const Glib::ustring& hostname)
	{
		m_browser.connect_to_host(hostname);
	}

protected:
#ifdef WITH_UNIQUE
	static UniqueResponse
	on_message_received_static(UniqueApp* app,
	                           UniqueCommand command,
	                           UniqueMessageData* message,
	                           guint time,
	                           gpointer user_data)
	{
		return static_cast<Window*>(user_data)->on_message_received(
			command, message, time);
	}
#endif

	// Gtk::Window overrides
	virtual bool on_delete_event(GdkEventAny* event);
	virtual bool on_key_press_event(GdkEventKey* event);

	virtual void on_realize();
	virtual void on_show();

	void on_initial_dialog_hide();

	static gboolean on_switch_to_chat_static(GtkAccelGroup* group,
	                                         GObject* acceleratable,
	                                         guint keyval,
	                                         GdkModifierType modifier,
	                                         gpointer user_data)
	{
		return static_cast<Window*>(user_data)->on_switch_to_chat();
	}

	static gboolean on_switch_to_text_static(GtkAccelGroup* group,
	                                         GObject* acceleratable,
	                                         guint keyval,
	                                         GdkModifierType modifier,
	                                         gpointer user_data)
	{
		return static_cast<Window*>(user_data)->on_switch_to_text();
	}

	bool on_switch_to_chat();
	bool on_switch_to_text();

	void on_chat_hide();
	void on_chat_show();

#ifdef WITH_UNIQUE
	UniqueResponse on_message_received(UniqueCommand command,
	                                   UniqueMessageData* message,
	                                   guint time);
#endif

	// Command line arguments
	// TODO: We only require these in on_show to initially open files
	// passed on the command line. We can't do it in the constructor
	// already, because otherwise the main window is shown after the
	// document location dialog, and therefore ends up having focus,
	// which it shouldn't. Maybe we'll find a better solution which does
	// not require these member variables.
	const unsigned int m_argc;
	const char* const* m_argv;

	// Config
	Config& m_config;
	GtkSourceLanguageManager* m_lang_manager;
	Preferences m_preferences;
	const IconManager& m_icon_mgr;

#ifdef WITH_UNIQUE
	UniqueApp* m_app;
#endif

	// GUI
	Gtk::VBox m_mainbox;
	Gtk::HPaned m_paned;
	Gtk::VPaned m_chat_paned;

	Header m_header;
	Browser m_browser;
	Folder m_text_folder;
	Folder m_chat_folder;
	ClosableFrame m_chat_frame;
	StatusBar m_statusbar;

	// Functionality
	DocumentInfoStorage m_info_storage;
	FileChooser m_file_chooser;
	Operations m_operations;

	BrowserCommands m_browser_commands;
	BrowserContextCommands m_browser_context_commands;

	AuthCommands m_auth_commands;

	AutosaveCommands m_autosave_commands;
	SubscriptionCommands m_subscription_commands;
	SynchronizationCommands m_synchronization_commands;
	UserJoinCommands m_user_join_commands;

	FolderCommands m_text_folder_commands;
	FolderCommands m_chat_folder_commands;
	FileCommands m_file_commands;
	EditCommands m_edit_commands;
	ViewCommands m_view_commands;
	HelpCommands m_help_commands;

	TitleBar m_title_bar;

	// Dialogs
	std::auto_ptr<InitialDialog> m_initial_dlg;
};

}

#endif // _GOBBY_WINDOW_HPP_
