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
#include <gtkmm/toggleaction.h>
#include <gtkmm/radioaction.h>
#include <obby/format_string.hpp>
#include <obby/local_buffer.hpp>
#include <obby/client_buffer.hpp>

#include "features.hpp"
#include "common.hpp"
#include "header.hpp"

namespace {
	Glib::ustring ui_desc = 
		"<ui>"
		"  <menubar name=\"MenuMainBar\">"
		"    <menu action=\"MenuApp\">"
		"      <menuitem action=\"CreateSession\" />"
		"      <menuitem action=\"JoinSession\" />"
		"      <menuitem action=\"QuitSession\" />"
		"      <separator />"
		"      <menuitem action=\"Quit\" />"
		"    </menu>"
		"    <menu action=\"MenuSession\">"
		"      <menuitem action=\"CreateDocument\" />"
		"      <menuitem action=\"OpenDocument\" />"
		"      <menuitem action=\"SaveDocument\" />"
		"      <menuitem action=\"CloseDocument\" />"
		"    </menu>"
		"    <menu action=\"MenuEdit\">"
		"      <menuitem action=\"EditPreferences\" />"
		"    </menu>"
		"    <menu action=\"MenuUser\">"
		"      <menuitem action=\"UserSetPassword\" />"
		"    </menu>"
		"    <menu action=\"MenuView\">"
		"      <menuitem action=\"ViewLanguageNone\" />"
		"    </menu>"
		"    <menu action=\"MenuHelp\">"
		"      <menuitem action=\"About\" />"
		"    </menu>"
		"  </menubar>"
		"  <toolbar name=\"ToolMainBar\">"
		"    <toolitem action=\"CreateSession\" />"
		"    <toolitem action=\"JoinSession\" />"
		"    <toolitem action=\"QuitSession\" />"
		"    <separator />"
		"    <toolitem action=\"CreateDocument\" />"
		"    <toolitem action=\"OpenDocument\" />"
		"    <toolitem action=\"SaveDocument\" />"
		"    <toolitem action=\"CloseDocument\" />"
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

Gobby::Header::Header(const Folder& folder)
 : m_ui_manager(Gtk::UIManager::create() ),
   m_group_app(Gtk::ActionGroup::create() ),
   m_toggle_language(false)
{
	// Add basic menu
	m_ui_manager->add_ui_from_string(ui_desc);

	// App menu
	m_group_app->add(Gtk::Action::create("MenuApp", _("Gobby")) );

	// Create session
	m_group_app->add(
		Gtk::Action::create(
			"CreateSession",
			Gtk::Stock::NETWORK,
			_("Create session"),
			_("Opens a new obby session")
		),
		sigc::mem_fun(
			*this,
			&Header::on_app_session_create
		)
	);

	// Join session
	m_group_app->add(
		Gtk::Action::create(
			"JoinSession",
			Gtk::Stock::CONNECT,
			_("Join session"),
			_("Join an existing obby session")
		),
		sigc::mem_fun(
			*this,
			&Header::on_app_session_join
		)
	);

	// Quit session
	m_group_app->add(
		Gtk::Action::create(
			"QuitSession",
			Gtk::Stock::DISCONNECT,
			_("Quit session"),
			_("Leaves the currently running obby session")
		),
		sigc::mem_fun(
			*this,
			&Header::on_app_session_quit
		)
	);

	// Quit application
	m_group_app->add(
		Gtk::Action::create(
			"Quit",
			Gtk::Stock::QUIT,
			_("Quit"),
			_("Quits the application")
		),
		sigc::mem_fun(
			*this,
			&Header::on_app_quit
		)
	);

	// Session menu
	m_group_app->add(Gtk::Action::create("MenuSession", _("Session")) );

	// Create document
	m_group_app->add(
		Gtk::Action::create(
			"CreateDocument",
			Gtk::Stock::NEW,
			_("Create document"),
			_("Creates a new document")
		),
		sigc::mem_fun(
			*this,
			&Header::on_app_document_create
		)
	);

	// Open document
	m_group_app->add(
		Gtk::Action::create(
			"OpenDocument",
			Gtk::Stock::OPEN,
			_("Open document"),
			_("Loads a file into a new document")
		),
		sigc::mem_fun(
			*this,
			&Header::on_app_document_open
		)
	);

	// Save document
	m_group_app->add(
		Gtk::Action::create(
			"SaveDocument",
			Gtk::Stock::SAVE,
			_("Save document"),
			_("Saves a document into a file")
		),
		sigc::mem_fun(
			*this,
			&Header::on_app_document_save
		)
	);

	// Close document
	m_group_app->add(
		Gtk::Action::create(
			"CloseDocument",
			Gtk::Stock::CLOSE,
			_("Close document"),
			_("Closes an open document")
		),
		sigc::mem_fun(
			*this,
			&Header::on_app_document_close
		)
	);

	// Edit menu
	m_group_app->add(Gtk::Action::create("MenuEdit", _("Edit")) );

	// Preferences
	m_group_app->add(
		Gtk::Action::create(
			"EditPreferences",
			Gtk::Stock::PREFERENCES,
			_("Preferences"),
			_("Shows up a dialog to set up gobby to your needs")
		),
		sigc::mem_fun(
			*this,
			&Header::on_app_edit_preferences
		)
	);

	// User menu
	m_group_app->add(Gtk::Action::create("MenuUser", _("User")) );

	// Set password
	m_group_app->add(
		Gtk::Action::create(
			"UserSetPassword",
			Gtk::Stock::DIALOG_AUTHENTICATION,
			_("Set password"),
			_("Sets a password for this user")
		),
		sigc::mem_fun(
			*this,
			&Header::on_app_user_set_password
		)
	);

	// Documents menu
	m_group_app->add(Gtk::Action::create("MenuView", _("View")) );

	// A kind of hack to ensure that
	// Gtk::SourceLanguage::sourcelanguage_class_.init() is called.
	// See the TODO item in Glib::wrap(GtkSourceLanguage*, bool) in
	// sourcelanguage.cpp
	GtkSourceLanguage* lang = NULL;
	Glib::wrap(lang, false);

	// Get languages manager
	Glib::RefPtr<Gtk::SourceLanguagesManager> lang_manager =
		folder.get_lang_manager();

	// Get available languages
	std::list<Glib::RefPtr<Gtk::SourceLanguage> > lang_list =
		lang_manager->get_available_languages();
	
	// Sort languages by name
	lang_list.sort(&Header::language_sort_callback);

	// Add None-Language
	m_group_app->add(
		Gtk::RadioAction::create(
			m_lang_group,
			"ViewLanguageNone",
			_("None"),
			_("Unselects the current language")
		),
		sigc::bind(
			sigc::mem_fun(
				*this,
				&Header::on_app_document_language
			),
			Glib::RefPtr<Gtk::SourceLanguage>()
		)
	);

	// Add languages
	std::list<Glib::RefPtr<Gtk::SourceLanguage> >::const_iterator iter;
	for(iter = lang_list.begin(); iter != lang_list.end(); ++ iter)
	{
		// Get current language 
		Glib::RefPtr<Gtk::SourceLanguage> language = *iter;
		Glib::ustring language_xml_name = language->get_name();

		// Build description string
		obby::format_string str(_("Selects %0 as language") );
		str << language->get_name().raw();

		// Add language to action group
		remove_dangerous_xml(language_xml_name);
		m_group_app->add(
			Gtk::RadioAction::create(
				m_lang_group,
				"ViewLanguage" + language_xml_name, 
				language->get_name(),
				str.str()
			),
			sigc::bind(
				sigc::mem_fun(
					*this,
					&Header::on_app_document_language
				),
				language
			)
		);

		// Add menu item to UI
		Glib::ustring xml_desc =
			"<ui>"
			"  <menubar name=\"MenuMainBar\">"
			"    <menu action=\"MenuView\">"
			"	<menuitem action=\"ViewLanguage"
				+ language_xml_name + "\" />"
			"    </menu>"
			"  </menubar>"
			"</ui>";

		m_ui_manager->add_ui_from_string(xml_desc);
	}

	// Help menu
	m_group_app->add(Gtk::Action::create("MenuHelp", _("Help")) );

	// Display about dialog
	m_group_app->add(
		Gtk::Action::create(
			"About",
			Gtk::Stock::ABOUT,
			_("About"),
			_("Shows Gobby's copyright and credits")
		),
		sigc::mem_fun(
			*this,
			&Header::on_app_about
		)
	);

	m_ui_manager->insert_action_group(m_group_app);

	m_menubar = static_cast<Gtk::MenuBar*>(
		m_ui_manager->get_widget("/MenuMainBar") );
	m_toolbar = static_cast<Gtk::Toolbar*>(
		m_ui_manager->get_widget("/ToolMainBar") );

	if(m_menubar == NULL)
		throw Error(Error::MENUBAR_MISSING,
			"XML UI definition lacks menubar" );
	if(m_toolbar == NULL)
		throw Error(Error::TOOLBAR_MISSING,
			"XML UI definition lacks toolbar" );

	pack_start(*m_menubar, Gtk::PACK_SHRINK);
	pack_start(*m_toolbar, Gtk::PACK_SHRINK);

	// Without having joined a session, there is no support for documents
	// or the possibility to quit it.
	m_group_app->get_action("CreateDocument")->set_sensitive(false);
	m_group_app->get_action("OpenDocument")->set_sensitive(false);
	m_group_app->get_action("SaveDocument")->set_sensitive(false);
	m_group_app->get_action("CloseDocument")->set_sensitive(false);
	m_group_app->get_action("QuitSession")->set_sensitive(false);

	m_group_app->get_action("MenuUser")->set_sensitive(false);
	m_group_app->get_action("MenuView")->set_sensitive(false);

	// Connect to folder's signals
	folder.tab_switched_event().connect(
		sigc::mem_fun(*this, &Header::on_folder_tab_switched) );
}

Gobby::Header::~Header()
{
}

Glib::RefPtr<Gtk::AccelGroup> Gobby::Header::get_accel_group()
{
	return m_ui_manager->get_accel_group();
}

Glib::RefPtr<const Gtk::AccelGroup> Gobby::Header::get_accel_group() const
{
	return m_ui_manager->get_accel_group();
}

void Gobby::Header::disable_document_actions()
{
	m_group_app->get_action("SaveDocument")->set_sensitive(false);
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

Gobby::Header::signal_document_language_type
Gobby::Header::document_language_event() const
{
	return m_signal_document_language;
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
	m_group_app->get_action("CloseDocument")->set_sensitive(false);
	m_group_app->get_action("MenuView")->set_sensitive(false);
}

void Gobby::Header::obby_end()
{
	// End of obby session: Enable create/join buttons, disable quit
	m_group_app->get_action("CreateSession")->set_sensitive(true);
	m_group_app->get_action("JoinSession")->set_sensitive(true);
	m_group_app->get_action("QuitSession")->set_sensitive(false);

	// Disable user buttons
	m_group_app->get_action("MenuUser")->set_sensitive(false);

	// Disable document buttons
	m_group_app->get_action("CreateDocument")->set_sensitive(false);
	m_group_app->get_action("OpenDocument")->set_sensitive(false);

	// Leave document actions like SaveDocument and CloseDocument as they
	// were to allow the user to save documents.
}

void Gobby::Header::obby_user_join(obby::user& user)
{
}

void Gobby::Header::obby_user_part(obby::user& user)
{
}

void Gobby::Header::obby_document_insert(obby::local_document_info& document)
{
	// Now we have at least one document open, so we could activate the
	// document actions.
	m_group_app->get_action("SaveDocument")->set_sensitive(true);
	m_group_app->get_action("CloseDocument")->set_sensitive(true);
	m_group_app->get_action("MenuView")->set_sensitive(true);
}

void Gobby::Header::obby_document_remove(obby::local_document_info& document)
{
	if(document.get_buffer().document_count() == 1)
	{
		// The document which is currently removed is the only
		// existing document? Disable document actions then.
		m_group_app->get_action("SaveDocument")->set_sensitive(false);
		m_group_app->get_action("CloseDocument")->set_sensitive(false);
		m_group_app->get_action("MenuView")->set_sensitive(false);
	}
}

void Gobby::Header::obby_preferences_changed(const Preferences& preferences)
{
}

void Gobby::Header::on_app_session_create()
{
	m_signal_session_create.emit();
}

void Gobby::Header::on_app_session_join()
{
	m_signal_session_join.emit();
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

void Gobby::Header::on_app_document_language(
	Glib::RefPtr<Gtk::SourceLanguage> lang
)
{
	// Same as above
	if(!m_toggle_language)
		m_signal_document_language.emit(lang);
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
		m_group_app->get_action("ViewLanguage" + langname)
	)->set_active();

	// We are no more toggling
	m_toggle_language = false;
}

bool Gobby::Header::language_sort_callback(
	const Glib::RefPtr<Gtk::SourceLanguage>& lang1,
	const Glib::RefPtr<Gtk::SourceLanguage>& lang2
)
{
	return lang1->get_name() < lang2->get_name();
}

