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
#include <gtkmm/radioaction.h>
#include <gtkmm/menubar.h>
#include <gtkmm/toolbar.h>

#include "sourceview/sourcelanguage.hpp"
#include "sourceview/sourcelanguagesmanager.hpp"

namespace Gobby
{

class Header: public Gtk::VBox
{
public:
	class Error: public Glib::Error
	{
	public:
		enum Code {
			MENUBAR_MISSING,
			TOOLBAR_MISSING
		};

		Error(Code error_code, const Glib::ustring& error_message);
		Code code() const;
	};

	class LanguageWrapper
	{
	public:
		typedef Glib::RefPtr<Gtk::RadioAction> Action;
		typedef Glib::RefPtr<Gtk::SourceLanguage> Language;

		LanguageWrapper(Action action,
		                Language language);

		Action get_action() const;
		Language get_language() const;
	protected:
		Action m_action;
		Language m_language;
	};

	Header();

	// Access to accelerator groups of the ui manager
	Glib::RefPtr<Gtk::AccelGroup> get_accel_group();
	Glib::RefPtr<const Gtk::AccelGroup> get_accel_group() const;

	Glib::RefPtr<Gtk::SourceLanguagesManager>
		get_lang_manager();
	Glib::RefPtr<const Gtk::SourceLanguagesManager>
		get_lang_manager() const;

	// Access to toolbar & menubar
	Gtk::MenuBar& get_menubar();
	Gtk::Toolbar& get_toolbar();

	const Glib::RefPtr<Gtk::ActionGroup> group_app;
	const Glib::RefPtr<Gtk::ActionGroup> group_session;
	const Glib::RefPtr<Gtk::ActionGroup> group_edit;
	const Glib::RefPtr<Gtk::ActionGroup> group_user;
	const Glib::RefPtr<Gtk::ActionGroup> group_view;
	const Glib::RefPtr<Gtk::ActionGroup> group_window;
	const Glib::RefPtr<Gtk::ActionGroup> group_help;

	const Glib::RefPtr<Gtk::Action> action_app;
	const Glib::RefPtr<Gtk::Action> action_app_session_create;
	const Glib::RefPtr<Gtk::Action> action_app_session_join;
	const Glib::RefPtr<Gtk::Action> action_app_session_save;
	const Glib::RefPtr<Gtk::Action> action_app_session_quit;
	const Glib::RefPtr<Gtk::Action> action_app_quit;

	const Glib::RefPtr<Gtk::Action> action_session;
	const Glib::RefPtr<Gtk::Action> action_session_document_create;
	const Glib::RefPtr<Gtk::Action> action_session_document_open;
	const Glib::RefPtr<Gtk::Action> action_session_document_save;
	const Glib::RefPtr<Gtk::Action> action_session_document_save_as;
	const Glib::RefPtr<Gtk::Action> action_session_document_close;

	const Glib::RefPtr<Gtk::Action> action_edit;
	const Glib::RefPtr<Gtk::Action> action_edit_preferences;

	const Glib::RefPtr<Gtk::Action> action_user;
	const Glib::RefPtr<Gtk::Action> action_user_set_password;
	const Glib::RefPtr<Gtk::Action> action_user_set_colour;

	const Glib::RefPtr<Gtk::Action> action_view;
	const Glib::RefPtr<Gtk::Action> action_view_preferences;
	const Glib::RefPtr<Gtk::Action> action_view_syntax;
	std::list<LanguageWrapper> action_view_syntax_languages;

	const Glib::RefPtr<Gtk::Action> action_window;
	const Glib::RefPtr<Gtk::ToggleAction> action_window_userlist;
	const Glib::RefPtr<Gtk::ToggleAction> action_window_documentlist;

	const Glib::RefPtr<Gtk::Action> action_help;
	const Glib::RefPtr<Gtk::Action> action_help_about;

protected:
	const Glib::RefPtr<Gtk::UIManager> m_ui_manager;
	Glib::RefPtr<Gtk::SourceLanguagesManager> m_lang_manager;

	Gtk::MenuBar* m_menubar;
	Gtk::Toolbar* m_toolbar;

	Gtk::RadioButtonGroup m_lang_group;
};

}

#endif // _GOBBY_HEADER_HPP_
