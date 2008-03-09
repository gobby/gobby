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
#include <gtkmm/toggleaction.h>
#include <gtkmm/radioaction.h>
#include <gtkmm/menubar.h>
#include <gtkmm/toolbar.h>

#include <gtksourceview/gtksourcelanguagemanager.h>

#include "preferences.hpp"

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

	class LanguageAction: public Gtk::RadioAction
	{
	protected:
		LanguageAction(GtkSourceLanguage* language,
		               Gtk::RadioAction::Group& group);
	public:
		static Glib::RefPtr<LanguageAction>
		create(GtkSourceLanguage* language,
		       Gtk::RadioAction::Group& group);

		GtkSourceLanguage* get_language() const { return m_language; }
	private:
		GtkSourceLanguage* m_language;
	};

        typedef std::list<Glib::RefPtr<LanguageAction> > LanguageList;
	typedef std::map<Glib::ustring, const LanguageList> LanguageMap;

	Header(Preferences& preferences,
	       GtkSourceLanguageManager* lang_mgr);

	Glib::RefPtr<Gtk::AccelGroup> get_accel_group();
	Glib::RefPtr<const Gtk::AccelGroup> get_accel_group() const;

	Gtk::MenuBar& get_menubar();
	Gtk::Toolbar& get_toolbar();

protected:
	Preferences& m_preferences;
	Gtk::RadioAction::Group m_highlight_group;

	const Glib::RefPtr<Gtk::UIManager> m_ui_manager;

	Gtk::MenuBar* m_menubar;
	Gtk::Toolbar* m_toolbar;

public:
	const Glib::RefPtr<Gtk::ActionGroup> group_file;
	const Glib::RefPtr<Gtk::ActionGroup> group_edit;
	const Glib::RefPtr<Gtk::ActionGroup> group_view;
	const Glib::RefPtr<Gtk::ActionGroup> group_help;

	const Glib::RefPtr<Gtk::Action> action_file;
	const Glib::RefPtr<Gtk::Action> action_file_new;
	const Glib::RefPtr<Gtk::Action> action_file_open;
	const Glib::RefPtr<Gtk::Action> action_file_save;
	const Glib::RefPtr<Gtk::Action> action_file_save_as;
	const Glib::RefPtr<Gtk::Action> action_file_save_all;
	const Glib::RefPtr<Gtk::Action> action_file_quit;

        const Glib::RefPtr<Gtk::Action> action_edit;
        const Glib::RefPtr<Gtk::Action> action_edit_undo;
        const Glib::RefPtr<Gtk::Action> action_edit_redo;
        const Glib::RefPtr<Gtk::Action> action_edit_cut;
        const Glib::RefPtr<Gtk::Action> action_edit_copy;
        const Glib::RefPtr<Gtk::Action> action_edit_paste;
        const Glib::RefPtr<Gtk::Action> action_edit_find;
        const Glib::RefPtr<Gtk::Action> action_edit_find_next;
        const Glib::RefPtr<Gtk::Action> action_edit_find_prev;
        const Glib::RefPtr<Gtk::Action> action_edit_find_replace;
        const Glib::RefPtr<Gtk::Action> action_edit_goto_line;
        const Glib::RefPtr<Gtk::Action> action_edit_preferences;

	const Glib::RefPtr<Gtk::Action> action_view;
	const Glib::RefPtr<Gtk::ToggleAction> action_view_toolbar;
	const Glib::RefPtr<Gtk::ToggleAction> action_view_statusbar;
	const Glib::RefPtr<Gtk::Action> action_view_highlight_mode;
	const Glib::RefPtr<LanguageAction> action_view_highlight_none;
        const LanguageMap action_view_highlight_languages;

	const Glib::RefPtr<Gtk::Action> action_help;
	const Glib::RefPtr<Gtk::Action> action_help_about;
};

}

#endif // _GOBBY_HEADER_HPP_
