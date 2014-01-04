/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2014 Armin Burgmeier <armin@arbur.net>
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
 * Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#ifndef _GOBBY_FOLDER_HPP_
#define _GOBBY_FOLDER_HPP_

#include "core/textsessionview.hpp"
#include "core/chatsessionview.hpp"
#include "core/preferences.hpp"
#include "util/defaultaccumulator.hpp"
#include "util/gtk-compat.hpp"

#include <gtkmm/notebook.h>
#include <sigc++/signal.h>

#include <gtksourceview/gtksourcelanguagemanager.h>

namespace Gobby
{

class Folder : public GtkCompat::Notebook
{
public:
	// TODO chat: This should be SignalSessionAdded/Removed/Changed
	typedef sigc::signal<void, SessionView&> SignalDocumentAdded;
	typedef sigc::signal<void, SessionView&> SignalDocumentRemoved;
	typedef sigc::signal<void, SessionView*> SignalDocumentChanged;

	typedef sigc::signal<bool, SessionView&>::
		accumulated<default_accumulator<bool, true> >
			SignalDocumentCloseRequest;

	// TODO chat: Should not require langmgr
	Folder(bool hide_single_tab,
	       Preferences& preferences,
	       GtkSourceLanguageManager* lang_manager);
	~Folder();

	TextSessionView& add_text_session(InfTextSession* session,
	                                  const Glib::ustring& title,
	                                  const Glib::ustring& path,
	                                  const Glib::ustring& hostname,
	                                  const std::string& info_storage_key);
	ChatSessionView& add_chat_session(InfChatSession* session,
	                                  const Glib::ustring& title,
	                                  const Glib::ustring& path,
	                                  const Glib::ustring& hostname);
	void remove_document(SessionView& view);

	SessionView& get_document(unsigned int n);

	SessionView* lookup_document(InfSession* session);
	SessionView* get_current_document();
	const SessionView* get_current_document() const;
	void switch_to_document(SessionView& document);

	SignalDocumentAdded signal_document_added() const
	{
		return m_signal_document_added;
	}

	SignalDocumentRemoved signal_document_removed() const
	{
		return m_signal_document_removed;
	}

	SignalDocumentChanged signal_document_changed() const
	{
		return m_signal_document_changed;
	}

	SignalDocumentCloseRequest signal_document_close_request() const
	{
		return m_signal_document_close_request;
	}

protected:
	virtual void on_switch_page(Gtk::Widget* page, guint page_num);
	virtual bool on_key_press_event(GdkEventKey* event);

	void on_tab_close_request(SessionView& window);

	const bool m_hide_single_tab;
	Preferences& m_preferences;
	GtkSourceLanguageManager* m_lang_manager;

	SignalDocumentAdded m_signal_document_added;
	SignalDocumentRemoved m_signal_document_removed;
	SignalDocumentChanged m_signal_document_changed;
	SignalDocumentCloseRequest m_signal_document_close_request;
};

}

#endif // _GOBBY_FOLDER_HPP_
