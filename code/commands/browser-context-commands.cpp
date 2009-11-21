/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008, 2009 Armin Burgmeier <armin@arbur.net>
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

#include "commands/browser-context-commands.hpp"
#include "operations/operation-open-multiple.hpp"
#include "util/i18n.hpp"

#include <gtkmm/icontheme.h>
#include <gtkmm/stock.h>
#include <giomm/file.h>

// TODO: Use file tasks for the commands, once we made them public

Gobby::BrowserContextCommands::BrowserContextCommands(Gtk::Window& parent,
                                                      Browser& browser,
                                                      FileChooser& chooser,
                                                      Operations& operations,
						      const Preferences& prf):
	m_parent(parent), m_browser(browser), m_file_chooser(chooser),
	m_operations(operations), m_preferences(prf), m_popup_menu(NULL)
{
	m_populate_popup_handler = g_signal_connect(
		m_browser.get_view(), "populate-popup",
		G_CALLBACK(on_populate_popup_static), this);
}

Gobby::BrowserContextCommands::~BrowserContextCommands()
{
	g_signal_handler_disconnect(m_browser.get_view(),
	                            m_populate_popup_handler);
}

void Gobby::BrowserContextCommands::on_node_removed()
{
	g_assert(m_popup_menu != NULL);

	// This calls deactivate, causing the watch to be removed.
	m_popup_menu->popdown();
}

void Gobby::BrowserContextCommands::on_menu_deactivate()
{
	m_watch.reset(NULL);
}

void Gobby::BrowserContextCommands::on_populate_popup(Gtk::Menu* menu)
{
	// TODO: Can this happen? Should we close the old popup here?
	g_assert(m_popup_menu == NULL);

	// Cancel previous attempts
	m_watch.reset(NULL);
	m_entry_dialog.reset(NULL);
	m_file_dialog.reset(NULL);

	InfcBrowser* browser;
	InfcBrowserIter iter;

	if(m_browser.get_selected(&browser, &iter))
	{
		InfcBrowserIter dummy_iter = iter;
		bool is_subdirectory =
			infc_browser_iter_is_subdirectory(browser, &iter);
		bool is_toplevel =
			!infc_browser_iter_get_parent(browser, &dummy_iter);

		// Watch the node, and close the popup menu when the node
		// it refers to is removed.
		m_watch.reset(new NodeWatch(browser, &iter));
		m_watch->signal_node_removed().connect(sigc::mem_fun(
			*this, &BrowserContextCommands::on_node_removed));

		menu->signal_deactivate().connect(sigc::mem_fun(
			*this, &BrowserContextCommands::on_menu_deactivate));

		// Create Document
		Gtk::ImageMenuItem* new_document_item = Gtk::manage(
			new Gtk::ImageMenuItem(_("Create Do_cument..."),
			                       true));
		new_document_item->set_image(*Gtk::manage(new Gtk::Image(
			Gtk::Stock::NEW, Gtk::ICON_SIZE_MENU)));
		new_document_item->signal_activate().connect(sigc::bind(
			sigc::mem_fun(*this,
				&BrowserContextCommands::on_new),
			browser, iter, false));
		new_document_item->set_sensitive(is_subdirectory);
		new_document_item->show();
		menu->append(*new_document_item);

		// Create Directory

		// Check whether we have the folder-new icon, fall back to
		// Stock::DIRECTORY otherwise
		Glib::RefPtr<Gdk::Screen> screen = menu->get_screen();
		Glib::RefPtr<Gtk::IconTheme> icon_theme(
			Gtk::IconTheme::get_for_screen(screen));

		Gtk::Image* new_directory_image = Gtk::manage(new Gtk::Image);
		if(icon_theme->lookup_icon("folder-new",
		                           Gtk::ICON_SIZE_MENU,
		                           Gtk::ICON_LOOKUP_USE_BUILTIN))
		{
			new_directory_image->set_from_icon_name(
				"folder-new", Gtk::ICON_SIZE_MENU);
		}
		else
		{
			new_directory_image->set(
				Gtk::Stock::DIRECTORY, Gtk::ICON_SIZE_MENU);
		}

		Gtk::ImageMenuItem* new_directory_item = Gtk::manage(
			new Gtk::ImageMenuItem(_("Create Directory..."),
			                       true));
		new_directory_item->set_image(*new_directory_image);
		new_directory_item->signal_activate().connect(sigc::bind(
			sigc::mem_fun(*this,
				&BrowserContextCommands::on_new),
			browser, iter, true));
		new_directory_item->set_sensitive(is_subdirectory);
		new_directory_item->show();
		menu->append(*new_directory_item);

		// Open Document
		Gtk::ImageMenuItem* open_document_item = Gtk::manage(
			new Gtk::ImageMenuItem(_("_Open Document..."), true));
		open_document_item->set_image(*Gtk::manage(new Gtk::Image(
			Gtk::Stock::OPEN, Gtk::ICON_SIZE_MENU)));
		open_document_item->signal_activate().connect(sigc::bind(
			sigc::mem_fun(*this,
				&BrowserContextCommands::on_open),
			browser, iter));
		open_document_item->set_sensitive(is_subdirectory);
		open_document_item->show();
		menu->append(*open_document_item);

		// Separator
		Gtk::SeparatorMenuItem* sep_item =
			Gtk::manage(new Gtk::SeparatorMenuItem);
		sep_item->show();
		menu->append(*sep_item);

		// Delete
		Gtk::ImageMenuItem* delete_item = Gtk::manage(
			new Gtk::ImageMenuItem(_("D_elete"), true));
		delete_item->set_image(*Gtk::manage(new Gtk::Image(
			Gtk::Stock::DELETE, Gtk::ICON_SIZE_MENU)));
		delete_item->signal_activate().connect(sigc::bind(
			sigc::mem_fun(*this,
				&BrowserContextCommands::on_delete),
			browser, iter));
		delete_item->set_sensitive(!is_toplevel);
		delete_item->show();
		menu->append(*delete_item);
		
		m_popup_menu = menu;
		menu->signal_selection_done().connect(
			sigc::mem_fun(
				*this,
				&BrowserContextCommands::on_popdown));
	}
}

void Gobby::BrowserContextCommands::on_popdown()
{
	m_popup_menu = NULL;
	m_watch.reset(NULL);
}

void Gobby::BrowserContextCommands::on_new(InfcBrowser* browser,
                                           InfcBrowserIter iter,
                                           bool directory)
{
	m_watch.reset(new NodeWatch(browser, &iter));
	m_watch->signal_node_removed().connect(sigc::mem_fun(
		*this, &BrowserContextCommands::on_new_node_removed));

	m_entry_dialog.reset(
		new EntryDialog(
			m_parent,
			directory ? _("Choose a name for the directory")
			          : _("Choose a name for the document"),
			directory ? _("_Directory Name:")
			          : _("_Document Name:")));

	m_entry_dialog->add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	m_entry_dialog->add_button(_("C_reate"), Gtk::RESPONSE_ACCEPT)
		->set_image(*Gtk::manage(new Gtk::Image(
			Gtk::Stock::NEW, Gtk::ICON_SIZE_BUTTON)));

	m_entry_dialog->set_text(directory ? _("New Directory")
	                                   : _("New Document"));
	m_entry_dialog->signal_response().connect(sigc::bind(
		sigc::mem_fun(*this,
			&BrowserContextCommands::on_new_response),
		browser, iter, directory));
	m_entry_dialog->present();
}

void Gobby::BrowserContextCommands::on_open(InfcBrowser* browser,
                                            InfcBrowserIter iter)
{
	m_watch.reset(new NodeWatch(browser, &iter));
	m_watch->signal_node_removed().connect(sigc::mem_fun(
		*this, &BrowserContextCommands::on_open_node_removed));

	m_file_dialog.reset(new FileChooser::Dialog(
		m_file_chooser, m_parent, _("Choose a text file to open"),
		Gtk::FILE_CHOOSER_ACTION_OPEN));
	m_file_dialog->signal_response().connect(sigc::bind(
		sigc::mem_fun(*this,
			&BrowserContextCommands::on_open_response),
		browser, iter));

	m_file_dialog->set_select_multiple(true);
	m_file_dialog->present();
}

void Gobby::BrowserContextCommands::on_delete(InfcBrowser* browser,
                                              InfcBrowserIter iter)
{
	m_operations.delete_node(browser, &iter);
}

void Gobby::BrowserContextCommands::on_new_node_removed()
{
	m_watch.reset(NULL);
	m_entry_dialog.reset(NULL);
}

void Gobby::BrowserContextCommands::on_new_response(int response_id,
                                                    InfcBrowser* browser,
						    InfcBrowserIter iter,
						    bool directory)
{
	if(response_id == Gtk::RESPONSE_ACCEPT)
	{
		if(directory)
		{
			// TODO: Select the newly created directory in tree
			m_operations.create_directory(
				browser, &iter, m_entry_dialog->get_text());
		}
		else
		{
			m_operations.create_document(
				browser, &iter, m_entry_dialog->get_text());
		}
	}

	m_watch.reset(NULL);
	m_entry_dialog.reset(NULL);
}

void Gobby::BrowserContextCommands::on_open_node_removed()
{
	m_watch.reset(NULL);
	m_file_dialog.reset(NULL);
}

void Gobby::BrowserContextCommands::on_open_response(int response_id,
                                                     InfcBrowser* browser,
                                                     InfcBrowserIter iter)
{
	if(response_id == Gtk::RESPONSE_ACCEPT)
	{
		Glib::SListHandle<Glib::ustring> uris = m_file_dialog->get_uris();

		OperationOpenMultiple* operation =
			m_operations.create_documents(browser, &iter, m_preferences, uris.size());

		for(Glib::SListHandle<Glib::ustring>::iterator i = uris.begin(); i != uris.end(); ++i)
		{
			Glib::RefPtr<Gio::File> file =
				Gio::File::create_for_uri(*i);

			operation->add_uri(*i, NULL, NULL);
		}
	}

	m_watch.reset(NULL);
	m_file_dialog.reset(NULL);
}
