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

	Glib::RefPtr<LanguageAction>
	lookup_language_action(GtkSourceLanguage* language);

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
	const Glib::RefPtr<Gtk::Action> action_file_open_location;
	const Glib::RefPtr<Gtk::Action> action_file_save;
	const Glib::RefPtr<Gtk::Action> action_file_save_as;
	const Glib::RefPtr<Gtk::Action> action_file_save_all;
	const Glib::RefPtr<Gtk::Action> action_file_export_html;
	const Glib::RefPtr<Gtk::Action> action_file_connect;
	const Glib::RefPtr<Gtk::Action> action_file_close;
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
	const Glib::RefPtr<Gtk::Action> action_view_hide_user_colors;
	const Glib::RefPtr<Gtk::ToggleAction> action_view_fullscreen;
	const Glib::RefPtr<Gtk::Action> action_view_zoom_in;
	const Glib::RefPtr<Gtk::Action> action_view_zoom_out;
	const Glib::RefPtr<Gtk::ToggleAction> action_view_toolbar;
	const Glib::RefPtr<Gtk::ToggleAction> action_view_statusbar;
	const Glib::RefPtr<Gtk::ToggleAction> action_view_browser;
	const Glib::RefPtr<Gtk::ToggleAction> action_view_chat;
	const Glib::RefPtr<Gtk::ToggleAction> action_view_document_userlist;
	const Glib::RefPtr<Gtk::ToggleAction> action_view_chat_userlist;
	const Glib::RefPtr<Gtk::Action> action_view_highlight_mode;
	const Glib::RefPtr<LanguageAction> action_view_highlight_none;
	const LanguageMap action_view_highlight_languages;

	const Glib::RefPtr<Gtk::Action> action_help;
	const Glib::RefPtr<Gtk::Action> action_help_contents;
	const Glib::RefPtr<Gtk::Action> action_help_about;
};

}

#endif // _GOBBY_HEADER_HPP_
