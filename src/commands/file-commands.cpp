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

// TODO: Introduce separate classes for the different tasks here...

namespace
{
	void save_document(Gobby::DocWindow& document,
	                   Gobby::Operations& operations,
	                   const Gobby::DocumentInfoStorage& info_storage,
	                   Gobby::Folder& folder,
	                   Gtk::FileChooserDialog& dialog)
	{
		const Gobby::DocumentInfoStorage::Info* info =
			info_storage.get_info(
				document.get_info_storage_key());

		// TODO: Get encoding from file dialog
		operations.save_document(
			document, folder, dialog.get_uri(),
			info ? info->encoding : "UTF-8",
			info ? info->eol_style :
				Gobby::DocumentInfoStorage::EOL_LF);
	}
}

Gobby::FileCommands::FileCommands(Gtk::Window& parent, Header& header,
                                  const Browser& browser, Folder& folder,
                                  Operations& operations,
				  const DocumentInfoStorage& info_storage,
                                  Preferences& preferences):
	m_parent(parent), m_header(header), m_browser(browser),
	m_folder(folder), m_operations(operations),
	m_document_info_storage(info_storage), m_preferences(preferences),
	m_current_uri(Glib::filename_to_uri(Glib::get_current_dir())),
	m_mode(MODE_NEW)
{
	header.action_file_new->signal_activate().connect(
		sigc::mem_fun(*this, &FileCommands::on_new));
	header.action_file_open->signal_activate().connect(
		sigc::mem_fun(*this, &FileCommands::on_open));
	header.action_file_save->signal_activate().connect(
		sigc::mem_fun(*this, &FileCommands::on_save));
	header.action_file_save_as->signal_activate().connect(
		sigc::mem_fun(*this, &FileCommands::on_save_as));
	folder.signal_document_removed().connect(
		sigc::mem_fun(*this, &FileCommands::on_document_removed));
	folder.signal_document_changed().connect(
		sigc::mem_fun(*this, &FileCommands::on_document_changed));

	set_sensitivity(folder.get_current_document() != NULL);
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

void Gobby::FileCommands::create_file_dialog(Gtk::FileChooserAction action)
{
	if(m_file_dialog.get() == NULL ||
	   m_file_dialog->Gtk::FileChooser::get_action() != action)
	{
		m_file_dialog.reset(new Gtk::FileChooserDialog(m_parent, "",
		                                               action));
		m_file_dialog->add_button(Gtk::Stock::CANCEL,
		                          Gtk::RESPONSE_CANCEL);

		if(action == Gtk::FILE_CHOOSER_ACTION_OPEN)
		{
			m_file_dialog->add_button(Gtk::Stock::OPEN,
			                          Gtk::RESPONSE_ACCEPT);
		}
		else
		{
			m_file_dialog->set_do_overwrite_confirmation(true);

			m_file_dialog->add_button(Gtk::Stock::SAVE,
			                          Gtk::RESPONSE_ACCEPT);
		}

		m_file_dialog->signal_response().connect(
			sigc::mem_fun(
				*this,
				&FileCommands::on_file_dialog_response));
	}
}

void Gobby::FileCommands::on_document_removed(DocWindow& document)
{
	// Hide save dialog again if the uses closes the document s/he is
	// about to save...
	if(m_mode == MODE_SAVE && &document == m_save_document)
	{
		m_save_document = NULL;
		m_file_dialog.reset(NULL);
	}
}

void Gobby::FileCommands::on_document_changed(DocWindow* document)
{
	set_sensitivity(document != NULL);
}

void Gobby::FileCommands::on_new()
{
	m_file_dialog.reset(NULL);
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

	create_file_dialog(Gtk::FILE_CHOOSER_ACTION_OPEN);

	// TODO: Allow multiple selection
	// TODO: Encoding selection in file chooser
	m_mode = MODE_OPEN;
	m_file_dialog->set_title(_("Choose a text file to open"));
	m_file_dialog->set_current_folder_uri(m_current_uri);
	m_file_dialog->present();
}

void Gobby::FileCommands::on_save()
{
	// TODO: Encoding selection in file chooser
	DocWindow* document = m_folder.get_current_document();
	g_assert(document != NULL);

	const DocumentInfoStorage::Info* info =
		m_document_info_storage.get_info(
			document->get_info_storage_key());

	if(info != NULL && !info->uri.empty())
	{
		m_operations.save_document(
			*document, m_folder, info->uri, info->encoding,
			info->eol_style);
	}
	else
	{
		on_save_as();
	}
}

void Gobby::FileCommands::on_save_as()
{
	if(m_location_dialog.get() != NULL)
		m_location_dialog->hide();

	create_file_dialog(Gtk::FILE_CHOOSER_ACTION_SAVE);

	DocWindow* document = m_folder.get_current_document();
	g_assert(document != NULL);
	
	m_mode = MODE_SAVE;
	m_save_document = document;
	m_file_dialog->set_title(
		Glib::ustring::compose(
			_("Choose a location to save document \"%1\" to"),
			  m_save_document->get_title()));

	const DocumentInfoStorage::Info* info =
		m_document_info_storage.get_info(
			document->get_info_storage_key());

	if(info != NULL && !info->uri.empty())
		m_file_dialog->set_uri(info->uri);
	else
		m_file_dialog->set_current_folder_uri(m_current_uri);

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
			save_document(*m_save_document, m_operations,
			              m_document_info_storage, m_folder,
			              *m_file_dialog);
			break;
		case MODE_NEW:
		default:
			g_assert_not_reached();
		}
	}

	m_current_uri = m_file_dialog->get_current_folder_uri();
	m_file_dialog.reset(NULL);
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
				m_preferences, m_open_uri, NULL);
			break;
		case MODE_SAVE:
		default:
			g_assert_not_reached();
		}
	}

	m_location_dialog->hide();
}

void Gobby::FileCommands::set_sensitivity(bool sensitivity)
{
	m_header.action_file_save->set_sensitive(sensitivity);
	m_header.action_file_save_as->set_sensitive(sensitivity);
	m_header.action_file_save_all->set_sensitive(sensitivity);
}
