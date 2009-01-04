/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008 Armin Burgmeier <armin@arbur.net>
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
#include "core/tablabel.hpp"
#include "util/closebutton.hpp"
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

Gobby::Folder::Folder(Preferences& preferences,
                      GtkSourceLanguageManager* lang_manager):
	m_preferences(preferences), m_lang_manager(lang_manager)
{
	set_scrollable(true);
}

Gobby::Folder::~Folder()
{
	// Remove all documents explicitely, so that all sessions are closed,
	// and records finished.
	while(get_n_pages())
		remove_document(*static_cast<DocWindow*>(get_nth_page(0)));
}

Gobby::DocWindow&
Gobby::Folder::add_document(InfTextSession* session,
                            const Glib::ustring& title,
                            const std::string& info_storage_key)
{
	Gobby::DocWindow* window = Gtk::manage(
		new DocWindow(session, title, info_storage_key,
		              m_preferences, m_lang_manager));
	window->show();
	m_signal_document_added.emit(*window);

	TabLabel* tablabel = Gtk::manage(new TabLabel(*this, *window));
	tablabel->signal_close_request().connect(
		sigc::bind(
			sigc::mem_fun(*this, &Folder::on_tab_close_request),
			sigc::ref(*window)));
	tablabel->show();
	append_page(*window, *tablabel);

	set_tab_reorderable(*window, true);

	// Record the session, for debugging purposes:
	record(session, title);

	return *window;
}

void Gobby::Folder::remove_document(DocWindow& window)
{
	m_signal_document_removed.emit(window);

	// Finish the record
	InfTextSession* session = window.get_session();
	g_object_set_data(G_OBJECT(session), "GOBBY_SESSION_RECORD", NULL);

	g_object_ref(session);
	inf_session_close(INF_SESSION(session));
	remove_page(window);
	g_object_unref(session);

	if(get_n_pages() == 0)
		m_signal_document_changed.emit(NULL);
}

Gobby::DocWindow*
Gobby::Folder::lookup_document(InfTextSession* session)
{
	const PageList& pagelist = pages();
	for(PageList::iterator iter = pagelist.begin();
	    iter != pagelist.end(); ++ iter)
	{
		DocWindow* window =
			static_cast<DocWindow*>(iter->get_child());

		if(window->get_session() == session)
			return window;
	}

	return NULL;
}

Gobby::DocWindow*
Gobby::Folder::get_current_document()
{
	return static_cast<DocWindow*>(get_nth_page(get_current_page()));
}

const Gobby::DocWindow*
Gobby::Folder::get_current_document() const
{
	return static_cast<const DocWindow*>(get_nth_page(get_current_page()));
}

void Gobby::Folder::switch_to_document(DocWindow& document)
{
	set_current_page(page_num(document));
}

void Gobby::Folder::on_tab_close_request(DocWindow& window)
{
	if(m_signal_document_close_request.emit(window))
		remove_document(window);
}

void Gobby::Folder::on_switch_page(GtkNotebookPage* page, guint page_num)
{
	Gtk::Notebook::on_switch_page(page, page_num);
	DocWindow& window = *static_cast<DocWindow*>(get_nth_page(page_num));
	m_signal_document_changed.emit(&window);
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
