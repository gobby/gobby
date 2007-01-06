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

#ifndef _GOBBY_HEADER_HPP_
#define _GOBBY_HEADER_HPP_

#include <gtkmm/box.h>
#include <gtkmm/uimanager.h>
#include <gtkmm/menubar.h>
#include <gtkmm/toolbar.h>
#include <obby/user.hpp>
#include <obby/local_document_info.hpp>
#include <obby/local_buffer.hpp>
#include "features.hpp"
#include "document.hpp"
#include "folder.hpp"
#include "preferences.hpp"

namespace Gobby
{

class Header : public Gtk::VBox
{
public:
	class Error : public Glib::Error
	{
	public:
		enum Code {
			MENUBAR_MISSING,
			TOOLBAR_MISSING
		};

		Error(Code error_code, const Glib::ustring& error_message);
		Code code() const;
	};

	typedef sigc::signal<void> signal_session_create_type;
	typedef sigc::signal<void> signal_session_join_type;
	typedef sigc::signal<void> signal_session_quit_type;
	typedef sigc::signal<void> signal_document_create_type;
	typedef sigc::signal<void> signal_document_open_type;
	typedef sigc::signal<void> signal_document_save_type;
	typedef sigc::signal<void> signal_document_save_as_type;
	typedef sigc::signal<void> signal_document_close_type;
	typedef sigc::signal<void> signal_edit_preferences_type;
	typedef sigc::signal<void> signal_user_set_password_type;
	typedef sigc::signal<void> signal_user_set_colour_type;
	typedef sigc::signal<void> signal_view_preferences_type;
	typedef sigc::signal<void, const Glib::RefPtr<Gtk::SourceLanguage>&>
		signal_view_language_type;
	typedef sigc::signal<void> signal_about_type;
	typedef sigc::signal<void> signal_quit_type;

	Header(const Folder& folder);
	~Header();

	// Access to accelerator groups of the ui manager
	Glib::RefPtr<Gtk::AccelGroup> get_accel_group();
	Glib::RefPtr<const Gtk::AccelGroup> get_accel_group() const;

	// Access to toolbar & menubar
	Gtk::MenuBar& get_menubar();
	Gtk::Toolbar& get_toolbar();

	// Disables actions that deal with documents. The windowscalls this
	// after all remaining documents have been closed after a connection
	// loss.
	void disable_document_actions();

	signal_session_create_type session_create_event() const;
	signal_session_join_type session_join_event() const;
	signal_session_quit_type session_quit_event() const;
	signal_document_create_type document_create_event() const;
	signal_document_open_type document_open_event() const;
	signal_document_save_type document_save_event() const;
	signal_document_save_as_type document_save_as_event() const;
	signal_document_close_type document_close_event() const;
	signal_edit_preferences_type edit_preferences_event() const;
	signal_user_set_password_type user_set_password_event() const;
	signal_user_set_colour_type user_set_colour_event() const;
	signal_view_preferences_type view_preferences_event() const;
	signal_view_language_type view_language_event() const;
	signal_about_type about_event() const;
	signal_quit_type quit_event() const;

	// Calls from the window
	void obby_start(obby::local_buffer& buf);
	void obby_end();
	void obby_user_join(obby::user& user);
	void obby_user_part(obby::user& user);
	void obby_document_insert(obby::local_document_info& document);
	void obby_document_remove(obby::local_document_info& document);

protected:
	void on_app_session_create();
	void on_app_session_join();
	void on_app_session_quit();
	void on_app_document_create();
	void on_app_document_open();
	void on_app_document_save();
	void on_app_document_save_as();
	void on_app_document_close();
	void on_app_edit_preferences();
	void on_app_user_set_password();
	void on_app_user_set_colour();
	void on_app_view_preferences();
	void on_app_view_language(Glib::RefPtr<Gtk::SourceLanguage> lang);
	void on_app_about();
	void on_app_quit();

	void on_folder_tab_switched(Document& document);

	Glib::RefPtr<Gtk::UIManager> m_ui_manager;
	Glib::RefPtr<Gtk::ActionGroup> m_group_app;

	Gtk::MenuBar* m_menubar;
	Gtk::Toolbar* m_toolbar;

	Gtk::RadioButtonGroup m_lang_group;

	/** Boolean variable whether we are currently toggling line numbers to
	 * prevent recursion.
	 */
	bool m_toggle_language;

	// Callback for sorting
	static bool language_sort_callback(
		const Glib::RefPtr<Gtk::SourceLanguage>& lang1,
		const Glib::RefPtr<Gtk::SourceLanguage>& lang2
	);

	signal_session_create_type m_signal_session_create;
	signal_session_join_type m_signal_session_join;
	signal_session_quit_type m_signal_session_quit;
	signal_document_create_type m_signal_document_create;
	signal_document_open_type m_signal_document_open;
	signal_document_save_type m_signal_document_save;
	signal_document_save_as_type m_signal_document_save_as;
	signal_document_close_type m_signal_document_close;
	signal_edit_preferences_type m_signal_edit_preferences;
	signal_user_set_password_type m_signal_user_set_password;
	signal_user_set_colour_type m_signal_user_set_colour;
	signal_view_preferences_type m_signal_view_preferences;
	signal_view_language_type m_signal_view_language;
	signal_about_type m_signal_about;
	signal_quit_type m_signal_quit;
};

}

#endif // _GOBBY_HEADER_HPP_

