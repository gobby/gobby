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

#include <gtkmm/stock.h>
#include <obby/format_string.hpp>

#include "common.hpp"
#include "header.hpp"
#include "icon.hpp"

#include <iostream>
namespace {
	Glib::ustring ui_desc = 
		"<ui>"
		"  <menubar name=\"MenuMainBar\">"
		"    <menu action=\"MenuFile\">"
		"      <menuitem action=\"FileNew\" />"
		"      <menuitem action=\"FileOpen\" />"
		"      <menuitem action=\"FileSave\" />"
		"      <menuitem action=\"FileSaveAs\" />"
		"      <menuitem action=\"FileSaveAll\" />"
		"      <separator />"
		"      <menuitem action=\"FileQuit\" />"
		"    </menu>"
		"    <menu action=\"MenuEdit\">"
		"      <menuitem action=\"EditUndo\" />"
		"      <menuitem action=\"EditRedo\" />"
		"      <separator />"
		"      <menuitem action=\"EditCut\" />"
		"      <menuitem action=\"EditCopy\" />"
		"      <menuitem action=\"EditPaste\" />"
		"      <separator />"
		"      <menuitem action=\"EditFind\" />"
		"      <menuitem action=\"EditFindNext\" />"
		"      <menuitem action=\"EditFindPrev\" />"
		"      <menuitem action=\"EditFindReplace\" />"
		"      <menuitem action=\"EditGotoLine\" />"
		"      <separator />"
		"      <menuitem action=\"EditPreferences\" />"
		"    </menu>"
		"    <menu action=\"MenuView\">"
		"      <menuitem action=\"ViewToolbar\" />"
		"      <menuitem action=\"ViewStatusbar\" />"
		"      <separator />"
		"      <menu action=\"ViewHighlightMode\">"
		"        <menuitem"
		"          action=\"ViewHighlightModeLanguage_None\" />"
		"      </menu>"
		"    </menu>"
		"    <menu action=\"MenuHelp\">"
		"      <menuitem action=\"HelpAbout\" />"
		"    </menu>"
		"  </menubar>"
		"  <toolbar name=\"ToolMainBar\">"
		"    <toolitem action=\"FileNew\" />"
		"    <toolitem action=\"FileOpen\" />"
		"    <toolitem action=\"FileSave\" />"
		"    <toolitem action=\"FileSaveAll\" />"
		"    <separator />"
		"    <toolitem action=\"EditUndo\" />"
		"    <toolitem action=\"EditRedo\" />"
		"    <separator />"
		"    <toolitem action=\"EditCut\" />"
		"    <toolitem action=\"EditCopy\" />"
		"    <toolitem action=\"EditPaste\" />"
		"    <separator />"
		"    <toolitem action=\"EditFind\" />"
		"    <toolitem action=\"EditFindReplace\" />"
		"  </toolbar>"
		"</ui>";

	void show_widget(Gtk::Widget& widget, bool show)
	{
		if(show)
			widget.show();
		else
			widget.hide();
	}

	bool
	language_sort_func(Glib::RefPtr<Gobby::Header::LanguageAction> act1,
	                   Glib::RefPtr<Gobby::Header::LanguageAction> act2)
	{
		// TODO: Speedup by using collation keys?
		// We should profile first.
		gchar* casefold1 = g_utf8_casefold(
			gtk_source_language_get_name(
				act1->get_language()), -1);
		gchar* casefold2 = g_utf8_casefold(
			gtk_source_language_get_name(
				act2->get_language()), -1);

		int ret = g_utf8_collate(casefold1, casefold2);

		g_free(casefold1);
		g_free(casefold2);

		return ret < 0;
	}

	Gobby::Header::LanguageMap
	load_highlight_languages(GtkSourceLanguageManager* manager,
	                         Gtk::RadioAction::Group& group)
	{
		typedef std::map<const Glib::ustring,
		                 Gobby::Header::LanguageList> TempLanguageMap;
		TempLanguageMap map;

		const gchar* const* language_ids =
			gtk_source_language_manager_get_language_ids(manager);

		for(const gchar* const* id = language_ids; *id != NULL; ++ id)
		{
			GtkSourceLanguage* language =
				gtk_source_language_manager_get_language(
					manager, *id);

			const Glib::ustring name(
				gtk_source_language_get_name(language));
			const Glib::ustring section(
				gtk_source_language_get_section(language));

			map[section].push_back(
				Gobby::Header::LanguageAction::create(
					language, group));
		}

		// Copy to map of const lists
		Gobby::Header::LanguageMap result;
		for(TempLanguageMap::iterator iter = map.begin();
		    iter != map.end();
		    ++ iter)
		{
			iter->second.sort(language_sort_func);
			result.insert(std::make_pair(iter->first,
			                             iter->second));
		}

		return result;
	}
}

Gobby::Header::Error::Error(Code error_code, const Glib::ustring& error_message)
 : Glib::Error(g_quark_from_static_string("GOBBY_HEADER_ERROR"),
               static_cast<int>(error_code), error_message)
{
}

Gobby::Header::Error::Code Gobby::Header::Error::code() const
{
	return static_cast<Code>(gobject_->code);
}

Gobby::Header::LanguageAction::LanguageAction(GtkSourceLanguage* language,
                                              Gtk::RadioAction::Group& group):
	Gtk::RadioAction(
		group,
		"ViewHighlightModeLanguage_" +
			Glib::ustring(language ?
				gtk_source_language_get_id(language) :
				"None"), Gtk::StockID(),
		language ? gtk_source_language_get_name(language) :
			_("_None")),
	m_language(language)
{
}

Glib::RefPtr<Gobby::Header::LanguageAction>
Gobby::Header::LanguageAction::create(GtkSourceLanguage* language,
                                      Gtk::RadioAction::Group& group)
{
	return Glib::RefPtr<LanguageAction>(
		new LanguageAction(language, group));
}

Gobby::Header::Header(Preferences& preferences,
                      GtkSourceLanguageManager* lang_mgr):
	m_preferences(preferences),

	group_file(Gtk::ActionGroup::create("MenuFile") ),
	group_edit(Gtk::ActionGroup::create("MenuEdit") ),
	group_view(Gtk::ActionGroup::create("MenuView") ),
	group_help(Gtk::ActionGroup::create("MenuHelp") ),

	action_file(Gtk::Action::create("MenuFile", _("_File"))),
	action_file_new(Gtk::Action::create("FileNew", Gtk::Stock::NEW)),
	action_file_open(Gtk::Action::create("FileOpen", Gtk::Stock::OPEN)),
	action_file_save(Gtk::Action::create("FileSave", Gtk::Stock::SAVE)),
	action_file_save_as(
		Gtk::Action::create("FileSaveAs", Gtk::Stock::SAVE_AS)),
	action_file_save_all(
		Gtk::Action::create(
			"FileSaveAll", Gtk::Stock::SAVE,
			_("Save all"), _("Save all open files locally"))),
	action_file_quit(Gtk::Action::create("FileQuit", Gtk::Stock::QUIT)),

	action_edit(Gtk::Action::create("MenuEdit", _("_Edit"))),
	action_edit_undo(Gtk::Action::create("EditUndo", Gtk::Stock::UNDO)),
	action_edit_redo(Gtk::Action::create("EditRedo", Gtk::Stock::REDO)),
	action_edit_cut(Gtk::Action::create("EditCut", Gtk::Stock::CUT)),
	action_edit_copy(Gtk::Action::create("EditCopy", Gtk::Stock::COPY)),
	action_edit_paste(
		Gtk::Action::create("EditPaste", Gtk::Stock::PASTE)),
	action_edit_find(Gtk::Action::create("EditFind", Gtk::Stock::FIND)),
	action_edit_find_next(
		Gtk::Action::create("EditFindNext", _("Find next"),
		                    _("Find next match of phrase "
		                      "searched for"))),
	action_edit_find_prev(
		Gtk::Action::create("EditFindPrev", _("Find prev"),
		                    _("Find previous match of phrase "
		                      "searched for"))),
	action_edit_find_replace(
		Gtk::Action::create("EditFindReplace",
		                    Gtk::Stock::FIND_AND_REPLACE)),
	action_edit_goto_line(
		Gtk::Action::create("EditGotoLine",
		                    Gtk::Stock::JUMP_TO)),
	action_edit_preferences(
		Gtk::Action::create("EditPreferences",
		                    Gtk::Stock::PREFERENCES)),

	action_view(Gtk::Action::create("MenuView", _("_View"))),
	action_view_toolbar(
		Gtk::ToggleAction::create(
			"ViewToolbar", _("View toolbar"),
		        _("Whether to show the toolbar"), 
			preferences.appearance.show_toolbar)),
	action_view_statusbar(
		Gtk::ToggleAction::create(
			"ViewStatusbar", _("View statusbar"),
			_("Whether to show the statusbar"),
			preferences.appearance.show_statusbar)),
	action_view_highlight_mode(
		Gtk::Action::create("ViewHighlightMode",
		                    _("_Highlight Mode"))),
	action_view_highlight_none(
		LanguageAction::create(NULL, m_highlight_group)),
	action_view_highlight_languages(
		load_highlight_languages(lang_mgr, m_highlight_group)),

	action_help(Gtk::Action::create("MenuHelp", _("_Help")) ),
	action_help_about(
		Gtk::Action::create(
			"HelpAbout", Gtk::Stock::ABOUT, _("About"),
			_("Shows Gobby's copyright and credits"))),

	m_ui_manager(Gtk::UIManager::create())
{
	// Add basic menu
	m_ui_manager->add_ui_from_string(ui_desc);

	group_file->add(action_file);
	group_file->add(action_file_new);
	group_file->add(action_file_open);
	group_file->add(action_file_save);
	group_file->add(action_file_save_as);
	group_file->add(action_file_save_all,
	                Gtk::AccelKey(""));
	group_file->add(action_file_quit);

	group_edit->add(action_edit);
	group_edit->add(action_edit_undo);
	group_edit->add(action_edit_redo);
	group_edit->add(action_edit_cut);
	group_edit->add(action_edit_copy);
	group_edit->add(action_edit_paste);
	group_edit->add(action_edit_find);
	group_edit->add(action_edit_find_next,
	                Gtk::AccelKey("<control>G",
	                              "<Actions>/MenuEdit/EditFindNext"));
	group_edit->add(action_edit_find_prev,
	                Gtk::AccelKey("<control><shift>G",
			              "<Actions>/MenuEdit/EditFindPrev"));
	group_edit->add(action_edit_find_replace);
	group_edit->add(action_edit_goto_line,
	                Gtk::AccelKey("<control>I",
	                              "<Actions>/MenuEdit/EditGotoLine"));
	group_edit->add(action_edit_preferences);

	group_edit->add(action_view);
	group_edit->add(action_view_toolbar);
	group_edit->add(action_view_statusbar);
	group_edit->add(action_view_highlight_mode);
	group_edit->add(action_view_highlight_none);
	for(LanguageMap::const_iterator iter =
		action_view_highlight_languages.begin();
	    iter != action_view_highlight_languages.end();
	    ++ iter)
	{
		Glib::ustring section_action_name =
			"ViewHighlightModeSection_" + iter->first;
		Glib::ustring section_action_xml =
			Glib::Markup::escape_text(section_action_name);

		Glib::RefPtr<Gtk::Action> section_action(
			Gtk::Action::create(section_action_name,
			                    iter->first));

		group_view->add(section_action);

		for(LanguageList::const_iterator iter2 =
			iter->second.begin();
		    iter2 != iter->second.end();
		    ++ iter2)
		{
			Glib::ustring language_action_xml =
				Glib::Markup::escape_text(
					(*iter2)->get_name());
			
			Glib::ustring xml_desc =
				"<ui>"
				"  <menubar name=\"MenuMainBar\">"
				"    <menu action=\"MenuView\">"
				"      <menu action=\"ViewHighlightMode\">"
				"        <menu action=\"" +
					section_action_xml + "\">"
				"	   <menuitem action=\"" +
					language_action_xml + "\" />"
				"        </menu>"
				"      </menu>"
				"    </menu>"
				"  </menubar>"
				"</ui>";
			m_ui_manager->add_ui_from_string(xml_desc);

			group_view->add(*iter2);
		}
	}

	group_help->add(action_help);
	group_help->add(action_help_about);

	m_ui_manager->insert_action_group(group_file);
	m_ui_manager->insert_action_group(group_edit);
	m_ui_manager->insert_action_group(group_view);
	m_ui_manager->insert_action_group(group_help);

	m_menubar = static_cast<Gtk::MenuBar*>(
		m_ui_manager->get_widget("/MenuMainBar") );
	m_toolbar = static_cast<Gtk::Toolbar*>(
		m_ui_manager->get_widget("/ToolMainBar") );

	if(m_menubar == NULL)
	{
		throw Error(
			Error::MENUBAR_MISSING,
			"XML UI definition lacks menubar"
		);
	}

	if(m_toolbar == NULL)
	{
		throw Error(
			Error::TOOLBAR_MISSING,
			"XML UI definition lacks toolbar"
		);
	}

	pack_start(*m_menubar, Gtk::PACK_SHRINK);
	pack_start(*m_toolbar, Gtk::PACK_SHRINK);

	m_toolbar->set_toolbar_style(preferences.appearance.toolbar_style);
	m_menubar->show();
	if(preferences.appearance.show_toolbar) m_toolbar->show();

	preferences.appearance.toolbar_style.signal_changed().connect(
		sigc::compose(
			sigc::mem_fun(
				*m_toolbar,
				&Gtk::Toolbar::set_toolbar_style),
			sigc::mem_fun(
				preferences.appearance.toolbar_style,
				&Preferences::Option<Gtk::ToolbarStyle>::
					operator const Gtk::ToolbarStyle&)));

	preferences.appearance.show_toolbar.signal_changed().connect(
		sigc::compose(
			sigc::bind<0>(
				sigc::ptr_fun(show_widget),
				sigc::ref(*m_toolbar)),
			sigc::mem_fun(
				preferences.appearance.show_toolbar,
				&Preferences::Option<bool>::
					operator const bool&)));
}

Glib::RefPtr<Gtk::AccelGroup> Gobby::Header::get_accel_group()
{
	return m_ui_manager->get_accel_group();
}

Glib::RefPtr<const Gtk::AccelGroup> Gobby::Header::get_accel_group() const
{
	return m_ui_manager->get_accel_group();
}

Gtk::MenuBar& Gobby::Header::get_menubar()
{
	return *m_menubar;
}

Gtk::Toolbar& Gobby::Header::get_toolbar()
{
	return *m_toolbar;
}
