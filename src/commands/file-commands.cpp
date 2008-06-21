/* gobby - A GTKmm driven libobby client
 * Copyright (C) 2005 - 2008 0x539 dev group
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

#include "commands/file-commands.hpp"
#include "util/i18n.hpp"

#include <gtkmm/stock.h>

Gobby::FileCommands::FileCommands(Gtk::Window& parent, Header& header,
                                  const Browser& browser, Folder& folder,
                                  Operations& operations):
	m_parent(parent), m_browser(browser), m_folder(folder),
	m_operations(operations), m_mode(MODE_NEW)
{
	header.action_file_new->signal_activate().connect(
		sigc::mem_fun(*this, &FileCommands::on_new));
	header.action_file_open->signal_activate().connect(
		sigc::mem_fun(*this, &FileCommands::on_open));
}

void Gobby::FileCommands::create_location_dialog()
{
	m_location_dialog.reset(
		new DocumentLocationDialog(
			m_parent,
			INF_GTK_BROWSER_MODEL(
				m_browser.get_store())));

	m_location_dialog->signal_response().connect(
		sigc::mem_fun(
			*this,
			&FileCommands::on_location_dialog_response));
}

void Gobby::FileCommands::create_file_dialog()
{
	m_file_dialog.reset(new Gtk::FileChooserDialog(m_parent, ""));
	m_file_dialog->add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	m_file_dialog->add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_ACCEPT);
	m_file_dialog->signal_response().connect(
		sigc::mem_fun(*this, &FileCommands::on_file_dialog_response));
}

void Gobby::FileCommands::on_new()
{
	if(m_file_dialog.get() != NULL)
		m_file_dialog->hide();

	if(m_location_dialog.get() == NULL)
		create_location_dialog();

	m_mode = MODE_NEW;
	m_location_dialog->set_document_name(_("New Document"));
	m_location_dialog->present();
}

void Gobby::FileCommands::on_open()
{
	if(m_location_dialog.get() != NULL)
		m_location_dialog->hide();

	if(m_file_dialog.get() == NULL)
		create_file_dialog();

	// TODO: Allow multiple selection
	// TODO: Encoding selection in file chooser
	m_mode = MODE_OPEN;
	m_file_dialog->set_title("Choose a text file to open");
	m_file_dialog->set_action(Gtk::FILE_CHOOSER_ACTION_OPEN);
	m_file_dialog->present();
}

void Gobby::FileCommands::on_file_dialog_response(int id)
{
	if(id == Gtk::RESPONSE_ACCEPT)
	{
		switch(m_mode)
		{
		case MODE_OPEN:
			if(m_location_dialog.get() == NULL)
				create_location_dialog();

			// TODO: Handle multiple selection
			m_open_uri = m_file_dialog->get_uri();
			m_location_dialog->set_document_name(
				Glib::path_get_basename(
					m_file_dialog->get_filename()));
			m_location_dialog->present();

			break;
		case MODE_SAVE:
			// TODO:
			break;
		case MODE_NEW:
		default:
			g_assert_not_reached();
		}
	}

	m_file_dialog->hide();
}

void Gobby::FileCommands::on_location_dialog_response(int id)
{
	if(id == Gtk::RESPONSE_ACCEPT)
	{
		InfcBrowserIter iter;
		InfcBrowser* browser =
			m_location_dialog->get_selected_directory(&iter);
		g_assert(browser != NULL);

		switch(m_mode)
		{
		case MODE_NEW:
			m_operations.create_document(
				browser, &iter,
				m_location_dialog->get_document_name());
			break;
		case MODE_OPEN:
			m_operations.create_document(
				browser, &iter,
				m_location_dialog->get_document_name(),
				m_open_uri, NULL);
			break;
		case MODE_SAVE:
		default:
			g_assert_not_reached();
		}
	}

	m_location_dialog->hide();
}
