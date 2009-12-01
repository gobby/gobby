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

#include "core/folder.hpp"
#include "core/sessionuserview.hpp"
#include "core/chattablabel.hpp"
#include "core/texttablabel.hpp"
#include "util/file.hpp"

#include <gdk/gdkkeysyms.h>
#include <stdexcept>
#include <iostream> // For std::cerr

#include <libinfinity/adopted/inf-adopted-session-record.h>

namespace
{
	class KeyMap
	{
	public:
		static const unsigned int nval = ~0u;

		KeyMap()
		{
			m_keyvals[GDK_0] = 9;
			m_keyvals[GDK_1] = 0;
			m_keyvals[GDK_2] = 1;
			m_keyvals[GDK_3] = 2;
			m_keyvals[GDK_4] = 3;
			m_keyvals[GDK_5] = 4;
			m_keyvals[GDK_6] = 5;
			m_keyvals[GDK_7] = 6;
			m_keyvals[GDK_8] = 7;
			m_keyvals[GDK_9] = 8;
		}

		unsigned int lookup(guint key) const
		{
			map_type::const_iterator iter = m_keyvals.find(key);
			if(iter == m_keyvals.end() ) return nval;
			return iter->second;
		}

	private:
		typedef std::map<guint, unsigned int> map_type;
		map_type m_keyvals;
	};

	void record(InfTextSession* session, const Glib::ustring& title)
	{
		std::string dirname = Glib::build_filename(
			Glib::get_home_dir(), ".infinote-records");
		std::string filename = Glib::build_filename(
			dirname, title + ".record.xml");

		try
		{
			Gobby::create_directory_with_parents(dirname);

			InfAdoptedSessionRecord* record =
				inf_adopted_session_record_new(
					INF_ADOPTED_SESSION(session));

			GError* error = NULL;
			inf_adopted_session_record_start_recording(
				record, filename.c_str(), &error);
			if(error != NULL)
			{
				g_object_unref(record);

				std::string what = error->message;
				g_error_free(error);
				throw std::runtime_error(what);
			}

			g_object_set_data_full(
				G_OBJECT(session), "GOBBY_SESSION_RECORD",
				record, g_object_unref);
		}
		catch(std::exception& ex)
		{
			std::cerr << "Failed to create record '" << filename
			          << "': " << ex.what() << std::endl;
		}
	}
}

Gobby::Folder::Folder(bool hide_single_tab,
                      Preferences& preferences,
                      GtkSourceLanguageManager* lang_manager):
	m_hide_single_tab(hide_single_tab), m_preferences(preferences),
	m_lang_manager(lang_manager)
{
	set_scrollable(true);
	if(hide_single_tab) set_show_tabs(false);
}

Gobby::Folder::~Folder()
{
	// Remove all documents explicitely, so that all sessions are closed,
	// and records finished.
	while(get_n_pages())
		remove_document(
			static_cast<SessionUserView*>(
				get_nth_page(0))->get_session_view());
}

// TODO: Share common code of add_text_session and add_chat_session
Gobby::TextSessionView&
Gobby::Folder::add_text_session(InfTextSession* session,
                                const Glib::ustring& title,
                                const Glib::ustring& path,
                                const Glib::ustring& hostname,
                                const std::string& info_storage_key)
{
	TextSessionView* view = Gtk::manage(
		new TextSessionView(session, title, path, hostname,
		                    info_storage_key, m_preferences,
		                    m_lang_manager));
	view->show();
	m_signal_document_added.emit(*view);

	SessionUserView* userview = Gtk::manage(
		new SessionUserView(
			*view, true,
			m_preferences.appearance.show_document_userlist,
			m_preferences.appearance.document_userlist_width));
	userview->show();

	TabLabel* tablabel = Gtk::manage(new TextTabLabel(*this, *view));
	tablabel->signal_close_request().connect(
		sigc::bind(
			sigc::mem_fun(*this, &Folder::on_tab_close_request),
			sigc::ref(*view)));
	tablabel->show();
	append_page(*userview, *tablabel);

	set_tab_reorderable(*userview, true);

	// Record the session, for debugging purposes:
	record(session, title);

	if(m_hide_single_tab && get_n_pages() > 1)
		set_show_tabs(true);
	return *view;
}

Gobby::ChatSessionView&
Gobby::Folder::add_chat_session(InfChatSession* session,
                                const Glib::ustring& title,
                                const Glib::ustring& path,
                                const Glib::ustring& hostname)
{
	ChatSessionView* view = Gtk::manage(
		new ChatSessionView(session, title, path, hostname,
		                    m_preferences));
	view->show();
	m_signal_document_added.emit(*view);

	SessionUserView* userview = Gtk::manage(
		new SessionUserView(
			*view, false,
			m_preferences.appearance.show_chat_userlist,
			m_preferences.appearance.chat_userlist_width));
	userview->show();

	TabLabel* tablabel = Gtk::manage(new ChatTabLabel(*this, *view));
	tablabel->signal_close_request().connect(
		sigc::bind(
			sigc::mem_fun(*this, &Folder::on_tab_close_request),
			sigc::ref(*view)));
	tablabel->show();
	append_page(*userview, *tablabel);

	set_tab_reorderable(*userview, true);
	if(m_hide_single_tab && get_n_pages() > 1)
		set_show_tabs(true);
	return *view;
}

void Gobby::Folder::remove_document(SessionView& view)
{
	m_signal_document_removed.emit(view);

	// Finish the record
	InfSession* session = view.get_session();
	g_object_set_data(G_OBJECT(session), "GOBBY_SESSION_RECORD", NULL);

	g_object_ref(session);
	inf_session_close(session);
	// This relies on the sessionuserview being the direct parent of
	// view - maybe we should make a loop here instead which searches
	// the folder in the widget hierarchy, to be more robust.
	remove_page(*view.get_parent());
	g_object_unref(session);

	if(get_n_pages() == 0)
		m_signal_document_changed.emit(NULL);

	if(m_hide_single_tab && get_n_pages() <= 1)
		set_show_tabs(false);
}

Gobby::SessionView*
Gobby::Folder::lookup_document(InfSession* session)
{
	const PageList& pagelist = pages();
	for(PageList::iterator iter = pagelist.begin();
	    iter != pagelist.end(); ++ iter)
	{
		SessionUserView* child =
			static_cast<SessionUserView*>(iter->get_child());

		if(child->get_session_view().get_session() == session)
			return &child->get_session_view();
	}

	return NULL;
}

Gobby::SessionView*
Gobby::Folder::get_current_document()
{
	SessionUserView* child = static_cast<SessionUserView*>(
		get_nth_page(get_current_page()));
	if(!child) return NULL;

	return &child->get_session_view();
}

const Gobby::SessionView*
Gobby::Folder::get_current_document() const
{
	const SessionUserView* child = static_cast<const SessionUserView*>(
		get_nth_page(get_current_page()));
	if(!child) return NULL;

	return &child->get_session_view();
}

void Gobby::Folder::switch_to_document(SessionView& document)
{
	// Again, here we rely on document being the direct child of
	// the SessionUserView...
	set_current_page(page_num(*document.get_parent()));
}

void Gobby::Folder::on_tab_close_request(SessionView& view)
{
	if(m_signal_document_close_request.emit(view))
		remove_document(view);
}

void Gobby::Folder::on_switch_page(GtkNotebookPage* page, guint page_num)
{
	Gtk::Notebook::on_switch_page(page, page_num);
	SessionUserView& view =
		*static_cast<SessionUserView*>(get_nth_page(page_num));

	m_signal_document_changed.emit(&view.get_session_view());
}

bool Gobby::Folder::on_key_press_event(GdkEventKey* event)
{
	static KeyMap keymap;

	if( (event->state & GDK_MOD1_MASK) == GDK_MOD1_MASK)
	{
		unsigned int page = keymap.lookup(event->keyval);
		if(page != KeyMap::nval)
		{
			set_current_page(page);
			return true;
		}
	}

	if( (event->state & (GDK_CONTROL_MASK | GDK_MOD1_MASK)) ==
	    (GDK_CONTROL_MASK | GDK_MOD1_MASK))
	{
		int offset = 0;
		if(event->keyval == GDK_Page_Up) offset = -1;
		if(event->keyval == GDK_Page_Down) offset = 1;

		if(offset != 0)
		{
			int res = get_current_page() + offset + get_n_pages();
			set_current_page(res % get_n_pages() );
			return true;
		}
	}

	return false;
}
