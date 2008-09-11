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

#include "core/folder.hpp"
#include "util/closebutton.hpp"
#include "util/file.hpp"

#include <gtkmm/box.h>
#include <gtkmm/stock.h>
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

	/* TODO: Put TabLabel into an extra source file */
	class TabLabel: public Gtk::HBox
	{
	public:
		typedef Glib::SignalProxy0<void> SignalCloseRequest;

		TabLabel(Gobby::DocWindow& document,
		         const Glib::ustring& title):
			Gtk::HBox(false, 4), m_document(document),
			m_title(title), m_label(title)
		{
			update_icon();
			m_icon.show();

			m_notify_editable_handle = g_signal_connect(
				document.get_text_view(), "notify::editable",
				G_CALLBACK(on_notify_editable_static), this);
			m_notify_status_handle = g_signal_connect(
				document.get_session(), "notify::status",
				G_CALLBACK(on_notify_status_static), this);
			m_notify_subscription_group_handle = g_signal_connect(
				document.get_session(),
				"notify::subscription-group",
				G_CALLBACK(
					on_notify_subscription_group_static),
				this);

			//m_label.set_ellipsize(Pango::ELLIPSIZE_END);
			m_label.show();
			m_button.show();

			pack_start(m_icon, Gtk::PACK_SHRINK);
			pack_start(m_label, Gtk::PACK_EXPAND_WIDGET);
			pack_start(m_button, Gtk::PACK_SHRINK);
		}

		~TabLabel()
		{
			g_signal_handler_disconnect(
				m_document.get_text_view(),
				m_notify_editable_handle);
			g_signal_handler_disconnect(
				m_document.get_session(),
				m_notify_status_handle);
			g_signal_handler_disconnect(
				m_document.get_session(),
				m_notify_subscription_group_handle);
		}

		SignalCloseRequest signal_close_request() {
			return m_button.signal_clicked();
		}

	private:
		static void on_notify_editable_static(GObject* object,
		                                      GParamSpec* pspec,
		                                      gpointer user_data)
		{
			static_cast<TabLabel*>(user_data)->update_icon();
		}

		static void on_notify_status_static(GObject* object,
		                                    GParamSpec* pspec,
		                                    gpointer user_data)
		{
			static_cast<TabLabel*>(user_data)->update_icon();
		}

		static void on_notify_subscription_group_static(GObject* obj,
		                                                GParamSpec* p,
		                                                gpointer data)
		{
			static_cast<TabLabel*>(data)->update_icon();
		}

		void update_icon()
		{
			InfTextSession* session = m_document.get_session();
			GtkTextView* view =
				GTK_TEXT_VIEW(m_document.get_text_view());

			if(inf_session_get_subscription_group(
				INF_SESSION(session)) == NULL)
			{
				m_icon.set(Gtk::Stock::DISCONNECT,
				           Gtk::ICON_SIZE_MENU);
				return;
			}

			switch(inf_session_get_status(INF_SESSION(session)))
			{
			case INF_SESSION_SYNCHRONIZING:
				m_icon.set(Gtk::Stock::EXECUTE,
				           Gtk::ICON_SIZE_MENU);
				break;
			case INF_SESSION_RUNNING:
				if(gtk_text_view_get_editable(view))
				{
					m_icon.set(Gtk::Stock::EDIT,
					           Gtk::ICON_SIZE_MENU);
				}
				else
				{
					m_icon.set(Gtk::Stock::FILE,
					           Gtk::ICON_SIZE_MENU);
				}

				break;
			case INF_SESSION_CLOSED:
				m_icon.set(Gtk::Stock::STOP,
				           Gtk::ICON_SIZE_MENU);
				break;
			}
		}

		Gobby::DocWindow& m_document;
		Glib::ustring m_title;

		Gtk::Image m_icon;
		Gtk::Label m_label;
		Gobby::CloseButton m_button;

		gulong m_notify_editable_handle;
		gulong m_notify_status_handle;
		gulong m_notify_subscription_group_handle;
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

	TabLabel* tablabel = Gtk::manage(new TabLabel(*window, title));
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

	// TODO: We should actually remove the page here, but destroy the
	// DocWindow after having emitted signal_document_changed.
	if(get_n_pages() == 1)
		m_signal_document_changed.emit(NULL);

	// Finish the record
	g_object_set_data(G_OBJECT(window.get_session()),
	                  "GOBBY_SESSION_RECORD", NULL);

	InfTextSession* session = window.get_session();
	g_object_ref(session);

	inf_session_close(INF_SESSION(session));
	remove_page(window);

	g_object_unref(session);
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

#if 0
	// Is already used by GtkTextView...
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
#endif

	return false;
}
