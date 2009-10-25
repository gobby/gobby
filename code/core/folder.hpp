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

#ifndef _GOBBY_FOLDER_HPP_
#define _GOBBY_FOLDER_HPP_

#include "core/docwindow.hpp"
#include "core/preferences.hpp"
#include "util/defaultaccumulator.hpp"

#include <gtkmm/notebook.h>
#include <sigc++/signal.h>

#include <gtksourceview/gtksourcelanguagemanager.h>

namespace Gobby
{

class Folder : public Gtk::Notebook
{
public:
	typedef sigc::signal<void, DocWindow&> SignalDocumentAdded;
	typedef sigc::signal<void, DocWindow&> SignalDocumentRemoved;
	typedef sigc::signal<void, DocWindow*> SignalDocumentChanged;

	typedef sigc::signal<bool, DocWindow&>::
		accumulated<default_accumulator<bool, true> >
			SignalDocumentCloseRequest;

	Folder(Preferences& preferences,
	       GtkSourceLanguageManager* lang_manager);
	~Folder();

	DocWindow& add_document(InfTextSession* session,
	                        const Glib::ustring& title,
	                        const Glib::ustring& path,
	                        const Glib::ustring& hostname,
	                        const std::string& info_storage_key);
	void remove_document(DocWindow& document);

	DocWindow* lookup_document(InfTextSession* session);
	DocWindow* get_current_document();
	const DocWindow* get_current_document() const;
	void switch_to_document(DocWindow& document);

	SignalDocumentAdded signal_document_added() const {
		return m_signal_document_added;
	}

	SignalDocumentRemoved signal_document_removed() const {
		return m_signal_document_removed;
	}

	SignalDocumentChanged signal_document_changed() const {
		return m_signal_document_changed;
	}

	SignalDocumentCloseRequest signal_document_close_request() const {
		return m_signal_document_close_request;
	}

protected:
	virtual void on_switch_page(GtkNotebookPage* page, guint page_num);
	virtual bool on_key_press_event(GdkEventKey* event);

	void on_tab_close_request(DocWindow& window);

	Preferences& m_preferences;
	GtkSourceLanguageManager* m_lang_manager;

	SignalDocumentAdded m_signal_document_added;
	SignalDocumentRemoved m_signal_document_removed;
	SignalDocumentChanged m_signal_document_changed;
	SignalDocumentCloseRequest m_signal_document_close_request;
};

}

#endif // _GOBBY_FOLDER_HPP_
