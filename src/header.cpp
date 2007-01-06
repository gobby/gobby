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
//#include <gtkmm/toggleaction.h>
#include <obby/format_string.hpp>
//#include <obby/local_buffer.hpp>
//#include <obby/client_buffer.hpp>

//#include "features.hpp"
#include "common.hpp"
#include "header.hpp"

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

Gobby::Header::Error::Error(Code error_code, const Glib::ustring& error_message)
 : Glib::Error(g_quark_from_static_string("GOBBY_HEADER_ERROR"),
               static_cast<int>(error_code), error_message)
{
}

Gobby::Header::Error::Code Gobby::Header::Error::code() const
{
	return static_cast<Code>(gobject_->code);
}

Gobby::Header::Header():
	group_app(Gtk::ActionGroup::create() ),
	group_session(Gtk::ActionGroup::create() ),
	group_edit(Gtk::ActionGroup::create() ),
	group_user(Gtk::ActionGroup::create() ),
	group_view(Gtk::ActionGroup::create() ),
	group_help(Gtk::ActionGroup::create() ),

	action_app(Gtk::Action::create("MenuApp", "Gobby") ),
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

	action_session(Gtk::Action::create("MenuSession", _("Session")) ),

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

	action_edit(Gtk::Action::create("MenuEdit", _("Edit")) ),

	action_edit_preferences(
		Gtk::Action::create(
			"EditPreferences",
			Gtk::Stock::PREFERENCES,
			_("Preferences..."),
			_("Shows up a dialog to customise Gobby for your needs")
		)
	),

	action_user(Gtk::Action::create("MenuUser", _("User")) ),

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
			_("Setsa new colour for this user")
		)
	),

	action_view(Gtk::Action::create("MenuView", _("View")) ),

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

	action_help(Gtk::Action::create("MenuHelp", _("Help")) ),

	action_help_about(
		Gtk::Action::create(
			"HelpAbout",
			Gtk::Stock::ABOUT,
			_("About"),
			_("Shows Gobby's copyright and credits")
		)
	),

	m_ui_manager(Gtk::UIManager::create() ),
	m_lang_manager(Gtk::SourceLanguagesManager::create() )
{
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
	group_edit->add(action_edit_preferences);

	group_user->add(action_user);
	group_user->add(action_user_set_password);
	group_user->add(action_user_set_colour);

	group_view->add(action_view);
	group_view->add(action_view_preferences);
	group_view->add(action_view_syntax);

	group_help->add(action_help);
	group_help->add(action_help_about);

	// A kind of hack to ensure that
	// Gtk::SourceLanguage::sourcelanguage_class_.init() is called.
	// See the TODO item in Glib::wrap(GtkSourceLanguage*, bool) in
	// sourcelanguage.cpp
	GtkSourceLanguage* lang = NULL;
	Glib::wrap(lang, false);

	// Get available languages
	std::list<Glib::RefPtr<Gtk::SourceLanguage> > lang_list =
		m_lang_manager->get_available_languages();

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
		str << language->get_name().raw();

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

Glib::RefPtr<Gtk::SourceLanguagesManager> Gobby::Header::get_lang_manager()
{
	return m_lang_manager;
}

Glib::RefPtr<const Gtk::SourceLanguagesManager>
Gobby::Header::get_lang_manager() const
{
	return m_lang_manager;
}

Gtk::MenuBar& Gobby::Header::get_menubar()
{
	return *m_menubar;
}

Gtk::Toolbar& Gobby::Header::get_toolbar()
{
	return *m_toolbar;
}

#if 0
void Gobby::Header::disable_document_actions()
{
	m_group_app->get_action("SaveDocument")->set_sensitive(false);
	m_group_app->get_action("SaveAsDocument")->set_sensitive(false);
	m_group_app->get_action("CloseDocument")->set_sensitive(false);
	m_group_app->get_action("MenuView")->set_sensitive(false);
}

Gobby::Header::signal_session_create_type
Gobby::Header::session_create_event() const
{
	return m_signal_session_create;
}

Gobby::Header::signal_session_join_type
Gobby::Header::session_join_event() const
{
	return m_signal_session_join;
}

Gobby::Header::signal_session_save_type
Gobby::Header::session_save_event() const
{
	return m_signal_session_save;
}

Gobby::Header::signal_session_quit_type
Gobby::Header::session_quit_event() const
{
	return m_signal_session_quit;
}

Gobby::Header::signal_document_create_type
Gobby::Header::document_create_event() const
{
	return m_signal_document_create;
}

Gobby::Header::signal_document_open_type
Gobby::Header::document_open_event() const
{
	return m_signal_document_open;
}

Gobby::Header::signal_document_save_type
Gobby::Header::document_save_event() const
{
	return m_signal_document_save;
}

Gobby::Header::signal_document_save_as_type
Gobby::Header::document_save_as_event() const
{
	return m_signal_document_save_as;
}

Gobby::Header::signal_document_close_type
Gobby::Header::document_close_event() const
{
	return m_signal_document_close;
}

Gobby::Header::signal_edit_preferences_type
Gobby::Header::edit_preferences_event() const
{
	return m_signal_edit_preferences;
}

Gobby::Header::signal_user_set_password_type
Gobby::Header::user_set_password_event() const
{
	return m_signal_user_set_password;
}

Gobby::Header::signal_user_set_colour_type
Gobby::Header::user_set_colour_event() const
{
	return m_signal_user_set_colour;
}

Gobby::Header::signal_view_preferences_type
Gobby::Header::view_preferences_event() const
{
	return m_signal_view_preferences;
}

Gobby::Header::signal_view_language_type
Gobby::Header::view_language_event() const
{
	return m_signal_view_language;
}

Gobby::Header::signal_about_type
Gobby::Header::about_event() const
{
	return m_signal_about;
}

Gobby::Header::signal_quit_type
Gobby::Header::quit_event() const
{
	return m_signal_quit;
}

void Gobby::Header::obby_start(obby::local_buffer& buf)
{
	// Begin of obby session: Disable create/join buttons, enable quit
	m_group_app->get_action("CreateSession")->set_sensitive(false);
	m_group_app->get_action("JoinSession")->set_sensitive(false);
	m_group_app->get_action("SaveSession")->set_sensitive(true);
	m_group_app->get_action("QuitSession")->set_sensitive(true);

	// Enable document buttons
	m_group_app->get_action("CreateDocument")->set_sensitive(true);
	m_group_app->get_action("OpenDocument")->set_sensitive(true);

	// Enable password button if we are client
	m_group_app->get_action("MenuUser")->set_sensitive(true);
	m_group_app->get_action("UserSetPassword")->set_sensitive(
		dynamic_cast<obby::client_buffer*>(&buf) != NULL);

	// Document actions will be activated from the insert_document event
	m_group_app->get_action("SaveDocument")->set_sensitive(false);
	m_group_app->get_action("SaveAsDocument")->set_sensitive(false);
	m_group_app->get_action("CloseDocument")->set_sensitive(false);
	m_group_app->get_action("MenuView")->set_sensitive(false);
}

void Gobby::Header::obby_end()
{
	// End of obby session: Enable create/join buttons, disable quit
	// Disable save session button for now. It will stay enabled later if
	// the window keeps the obby buffer reference.
	m_group_app->get_action("CreateSession")->set_sensitive(true);
	m_group_app->get_action("JoinSession")->set_sensitive(true);
	m_group_app->get_action("SaveSession")->set_sensitive(false);
	m_group_app->get_action("QuitSession")->set_sensitive(false);

	// Disable user buttons
	m_group_app->get_action("MenuUser")->set_sensitive(false);

	// Disable document buttons
	m_group_app->get_action("CreateDocument")->set_sensitive(false);
	m_group_app->get_action("OpenDocument")->set_sensitive(false);

	// Leave document actions like SaveDocument and CloseDocument as they
	// were to allow the user to save documents.
}

void Gobby::Header::obby_user_join(const obby::user& user)
{
}

void Gobby::Header::obby_user_part(const obby::user& user)
{
}

void Gobby::Header::obby_document_insert(obby::local_document_info& document)
{
	// Now we have at least one document open, so we could activate the
	// document actions.
	m_group_app->get_action("SaveDocument")->set_sensitive(true);
	m_group_app->get_action("SaveAsDocument")->set_sensitive(true);
	//m_group_app->get_action("CloseDocument")->set_sensitive(true);
	m_group_app->get_action("MenuView")->set_sensitive(true);
}

void Gobby::Header::obby_document_remove(obby::local_document_info& document)
{
	if(document.get_buffer().document_count() == 1)
	{
		// The document which is currently removed is the only
		// existing document? Disable document actions then.
		m_group_app->get_action("SaveDocument")->set_sensitive(false);
		m_group_app->get_action("SaveAsDocument")->set_sensitive(false);
		m_group_app->get_action("CloseDocument")->set_sensitive(false);
		m_group_app->get_action("MenuView")->set_sensitive(false);
	}
}

void Gobby::Header::on_app_session_create()
{
	m_signal_session_create.emit();
}

void Gobby::Header::on_app_session_join()
{
	m_signal_session_join.emit();
}

void Gobby::Header::on_app_session_save()
{
	m_signal_session_save.emit();
}

void Gobby::Header::on_app_session_quit()
{
	m_signal_session_quit.emit();
}

void Gobby::Header::on_app_document_create()
{
	m_signal_document_create.emit();
}

void Gobby::Header::on_app_document_open()
{
	m_signal_document_open.emit();
}

void Gobby::Header::on_app_document_save()
{
	m_signal_document_save.emit();
}

void Gobby::Header::on_app_document_save_as()
{
	m_signal_document_save_as.emit();
}

void Gobby::Header::on_app_document_close()
{
	m_signal_document_close.emit();
}

void Gobby::Header::on_app_edit_preferences()
{
	m_signal_edit_preferences.emit();
}

void Gobby::Header::on_app_user_set_password()
{
	m_signal_user_set_password.emit();
}

void Gobby::Header::on_app_user_set_colour()
{
	m_signal_user_set_colour.emit();
}

void Gobby::Header::on_app_view_preferences()
{
	m_signal_view_preferences.emit();
}

void Gobby::Header::on_app_view_language(Glib::RefPtr<Gtk::SourceLanguage> lang)
{
	// Same as below
	if(!m_toggle_language)
		m_signal_view_language.emit(lang);
}

void Gobby::Header::on_app_about()
{
	m_signal_about.emit();
}

void Gobby::Header::on_app_quit()
{
	m_signal_quit.emit();
}

void Gobby::Header::on_folder_tab_switched(Document& document)
{
	// We are toggling some flags
	m_toggle_language = true;

	// Set current language
	Glib::ustring langname = document.get_language() ?
		document.get_language()->get_name() : "None";
	remove_dangerous_xml(langname);
	Glib::RefPtr<Gtk::RadioAction>::cast_static<Gtk::Action>(
		m_group_app->get_action("ViewSyntaxLanguage" + langname)
	)->set_active();

	// Foognar
	m_group_app->get_action("CloseDocument")->set_sensitive(
		document.is_subscribed()
	);

	// We are toggling no more
	m_toggle_language = false;
}
#endif
