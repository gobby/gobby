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

Gobby::Header::Header()
 : m_ui_manager(Gtk::UIManager::create() ),
   m_group_app(Gtk::ActionGroup::create() ),
   m_group_session(Gtk::ActionGroup::create() )
{
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
	m_ui_manager->add_ui_from_string(ui_desc);

	m_menubar = static_cast<Gtk::MenuBar*>(
		m_ui_manager->get_widget("/MenuMainBar") );
	m_toolbar = static_cast<Gtk::Toolbar*>(
		m_ui_manager->get_widget("/ToolMainBar") );

	if(m_menubar == NULL)
		throw Error(Error::MENUBAR_MISSING,
			"XML UI definition lacks menubar");
	if(m_toolbar == NULL)
		throw Error(Error::TOOLBAR_MISSING,
			"XML UI definition lacks toolbar");

	pack_start(*m_menubar, Gtk::PACK_SHRINK);
	pack_start(*m_toolbar, Gtk::PACK_SHRINK);

	// Without having joined a session, there is no support for documents
	// or the possibility to quit it.
	m_group_app->get_action("CreateDocument")->set_sensitive(false);
	m_group_app->get_action("OpenDocument")->set_sensitive(false);
	m_group_app->get_action("SaveDocument")->set_sensitive(false);
	m_group_app->get_action("CloseDocument")->set_sensitive(false);
	m_group_app->get_action("QuitSession")->set_sensitive(false);
}

Gobby::Header::~Header()
{
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

void Gobby::Header::obby_start()
{
	// Begin of obby session: Disable create/join buttons, enable quit
	m_group_app->get_action("CreateSession")->set_sensitive(false);
	m_group_app->get_action("JoinSession")->set_sensitive(false);
	m_group_app->get_action("QuitSession")->set_sensitive(true);

	// Enable document buttons
	m_group_app->get_action("CreateDocument")->set_sensitive(true);
	m_group_app->get_action("OpenDocument")->set_sensitive(true);

	// SaveDocument and CloseDocument will be activated from the
	// insert_document event
	m_group_app->get_action("SaveDocument")->set_sensitive(false);
	m_group_app->get_action("CloseDocument")->set_sensitive(false);
}

void Gobby::Header::obby_end()
{
	// End of obby session: Enable create/join buttons, disable quit
	m_group_app->get_action("CreateSession")->set_sensitive(true);
	m_group_app->get_action("JoinSession")->set_sensitive(true);
	m_group_app->get_action("QuitSession")->set_sensitive(false);

	// Disable document buttons
	m_group_app->get_action("CreateDocument")->set_sensitive(false);
	m_group_app->get_action("OpenDocument")->set_sensitive(false);
	m_group_app->get_action("SaveDocument")->set_sensitive(false);
	m_group_app->get_action("CloseDocument")->set_sensitive(false);
}

void Gobby::Header::obby_user_join(obby::user& user)
{
}

void Gobby::Header::obby_user_part(obby::user& user)
{
}

void Gobby::Header::obby_document_insert(obby::document& document)
{
	// Now we have at least one document open, so we could activate the
	// save and close buttons.
	m_group_app->get_action("SaveDocument")->set_sensitive(true);
	m_group_app->get_action("CloseDocument")->set_sensitive(true);
}

void Gobby::Header::obby_document_remove(obby::document& document)
{
	// TODO: Check the documents for being 0 and deactivate save and close
	// document
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

void Gobby::Header::on_app_about()
{
	m_signal_about.emit();
}

void Gobby::Header::on_app_quit()
{
	m_signal_quit.emit();
}


