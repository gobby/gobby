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

namespace {
	Glib::ustring ui_desc = 
		"<ui>"
		"  <menubar name=\"MenuMainBar\">"
		"    <menu action=\"MenuApp\">"
		"      <menuitem action=\"AppSessionCreate\" />"
		"      <menuitem action=\"AppSessionJoin\" />"
		"      <menuitem action=\"AppSessionSave\" />"
		"      <menuitem action=\"AppSessionQuit\" />"
		"      <separator />"
		"      <menuitem action=\"AppQuit\" />"
		"    </menu>"
		"    <menu action=\"MenuSession\">"
		"      <menuitem action=\"SessionDocumentCreate\" />"
		"      <menuitem action=\"SessionDocumentOpen\" />"
		"      <menuitem action=\"SessionDocumentSave\" />"
		"      <menuitem action=\"SessionDocumentSaveAs\" />"
		"      <menuitem action=\"SessionDocumentClose\" />"
		"    </menu>"
		"    <menu action=\"MenuEdit\">"
		"      <menuitem action=\"EditSearch\" />"
		"      <menuitem action=\"EditSearchReplace\" />"
		"      <separator />"
		"      <menuitem action=\"EditGotoLine\" />"
		"      <separator />"
		"      <menuitem action=\"EditPreferences\" />"
		"    </menu>"
		"    <menu action=\"MenuUser\">"
		"      <menuitem action=\"UserSetPassword\" />"
		"      <menuitem action=\"UserSetColour\" />"
		"    </menu>"
		"    <menu action=\"MenuView\">"
		"      <menuitem action=\"ViewPreferences\" />"
		"      <separator />"
		"      <menu action=\"MenuViewSyntax\">"
		"        <menuitem action=\"ViewSyntaxLanguageNone\" />"
		"      </menu>"
		"    </menu>"
		"    <menu action=\"MenuWindow\">"
		"      <menuitem action=\"WindowUserList\" />"
		"      <menuitem action=\"WindowDocumentList\" />"
		"      <menuitem action=\"WindowChat\" />"
		"    </menu>"
		"    <menu action=\"MenuHelp\">"
		"      <menuitem action=\"HelpAbout\" />"
		"    </menu>"
		"  </menubar>"
		"  <toolbar name=\"ToolMainBar\">"
		"    <toolitem action=\"AppSessionCreate\" />"
		"    <toolitem action=\"AppSessionJoin\" />"
		"    <toolitem action=\"AppSessionQuit\" />"
		"    <separator />"
		"    <toolitem action=\"SessionDocumentCreate\" />"
		"    <toolitem action=\"SessionDocumentOpen\" />"
		"    <toolitem action=\"SessionDocumentSave\" />"
		"    <toolitem action=\"SessionDocumentClose\" />"
		"    <separator />"
		"    <toolitem action=\"WindowUserList\" />"
		"    <toolitem action=\"WindowDocumentList\" />"
		"    <toolitem action=\"WindowChat\" />"
		"  </toolbar>"
		"</ui>";

	/** Replaces dangerous characters for an XML attribute by their
	 * Unicode value.
	 */
	void remove_dangerous_xml(Glib::ustring& string)
	{
		for(Glib::ustring::iterator iter = string.begin();
		    iter != string.end();
		    ++ iter)
		{
			// Get current character
			gunichar c = *iter;

			// Not an ASCII character, or a dangerous one?
			if(c == '<' || c == '>' || c == '\"' || c > 0x7f)
			{
				// Get next iter to find the end position
				Glib::ustring::iterator next = iter;
				++ next;

				// Build value string
				std::stringstream value_stream;
				value_stream << c;

				// Erase dangerous character
				iter = string.erase(iter, next);

				// Insert string char by char to keep the
				// iterator valid.
				char cval;
				while(value_stream >> cval)
					iter = string.insert(iter, cval);
			}
		}
	}

	/** Callback function for std::list::sort to sort the languages by
	 * their name.
	 */
	bool language_sort_callback(
		const Glib::RefPtr<Gtk::SourceLanguage>& lang1,
		const Glib::RefPtr<Gtk::SourceLanguage>& lang2
	)
	{
		return lang1->get_name() < lang2->get_name();
	}
}

Gobby::Header::LanguageWrapper::LanguageWrapper(Action action,
                                                Language language):
	m_action(action), m_language(language)
{
}

Gobby::Header::LanguageWrapper::Action
Gobby::Header::LanguageWrapper::get_action() const
{
	return m_action;
}

Gobby::Header::LanguageWrapper::Language
Gobby::Header::LanguageWrapper::get_language() const
{
	return m_language;
}

Gobby::Header::AutoAction::AutoAction(action_type action,
                                      const ApplicationState& state,
                                      ApplicationFlags inc_flags,
                                      ApplicationFlags exc_flags):
	m_action(action), m_inc_flags(inc_flags), m_exc_flags(exc_flags)
{
	state.state_changed_event().connect(
		sigc::bind(
			sigc::hide(
				sigc::mem_fun(
					*this,
					&AutoAction::on_state_change
				)
			),
			sigc::ref(state)
		)
	);
}

void Gobby::Header::AutoAction::on_state_change(const ApplicationState& state)
{
	m_action->set_sensitive(state.query(m_inc_flags, m_exc_flags) );
}

void Gobby::Header::AutoList::add(action_type action,
                                  const ApplicationState& state,
                                  ApplicationFlags inc_flags,
                                  ApplicationFlags exc_flags)
{
	m_list.push_back(new AutoAction(action, state, inc_flags, exc_flags) );
}

Gobby::Header::AutoList::~AutoList()
{
	for(std::list<AutoAction*>::iterator iter = m_list.begin();
	    iter != m_list.end();
	    ++ iter)
	{
		delete *iter;
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

#include <iostream>
Gobby::Header::Header(const ApplicationState& state,
                      const LangManager& lang_mgr):
	group_app(Gtk::ActionGroup::create("MenuApp") ),
	group_session(Gtk::ActionGroup::create("MenuSession") ),
	group_edit(Gtk::ActionGroup::create("MenuEdit") ),
	group_user(Gtk::ActionGroup::create("MenuUser") ),
	group_view(Gtk::ActionGroup::create("MenuView") ),
	group_window(Gtk::ActionGroup::create("MenuWindow") ),
	group_help(Gtk::ActionGroup::create("MenuHelp") ),

	action_app(Gtk::Action::create("MenuApp", "_Gobby") ),
	action_app_session_create(
		Gtk::Action::create(
			"AppSessionCreate",
			Gtk::Stock::NETWORK,
			_("Create session..."),
			_("Opens a new obby session")
		)
	),

	action_app_session_join(
		Gtk::Action::create(
			"AppSessionJoin",
			Gtk::Stock::CONNECT,
			_("Join session..."),
			_("Joins an existing obby session")
		)
	),

	action_app_session_save(
		Gtk::Action::create(
			"AppSessionSave",
			Gtk::Stock::SAVE,
			_("Save session..."),
			_("Saves the complete session for a later restore")
		)
	),

	action_app_session_quit(
		Gtk::Action::create(
			"AppSessionQuit",
			Gtk::Stock::DISCONNECT,
			_("Quit session"),
			_("Leaves the currently running obby session")
		)
	),

	action_app_quit(
		Gtk::Action::create(
			"AppQuit",
			Gtk::Stock::QUIT,
			_("Quit"),
			_("Quits the application")
		)
	),

	action_session(Gtk::Action::create("MenuSession", _("_Session")) ),

	action_session_document_create(
		Gtk::Action::create(
			"SessionDocumentCreate",
			Gtk::Stock::NEW,
			_("Create document..."),
			_("Creates a new document")
		)
	),

	action_session_document_open(
		Gtk::Action::create(
			"SessionDocumentOpen",
			Gtk::Stock::OPEN,
			_("Open document..."),
			_("Loads a file into a new document")
		)
	),

	action_session_document_save(
		Gtk::Action::create(
			"SessionDocumentSave",
			Gtk::Stock::SAVE,
			_("Save document"),
			_("Saves a document into a file")
		)
	),

	action_session_document_save_as(
		Gtk::Action::create(
			"SessionDocumentSaveAs",
			Gtk::Stock::SAVE_AS,
			_("Save document as..."),
			_("Saves a document to another location")
		)
	),

	action_session_document_close(
		Gtk::Action::create(
			"SessionDocumentClose",
			Gtk::Stock::CLOSE,
			_("Close document"),
			_("Closes an opened document")
		)
	),

	action_edit(Gtk::Action::create("MenuEdit", _("_Edit")) ),

	action_edit_search(
		Gtk::Action::create(
			"EditSearch",
			Gtk::Stock::FIND,
			_("Find..."),
			_("Search for a text in the current document")
		)
	),

	action_edit_search_replace(
		Gtk::Action::create(
			"EditSearchReplace",
			Gtk::Stock::FIND_AND_REPLACE,
			_("Find and replace..."),
			_("Search for a text and replace it with another one")
		)
	),

	action_edit_goto_line(
		Gtk::Action::create(
			"EditGotoLine",
			Gtk::Stock::JUMP_TO,
			_("Go to line..."),
			_("Move cursor to a specified line")
		)
	),

	action_edit_preferences(
		Gtk::Action::create(
			"EditPreferences",
			Gtk::Stock::PREFERENCES,
			_("Preferences..."),
			_("Displays a dialog to customise Gobby for your needs")
		)
	),

	action_user(Gtk::Action::create("MenuUser", _("_User")) ),

	action_user_set_password(
		Gtk::Action::create(
			"UserSetPassword",
			Gtk::Stock::DIALOG_AUTHENTICATION,
			_("Set password..."),
			_("Sets a password for this user")
		)
	),

	action_user_set_colour(
		Gtk::Action::create(
			"UserSetColour",
			Gtk::Stock::SELECT_COLOR,
			_("Set colour..."),
			_("Sets a new colour for this user")
		)
	),

	action_view(Gtk::Action::create("MenuView", _("_View")) ),

	action_view_preferences(
		Gtk::Action::create(
			"ViewPreferences",
			Gtk::Stock::PREFERENCES,
			_("Document preferences..."),
			_("Shows a preferences dialog that is just applied to "
			  "this document")
		)
	),

	action_view_syntax(Gtk::Action::create("MenuViewSyntax", _("Syntax")) ),

	action_window(Gtk::Action::create("MenuWindow", _("_Window")) ),

	action_window_userlist(
		Gtk::ToggleAction::create(
			"WindowUserList",
			IconManager::STOCK_USERLIST,
			_("User list"),
			_("Displays a list of users that are currently joined")
		)
	),

	action_window_documentlist(
		Gtk::ToggleAction::create(
			"WindowDocumentList",
			IconManager::STOCK_DOCLIST,
			_("Document list"),
			_("Displays a list of documents within the "
			  "current session")
		)
	),

	action_window_chat(
		Gtk::ToggleAction::create(
			"WindowChat",
			IconManager::STOCK_CHAT,
			_("Chat"),
			_("Displays a chat to talk to other people in "
			  "the session")
		)
	),

	action_help(Gtk::Action::create("MenuHelp", _("_Help")) ),

	action_help_about(
		Gtk::Action::create(
			"HelpAbout",
			Gtk::Stock::ABOUT,
			_("About"),
			_("Shows Gobby's copyright and credits")
		)
	),

	m_ui_manager(Gtk::UIManager::create() )
{
	// Assign auto actions
	set_action_auto(
		action_app, state,
		APPLICATION_NONE, APPLICATION_NONE
	);

	set_action_auto(
		action_app_session_create, state,
		APPLICATION_NONE, APPLICATION_SESSION
	);

	set_action_auto(
		action_app_session_join, state,
		APPLICATION_NONE, APPLICATION_SESSION
	);

	set_action_auto(
		action_app_session_save, state,
		APPLICATION_NONE, APPLICATION_INITIAL
	);

	set_action_auto(
		action_app_session_quit, state,
		APPLICATION_SESSION, APPLICATION_NONE
	);

	set_action_auto(
		action_app_quit, state,
		APPLICATION_NONE, APPLICATION_NONE
	);

	set_action_auto(
		action_session, state,
		APPLICATION_NONE, APPLICATION_INITIAL
	);

	set_action_auto(
		action_session_document_create, state,
		APPLICATION_SESSION, APPLICATION_NONE
	);

	set_action_auto(
		action_session_document_open, state,
		APPLICATION_SESSION, APPLICATION_NONE
	);

	set_action_auto(
		action_session_document_save, state,
		APPLICATION_DOCUMENT, APPLICATION_NONE
	);

	set_action_auto(
		action_session_document_save_as, state,
		APPLICATION_DOCUMENT, APPLICATION_NONE
	);

	set_action_auto(
		action_session_document_close, state,
		APPLICATION_DOCUMENT, APPLICATION_NONE
	);

	set_action_auto(
		action_edit, state,
		APPLICATION_NONE, APPLICATION_NONE
	);

	set_action_auto(
		action_edit_search, state,
		APPLICATION_NONE, APPLICATION_NONE
	);

	set_action_auto(
		action_edit_search_replace, state,
		APPLICATION_NONE, APPLICATION_NONE
	);

	set_action_auto(
		action_edit_goto_line, state,
		APPLICATION_NONE, APPLICATION_NONE
	);

	set_action_auto(
		action_edit_preferences, state,
		APPLICATION_NONE, APPLICATION_NONE
	);

	set_action_auto(
		action_user, state,
		APPLICATION_SESSION, APPLICATION_NONE
	);

	set_action_auto(
		action_user_set_password, state,
		APPLICATION_SESSION, APPLICATION_HOST
	);

	set_action_auto(
		action_user_set_colour, state,
		APPLICATION_SESSION, APPLICATION_NONE
	);

	set_action_auto(
		action_view, state,
		APPLICATION_DOCUMENT, APPLICATION_NONE
	);

	set_action_auto(
		action_view_preferences, state,
		APPLICATION_DOCUMENT, APPLICATION_NONE
	);

	set_action_auto(
		action_view_syntax, state,
		APPLICATION_DOCUMENT, APPLICATION_NONE
	);

	set_action_auto(
		action_window, state,
		APPLICATION_NONE, APPLICATION_NONE
	);

	set_action_auto(
		action_window_userlist, state,
		APPLICATION_NONE, APPLICATION_NONE
	);

	set_action_auto(
		action_window_documentlist, state,
		APPLICATION_NONE, APPLICATION_NONE
	);

	set_action_auto(
		action_window_chat, state,
		APPLICATION_NONE, APPLICATION_NONE
	);

	set_action_auto(
		action_help, state,
		APPLICATION_NONE, APPLICATION_NONE
	);

	set_action_auto(
		action_help_about, state,
		APPLICATION_NONE, APPLICATION_NONE
	);

	// Add basic menu
	m_ui_manager->add_ui_from_string(ui_desc);

	group_app->add(action_app);
	group_app->add(action_app_session_create);
	group_app->add(action_app_session_join);
	group_app->add(action_app_session_save, Gtk::AccelKey("") );
	group_app->add(action_app_session_quit);
	group_app->add(action_app_quit);

	group_session->add(action_session);
	group_session->add(action_session_document_create);
	group_session->add(action_session_document_open);
	group_session->add(action_session_document_save);
	group_session->add(action_session_document_save_as);
	group_session->add(action_session_document_close);

	group_edit->add(action_edit);
	group_edit->add(action_edit_search);
	group_edit->add(action_edit_search_replace);
	group_edit->add(
		action_edit_goto_line,
		Gtk::AccelKey("<control>I", "<Actions>/MenuEdit/EditGotoLine")
	);

	group_edit->add(action_edit_preferences);

	group_user->add(action_user);
	group_user->add(action_user_set_password);
	group_user->add(action_user_set_colour);

	group_view->add(action_view);
	group_view->add(action_view_preferences);
	group_view->add(action_view_syntax);

	group_window->add(action_window);
	group_window->add(action_window_userlist);
	group_window->add(action_window_documentlist);
	group_window->add(action_window_chat);

	group_help->add(action_help);
	group_help->add(action_help_about);

	// A kind of hack to ensure that
	// Gtk::SourceLanguage::sourcelanguage_class_.init() is called.
	// See the TODO item in Glib::wrap(GtkSourceLanguage*, bool) in
	// sourcelanguage.cpp
	//GtkSourceLanguage* lang = NULL;
	//Glib::wrap(lang, false);

	// Get available languages
	std::list<Glib::RefPtr<Gtk::SourceLanguage> > lang_list =
		lang_mgr->get_available_languages();

	// Sort languages by name
	lang_list.sort(&language_sort_callback);

	// Add None-Language
	Glib::RefPtr<Gtk::RadioAction> action = Gtk::RadioAction::create(
		m_lang_group,
		"ViewSyntaxLanguageNone",
		_("None"),
		_("Unselects the current language")
	);

	group_view->add(action);
	action_view_syntax_languages.push_back(
		LanguageWrapper(
			action,
			Glib::RefPtr<Gtk::SourceLanguage>(NULL)
		)
	);

	// Add languages
	for(std::list<Glib::RefPtr<Gtk::SourceLanguage> >::const_iterator iter =
		lang_list.begin();
	    iter != lang_list.end();
	    ++ iter)
	{
		// Get current language 
		Glib::RefPtr<Gtk::SourceLanguage> language = *iter;
		Glib::ustring language_xml_name = language->get_name();

		// Build description string
		obby::format_string str(_("Selects %0% as language") );
		Glib::ustring name = language->get_name();
		str << name.raw();

		// Add language to action group
		remove_dangerous_xml(language_xml_name);
		action = Gtk::RadioAction::create(
			m_lang_group,
			"ViewSyntaxLanguage" + language_xml_name,
			language->get_name(),
			str.str()
		);

		group_view->add(action);
		action_view_syntax_languages.push_back(
			LanguageWrapper(action, language)
		);

		// Add menu item to UI
		Glib::ustring xml_desc =
			"<ui>"
			"  <menubar name=\"MenuMainBar\">"
			"    <menu action=\"MenuView\">"
			"      <menu action=\"MenuViewSyntax\">"
			"	 <menuitem action=\"ViewSyntaxLanguage"
				 + language_xml_name + "\" />"
			"      </menu>"
			"    </menu>"
			"  </menubar>"
			"</ui>";

		m_ui_manager->add_ui_from_string(xml_desc);
	}

	m_ui_manager->insert_action_group(group_app);
	m_ui_manager->insert_action_group(group_session);
	m_ui_manager->insert_action_group(group_edit);
	m_ui_manager->insert_action_group(group_user);
	m_ui_manager->insert_action_group(group_view);
	m_ui_manager->insert_action_group(group_window);
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
}

Glib::RefPtr<Gtk::AccelGroup> Gobby::Header::get_accel_group()
{
	return m_ui_manager->get_accel_group();
}

Glib::RefPtr<const Gtk::AccelGroup> Gobby::Header::get_accel_group() const
{
	return m_ui_manager->get_accel_group();
}

#if 0
Glib::RefPtr<Gtk::SourceLanguagesManager> Gobby::Header::get_lang_manager()
{
	return m_lang_manager;
}

Glib::RefPtr<const Gtk::SourceLanguagesManager>
Gobby::Header::get_lang_manager() const
{
	return m_lang_manager;
}
#endif

Gtk::MenuBar& Gobby::Header::get_menubar()
{
	return *m_menubar;
}

Gtk::Toolbar& Gobby::Header::get_toolbar()
{
	return *m_toolbar;
}

void Gobby::Header::set_action_auto(const Glib::RefPtr<Gtk::Action>& action,
                                    const ApplicationState& state,
                                    ApplicationFlags inc_flags,
                                    ApplicationFlags exc_flags)
{
	m_auto_actions.add(action, state, inc_flags, exc_flags);
}

