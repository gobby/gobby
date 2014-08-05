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

#include "core/selfhoster.hpp"
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
	       Config& config, Preferences& preferences,
	       const IconManager& icon_manager,
	       CertificateManager& cert_manager
#ifdef WITH_UNIQUE
	       , UniqueApp* app
#endif
	       );
	~Window();

	void connect_to_host(const Glib::ustring& hostname)
	{
		m_operations.subscribe_path(hostname);
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
	unsigned int m_argc;
	const char* const* m_argv;

	// Config
	Config& m_config;
	GtkSourceLanguageManager* m_lang_manager;
	Preferences& m_preferences;
	CertificateManager& m_cert_manager;
	const IconManager& m_icon_manager;
	ConnectionManager m_connection_manager;
#ifdef WITH_UNIQUE
	UniqueApp* m_app;
#endif

	// GUI
	Gtk::VBox m_mainbox;
	Gtk::HPaned m_paned;
	Gtk::VPaned m_chat_paned;

	Folder m_text_folder;
	Folder m_chat_folder;
	StatusBar m_statusbar;
	Header m_header;
	Browser m_browser;
	ClosableFrame m_chat_frame;

	// Functionality
	DocumentInfoStorage m_info_storage;
	FolderManager m_folder_manager;
	FileChooser m_file_chooser;
	Operations m_operations;

	BrowserCommands m_browser_commands;
	BrowserContextCommands m_browser_context_commands;

	// TODO: This setup is a bit awkward. We need the auth commands to
	// provide a SASL context to the self hoster.
	// A better solution would be to have a new class, say AuthManager,
	// which creates the SASL context, and the auth manager is passed to
	// both connection manager and self hoster. The new class would not
	// do any UI, but it would do password checking from remote. For the
	// UI it would provide a hook which allows m_auth_commands to hook in.
	// This would also get rid of the ugly
	// connection_manager.set_sasl_context() call, since the connection
	// manager would then set the SASL context by itself.
	AuthCommands m_auth_commands;
	SelfHoster m_self_hoster;

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
