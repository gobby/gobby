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

#include <list>

#include <gtkmm/box.h>
#include <gtkmm/uimanager.h>
#include <gtkmm/radioaction.h>
#include <gtkmm/menubar.h>
#include <gtkmm/toolbar.h>

#include "preferences.hpp" // Defines GtkSourceLanguageManager (gtksourceview1)
#include "application_state.hpp"

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

	/** @brief Action that automatically chances sensitivity depending
	 * on application state.
	 */
	class AutoAction
	{
	public:
		typedef Glib::RefPtr<Gtk::Action> action_type;

		AutoAction(action_type action,
		           const ApplicationState& state,
		           ApplicationFlags inc_flags,
			   ApplicationFlags exc_flags);

	protected:
		void on_state_change(const ApplicationState& state);

		action_type m_action;
		ApplicationFlags m_inc_flags;
		ApplicationFlags m_exc_flags;
	};

	/** @brief Class that stores multiple AutoActions.
	 *
	 * Once an AutoAction has been created, it works without any further
	 * need to access the AutoAction object again, one just needs to
	 * keep the AutoAction somewhere and release it when the application
	 * exits.
	 *
	 * So this class does just keep auto actions without providing access
	 * to them.
	 */
	class AutoList
	{
	public:
		typedef AutoAction::action_type action_type;

		void add(action_type action,
		         const ApplicationState& state,
			 ApplicationFlags inc_flags,
			 ApplicationFlags exc_flags);

		~AutoList();

	protected:
		std::list<AutoAction*> m_list;
	};

	class LanguageWrapper
	{
	public:
		//typedef AutoAction<Gtk::RadioAction> Action;
		typedef Glib::RefPtr<Gtk::RadioAction> Action;

		LanguageWrapper(Action action,
		                GtkSourceLanguage* language);
		~LanguageWrapper();

		Action get_action() const;
		GtkSourceLanguage* get_language() const;
	protected:
		Action m_action;
		GtkSourceLanguage* m_language;
	};

	Header(const ApplicationState& state,
	       GtkSourceLanguageManager* lang_mgr);

	// Access to accelerator groups of the ui manager
	Glib::RefPtr<Gtk::AccelGroup> get_accel_group();
	Glib::RefPtr<const Gtk::AccelGroup> get_accel_group() const;

	// Access to toolbar & menubar
	Gtk::MenuBar& get_menubar();
	Gtk::Toolbar& get_toolbar();

	const Glib::RefPtr<Gtk::ActionGroup> group_app;
	const Glib::RefPtr<Gtk::ActionGroup> group_session;
	const Glib::RefPtr<Gtk::ActionGroup> group_edit;
	const Glib::RefPtr<Gtk::ActionGroup> group_user;
	const Glib::RefPtr<Gtk::ActionGroup> group_window;
	const Glib::RefPtr<Gtk::ActionGroup> group_help;

	const Glib::RefPtr<Gtk::Action> action_app;
	const Glib::RefPtr<Gtk::Action> action_app_session_create;
	const Glib::RefPtr<Gtk::Action> action_app_session_join;
	const Glib::RefPtr<Gtk::Action> action_app_session_save;
	const Glib::RefPtr<Gtk::Action> action_app_session_save_as;
	const Glib::RefPtr<Gtk::Action> action_app_session_quit;
	const Glib::RefPtr<Gtk::Action> action_app_quit;

	const Glib::RefPtr<Gtk::Action> action_session;
	const Glib::RefPtr<Gtk::Action> action_session_document_create;
	const Glib::RefPtr<Gtk::Action> action_session_document_open;
	const Glib::RefPtr<Gtk::Action> action_session_document_save;
	const Glib::RefPtr<Gtk::Action> action_session_document_save_as;
	const Glib::RefPtr<Gtk::Action> action_session_document_close;

	const Glib::RefPtr<Gtk::Action> action_edit;
	const Glib::RefPtr<Gtk::Action> action_edit_search;
	const Glib::RefPtr<Gtk::Action> action_edit_search_replace;
	const Glib::RefPtr<Gtk::Action> action_edit_goto_line;
	const Glib::RefPtr<Gtk::Action> action_edit_preferences;
	const Glib::RefPtr<Gtk::Action> action_edit_document_preferences;
	const Glib::RefPtr<Gtk::Action> action_edit_syntax;
	std::list<LanguageWrapper> action_edit_syntax_languages;

	const Glib::RefPtr<Gtk::Action> action_user;
	const Glib::RefPtr<Gtk::Action> action_user_set_password;
	const Glib::RefPtr<Gtk::Action> action_user_set_colour;

	const Glib::RefPtr<Gtk::Action> action_window;
	const Glib::RefPtr<Gtk::ToggleAction> action_window_userlist;
	const Glib::RefPtr<Gtk::ToggleAction> action_window_documentlist;
	const Glib::RefPtr<Gtk::ToggleAction> action_window_chat;

	const Glib::RefPtr<Gtk::Action> action_help;
	const Glib::RefPtr<Gtk::Action> action_help_about;

protected:
	void set_action_auto(const Glib::RefPtr<Gtk::Action>& action,
	                     const ApplicationState& state,
	                     ApplicationFlags inc_flags,
	                     ApplicationFlags exc_flags);

	const Glib::RefPtr<Gtk::UIManager> m_ui_manager;

	Gtk::MenuBar* m_menubar;
	Gtk::Toolbar* m_toolbar;

	Gtk::RadioButtonGroup m_lang_group;

	/** @brief List that just stores internally the auto actions to have
	 * the actions change sensitivity automatically.
	 */
	AutoList m_auto_actions;
};

}

#endif // _GOBBY_HEADER_HPP_
