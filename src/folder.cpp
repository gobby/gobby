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

#include "folder.hpp"

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/stock.h>
#include <gdk/gdkkeysyms.h>
#include <stdexcept>

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

	class TabLabel: public Gtk::HBox
	{
	public:
		typedef Glib::SignalProxy0<void> SignalCloseRequest;

		TabLabel(InfTextSession* session, const Glib::ustring& title):
			m_session(session), m_title(title), m_label(title),
			m_button(Gtk::Stock::CLOSE)
		{
			m_label.set_ellipsize(Pango::ELLIPSIZE_END);
			m_label.show();

			m_icon.show();
			m_button.show();

			pack_start(m_icon, Gtk::PACK_SHRINK);
			pack_start(m_label, Gtk::PACK_EXPAND_WIDGET);
			pack_end(m_button, Gtk::PACK_SHRINK);
		}

		SignalCloseRequest signal_close_request() {
			return m_button.signal_clicked();
		}

	private:
		InfTextSession* m_session;
		Glib::ustring m_title;

		Gtk::Image m_icon;
		Gtk::Label m_label;
		Gtk::Button m_button;
	};
}

Gobby::Folder::Folder(const Preferences& preferences,
                      GtkSourceLanguageManager* lang_manager):
	m_preferences(preferences), m_lang_manager(lang_manager)
{
	set_scrollable(true);
}

Gobby::DocWindow& Gobby::Folder::add_document(InfTextSession* session,
                                              const Glib::ustring& title)
{
	Gobby::DocWindow* window = Gtk::manage(
		new DocWindow(session, title, m_preferences, m_lang_manager));

	TabLabel* tablabel = Gtk::manage(new TabLabel(session, title));
	tablabel->signal_close_request().connect(
		sigc::bind(
			sigc::mem_fun(*this, &Folder::on_tab_close_request),
			sigc::ref(*window)));
	append_page(*window, *tablabel);

	m_signal_document_added.emit(*window);
	return *window;
}

void Gobby::Folder::remove_document(DocWindow& window)
{
	m_signal_document_removed.emit(window);
	remove_page(window);
	// TODO: Emit document_changed with NULL DocWindow if this was the
	// last document, since switch_page is not called.
}

Gobby::DocWindow*
Gobby::Folder::lookup_document(InfTextSession* session)
{
	for(unsigned int i = 0; i < get_n_pages(); ++ i)
	{
		DocWindow* window = static_cast<DocWindow*>(get_nth_page(i));
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

	// Is already used by GtkTextTextView...
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
