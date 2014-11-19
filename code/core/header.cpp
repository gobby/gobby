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

#include "core/header.hpp"
#include "util/i18n.hpp"
#include "features.hpp" // For PLATFORM_OSX_NATIVE

#include <glibmm/markup.h>
#include <gtksourceview/gtksourcelanguage.h>
#include <gdk/gdkkeysyms.h>

#ifdef PLATFORM_OSX_NATIVE
#include <ige-mac-menu.h>
#endif

namespace {
	Glib::ustring ui_desc = 
		"<ui>"
		"  <menubar name=\"MenuMainBar\">"
		"    <menu action=\"MenuFile\">"
		"      <menuitem action=\"FileNew\" />"
		"      <menuitem action=\"FileOpen\" />"
		"      <menuitem action=\"FileOpenLocation\" />"
		"      <separator />"
		"      <menuitem action=\"FileSave\" />"
		"      <menuitem action=\"FileSaveAs\" />"
		"      <menuitem action=\"FileSaveAll\" />"
		"      <separator />"
		"      <menuitem action=\"FileExportHtml\" />"
		"      <menuitem action=\"FileConnect\" />"
		"      <separator />"
		"      <menuitem action=\"FileClose\" />"
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
		"      <menuitem action=\"ViewHideUserColors\" />"
		"      <separator />"
		"      <menuitem action=\"ViewFullscreen\" />"
		"      <menuitem action=\"ViewZoomIn\" />"
		"      <menuitem action=\"ViewZoomOut\" />"
		"      <separator />"
		"      <menuitem action=\"ViewToolbar\" />"
		"      <menuitem action=\"ViewStatusbar\" />"
		"      <separator />"
		"      <menuitem action=\"ViewBrowser\" />"
		"      <menuitem action=\"ViewChat\" />"
		"      <menuitem action=\"ViewDocumentUserList\" />"
		"      <menuitem action=\"ViewChatUserList\" />"
		"      <separator />"
		"      <menu action=\"ViewHighlightMode\">"
		"        <menuitem"
		"          action=\"ViewHighlightModeLanguage_None\" />"
		"      </menu>"
		"    </menu>"
		"    <menu action=\"MenuHelp\">"
		"      <menuitem action=\"HelpContents\" />"
		"      <separator />"
		"      <menuitem action=\"HelpAbout\" />"
		"    </menu>"
		"  </menubar>"
		"  <toolbar name=\"ToolMainBar\">"
		"    <toolitem action=\"FileNew\" />"
		"    <toolitem action=\"FileOpen\" />"
		"    <toolitem action=\"FileSave\" />"
//		"    <toolitem action=\"FileSaveAll\" />"
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
		if(language_ids == NULL) return Gobby::Header::LanguageMap();

		for(const gchar* const* id = language_ids; *id != NULL; ++ id)
		{
			GtkSourceLanguage* language =
				gtk_source_language_manager_get_language(
					manager, *id);
			if(gtk_source_language_get_hidden(language)) continue;

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
				"None"),
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

	m_ui_manager(Gtk::UIManager::create()),

	group_file(Gtk::ActionGroup::create("MenuFile") ),
	group_edit(Gtk::ActionGroup::create("MenuEdit") ),
	group_view(Gtk::ActionGroup::create("MenuView") ),
	group_help(Gtk::ActionGroup::create("MenuHelp") ),

	action_file(Gtk::Action::create("MenuFile", _("_File"))),
	action_file_new(Gtk::Action::create("FileNew", _("_New..."))),
	action_file_open(Gtk::Action::create("FileOpen", _("_Open..."))),
	action_file_open_location(
		Gtk::Action::create("FileOpenLocation",
		                    _("Open _Location..."))),
	action_file_save(Gtk::Action::create("FileSave", _("_Save"))),
	action_file_save_as(
		Gtk::Action::create("FileSaveAs", _("Save _As..."))),
	action_file_save_all(
		Gtk::Action::create(
			"FileSaveAll", _("Save All"),
			_("Save all open files locally"))),
	action_file_export_html(
		Gtk::Action::create("FileExportHtml",
		                    _("Export As _HTML..."))),
	action_file_connect(
		Gtk::Action::create("FileConnect",
		                    _("Connect _to Server..."))),
	action_file_close(
		Gtk::Action::create("FileClose", _("_Close"))),
	action_file_quit(Gtk::Action::create("FileQuit", _("_Quit"))),

	action_edit(Gtk::Action::create("MenuEdit", _("_Edit"))),
	action_edit_undo(Gtk::Action::create("EditUndo", _("_Undo"))),
	action_edit_redo(Gtk::Action::create("EditRedo", _("_Redo"))),
	action_edit_cut(Gtk::Action::create("EditCut", _("Cut"))),
	action_edit_copy(Gtk::Action::create("EditCopy", _("Copy"))),
	action_edit_paste(Gtk::Action::create("EditPaste", _("Paste"))),
	action_edit_find(Gtk::Action::create("EditFind", _("_Find..."))),
	action_edit_find_next(
		Gtk::Action::create("EditFindNext", _("Find Ne_xt"),
		                    _("Find next match of phrase "
		                      "searched for"))),
	action_edit_find_prev(
		Gtk::Action::create("EditFindPrev", _("Find Pre_vious"),
		                    _("Find previous match of phrase "
		                      "searched for"))),
	action_edit_find_replace(
		Gtk::Action::create("EditFindReplace",
		                    _("Find and Re_place..."))),
	action_edit_goto_line(
		Gtk::Action::create("EditGotoLine",
				    _("Go to _Line..."))),
	action_edit_preferences(
		Gtk::Action::create("EditPreferences",
		                    _("_Preferences..."))),

	action_view(Gtk::Action::create("MenuView", _("_View"))),
	action_view_hide_user_colors(
		Gtk::Action::create(
			"ViewHideUserColors", _("Reset User Colors"),
			_("Hide user colors in current document"))),
	action_view_fullscreen(
		Gtk::ToggleAction::create(
			"ViewFullscreen",
			_("Fullscreen"),
			_("Show the editing window in fullscreen mode"),
			false)),
	action_view_zoom_in(
		Gtk::Action::create("ViewZoomIn", _("Zoom In"))),
	action_view_zoom_out(
		Gtk::Action::create("ViewZoomOut", _("Zoom Out"))),
	action_view_toolbar(
		Gtk::ToggleAction::create(
			"ViewToolbar", _("View Toolbar"),
		        _("Whether to show the toolbar"),
			preferences.appearance.show_toolbar)),
	action_view_statusbar(
		Gtk::ToggleAction::create(
			"ViewStatusbar", _("View Statusbar"),
			_("Whether to show the statusbar"),
			preferences.appearance.show_statusbar)),
	action_view_browser(
		Gtk::ToggleAction::create(
			"ViewBrowser", _("View Document Browser"),
			_("Whether to show the document browser"),
			preferences.appearance.show_browser)),
	action_view_chat(
		Gtk::ToggleAction::create(
			"ViewChat", _("View Chat"),
			_("Whether to show the chat pane"),
			preferences.appearance.show_chat)),
	action_view_document_userlist(
		Gtk::ToggleAction::create(
			"ViewDocumentUserList", _("View Document User List"),
			_("Whether to show the user list for documents"),
			preferences.appearance.show_document_userlist)),
	action_view_chat_userlist(
		Gtk::ToggleAction::create(
			"ViewChatUserList", _("View Chat User List"),
			_("Whether to show the user list for the chat"),
			preferences.appearance.show_chat_userlist)),
	action_view_highlight_mode(
		Gtk::Action::create("ViewHighlightMode",
		                    _("_Highlight Mode"))),
	action_view_highlight_none(
		LanguageAction::create(NULL, m_highlight_group)),
	action_view_highlight_languages(
		load_highlight_languages(lang_mgr, m_highlight_group)),

	action_help(Gtk::Action::create("MenuHelp", _("_Help")) ),
	action_help_contents(
		Gtk::Action::create(
			"HelpContents", _("_Contents"),
			_("Opens the Gobby manual"))),
	action_help_about(
		Gtk::Action::create(
			"HelpAbout", _("_About..."),
			_("Shows Gobby's copyright and credits")))
{
	action_file_new->set_icon_name("document-new");
	action_file_open->set_icon_name("document-open");
	action_file_save->set_icon_name("document-save");
	action_file_save_as->set_icon_name("document-save-as");
	action_file_connect->set_icon_name("network-server");
	action_file_close->set_icon_name("window-close");
	action_file_quit->set_icon_name("application-quit");
	action_edit_undo->set_icon_name("edit-undo");
	action_edit_redo->set_icon_name("edit-redo");
	action_edit_cut->set_icon_name("edit-cut");
	action_edit_copy->set_icon_name("edit-copy");
	action_edit_paste->set_icon_name("edit-paste");
	action_edit_find->set_icon_name("edit-find");
	action_edit_find_replace->set_icon_name("edit-find-replace");
	action_edit_goto_line->set_icon_name("go-jump");
	action_edit_preferences->set_icon_name("preferences-system");
	action_view_fullscreen->set_icon_name("view-fullscreen");
	action_view_zoom_in->set_icon_name("zoom-in");
	action_view_zoom_out->set_icon_name("zoom-out");
	action_help_contents->set_icon_name("help-contents");
	action_help_about->set_icon_name("help-about");

	// Add basic menu
	m_ui_manager->add_ui_from_string(ui_desc);

	// Set all toolbar actions to be "important", so that their text
	// shows up in the toolbar when the toolbar style is set to
	// GTK_TOOLBAR_STYLE_BOTH_HORIZ.
	action_file_new->set_is_important(true);
	action_file_open->set_is_important(true);
	action_file_save->set_is_important(true);
	action_edit_undo->set_is_important(true);
	action_edit_redo->set_is_important(true);
	action_edit_cut->set_is_important(true);
	action_edit_copy->set_is_important(true);
	action_edit_paste->set_is_important(true);
	action_edit_find->set_is_important(true);
	action_edit_find_replace->set_is_important(true);

	group_file->add(action_file);
	group_file->add(action_file_new,
	                Gtk::AccelKey("<control>N",
			              "<Actions>/MenuFile/FileNew"));
	group_file->add(action_file_open,
	                Gtk::AccelKey("<control>O",
			              "<Actions>/MenuFile/FileOpen"));
	group_file->add(action_file_open_location,
	                Gtk::AccelKey("<control>L",
			              "<Actions>/MenuFile/FileOpenLocation"));
	group_file->add(action_file_save,
	                Gtk::AccelKey("<control>S",
	                              "<Actions>/MenuFile/FileSave"));
	group_file->add(action_file_save_as,
	                Gtk::AccelKey("<control><shift>S",
			              "<Actions>/MenuFile/FileSaveAs"));
	group_file->add(action_file_save_all,
	                Gtk::AccelKey("<control><shift>L",
			              "<Actions>/MenuFile/FileSaveAll"));
	group_file->add(action_file_export_html,
	                Gtk::AccelKey("<control><shift>H",
			              "<Actions>/MenuFile/FileExportHtml"));
	group_file->add(action_file_connect,
	                Gtk::AccelKey("<control>T",
	                              "<Actions>/MenuFile/FileConnect"));
	group_file->add(action_file_close,
	                Gtk::AccelKey("<control>W",
			              "<Actions>/MenuFile/FileClose"));
	group_file->add(action_file_quit,
	                Gtk::AccelKey("<control>Q",
			              "<Actions>/MenuFile/FileQuit"));

	group_edit->add(action_edit);
	group_edit->add(action_edit_undo,
	                Gtk::AccelKey("<control>Z",
			              "<Actions>/MenuEdit/EditUndo"));
	group_edit->add(action_edit_redo,
	                Gtk::AccelKey("<control><shift>Z",
			              "<Actions>/MenuEdit/EditRedo"));
	group_edit->add(action_edit_cut,
	                Gtk::AccelKey("<control>X",
			              "<Actions>/MenuFile/EditCut"));
	group_edit->add(action_edit_copy,
	                Gtk::AccelKey("<control>C",
			              "<Actions>/MenuFile/EditCopy"));
	group_edit->add(action_edit_paste,
	                Gtk::AccelKey("<control>V",
			              "<Actions>/MenuFile/EditPaste"));
	group_edit->add(action_edit_find,
	                Gtk::AccelKey("<control>F",
			              "<Actions>/MenuFile/EditFind"));
	group_edit->add(action_edit_find_next,
	                Gtk::AccelKey("<control>G",
	                              "<Actions>/MenuEdit/EditFindNext"));
	group_edit->add(action_edit_find_prev,
	                Gtk::AccelKey("<control><shift>G",
			              "<Actions>/MenuEdit/EditFindPrev"));
	group_edit->add(action_edit_find_replace,
	                Gtk::AccelKey("<control>H",
			              "<Actions>/MenuEdit/EditFindReplace"));
	group_edit->add(action_edit_goto_line,
	                Gtk::AccelKey("<control>I",
	                              "<Actions>/MenuEdit/EditGotoLine"));
	group_edit->add(action_edit_preferences);

	group_view->add(action_view);
	group_view->add(
		action_view_hide_user_colors,
		Gtk::AccelKey("<shift><control>C",
		              "<Actions>/MenuView/ViewHideUserColors"));
	group_view->add(action_view_fullscreen, Gtk::AccelKey(
		"F11", "<Actions>/MenuView/ViewFullscreen"));
	group_view->add(
		action_view_zoom_in,
		Gtk::AccelKey('+', Gdk::CONTROL_MASK,
		              "<Actions>/MenuView/ViewZoomIn"));
	group_view->add(
		action_view_zoom_out,
		Gtk::AccelKey('-', Gdk::CONTROL_MASK,
		              "<Actions>/MenuView/ViewZoomOut"));
	group_view->add(action_view_toolbar);
	group_view->add(action_view_statusbar);
	group_view->add(action_view_browser, Gtk::AccelKey(
		"F9", "<Actions>/MenuView/ViewBrowser"));
	group_view->add(action_view_chat, Gtk::AccelKey(
		"<shift>F9", "<Actions>/MenuView/ViewChat"));
	group_view->add(action_view_document_userlist, Gtk::AccelKey(
		"<control>F9", "<Actions>/MenuView/ViewDocumentUserList"));
	group_view->add(action_view_chat_userlist, Gtk::AccelKey(
		"<control><shift>F9", "<Actions>/MenuView/ViewChatUserList"));
	group_view->add(action_view_highlight_mode);
	group_view->add(action_view_highlight_none);

	unsigned int language_menu_counter = 0;
	for(LanguageMap::const_iterator iter =
		action_view_highlight_languages.begin();
	    iter != action_view_highlight_languages.end();
	    ++ iter)
	{
		// We cannot use iter->first for the action name, since this
		// is a localized string.
		Glib::ustring section_action_name =
 			Glib::ustring::compose("ViewHighlightModeSection_%1",
				language_menu_counter);
		++language_menu_counter;

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
	group_help->add(action_help_contents,
	                Gtk::AccelKey("F1",
			              "<Actions>/MenuHelp/HelpContents"));

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

#ifdef PLATFORM_OSX_NATIVE
	ige_mac_menu_set_menu_bar(GTK_MENU_SHELL(m_menubar->gobj()));

	ige_mac_menu_set_quit_menu_item(GTK_MENU_ITEM(
		m_ui_manager->get_widget(
			"/MenuMainBar/MenuFile/FileQuit")->gobj()));

	ige_mac_menu_add_app_menu_item(
		ige_mac_menu_add_app_menu_group(), GTK_MENU_ITEM(
			m_ui_manager->get_widget(
				"/MenuMainBar/MenuHelp/HelpAbout")->gobj()),
		NULL);

	 ige_mac_menu_add_app_menu_item(
	 	ige_mac_menu_add_app_menu_group(), GTK_MENU_ITEM(
			m_ui_manager->get_widget(
				"/MenuMainBar/MenuEdit/EditPreferences")
			->gobj()),
		NULL);
#else
	pack_start(*m_menubar, Gtk::PACK_SHRINK);
#endif
	pack_start(*m_toolbar, Gtk::PACK_SHRINK);

	m_toolbar->set_toolbar_style(preferences.appearance.toolbar_style);
	m_menubar->show();
	if(preferences.appearance.show_toolbar) m_toolbar->show();
	else m_toolbar->hide();

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

Glib::RefPtr<Gobby::Header::LanguageAction>
Gobby::Header::lookup_language_action(GtkSourceLanguage* language)
{
	const Glib::ustring section(
		gtk_source_language_get_section(language));
	LanguageMap::const_iterator iter =
		action_view_highlight_languages.find(section);
	g_assert(iter != action_view_highlight_languages.end());

	const LanguageList& list = iter->second;
	for(LanguageList::const_iterator iter = list.begin();
	    iter != list.end(); ++ iter)
	{
		const Glib::RefPtr<LanguageAction> action = *iter;
		if(action->get_language() == language)
			return action;
	}

	g_assert_not_reached();
	return Glib::RefPtr<LanguageAction>();
}
