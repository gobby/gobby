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

#include <stdexcept>
#include <gtkmm/box.h>
#include <gtkmm/stock.h>

#include "document.hpp"
#include "folder.hpp"

namespace
{
	class KeyMap: private net6::non_copyable
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
}

Gobby::Folder::TabLabel::TabLabel(const Glib::ustring& label)
 : m_image(Gtk::Stock::CLOSE, Gtk::ICON_SIZE_MENU), m_label(label),
   m_modified("")
{
	// Lookup icon size
	int width, height;
	Gtk::IconSize::lookup(Gtk::ICON_SIZE_MENU, width, height);

	// Resize button to image's size
	m_button.set_size_request(width + 4, height + 4);
	m_button.add(m_image);
	m_button.set_relief(Gtk::RELIEF_NONE);

	// Add box
	set_spacing(5);
	pack_start(m_modified, Gtk::PACK_SHRINK);
	pack_start(m_label, Gtk::PACK_SHRINK);
	pack_start(m_button, Gtk::PACK_SHRINK);
	show_all();
}

Glib::ustring Gobby::Folder::TabLabel::get_label() const
{
	return m_label.get_text();
}

void Gobby::Folder::TabLabel::set_close_sensitive(bool sensitive)
{
	m_button.set_sensitive(sensitive);
}

void Gobby::Folder::TabLabel::set_modified(bool modified)
{
	if(modified)
		m_modified.set_text("*");
	else
		m_modified.set_text("");
}

void Gobby::Folder::TabLabel::set_label(const Glib::ustring& label)
{
	m_label.set_text(label);
}

void Gobby::Folder::TabLabel::set_use_markup(bool setting)
{
	m_label.set_use_markup(setting);
}

Gobby::Folder::TabLabel::close_signal_type
Gobby::Folder::TabLabel::close_event() 
{
	return m_button.signal_clicked();
}

Gobby::Folder::Folder(Header& header,
                      const Preferences& preferences):
	Gtk::Notebook(),
	m_block_language(false), m_header(header), m_preferences(preferences),
	m_buffer(NULL)
{
	set_scrollable(true);

	for(std::list<Header::LanguageWrapper>::const_iterator iter =
		m_header.action_edit_syntax_languages.begin();
	    iter != m_header.action_edit_syntax_languages.end();
	    ++ iter)
	{
		iter->get_action()->signal_activate().connect(
			sigc::bind(
				sigc::mem_fun(
					*this,
					&Folder::on_language_changed
				),
				iter->get_language()
			)
		);
	}

	add_events(Gdk::KEY_PRESS_MASK);
}

#if 0
Glib::RefPtr<const Gtk::SourceLanguagesManager>
Gobby::Folder::get_lang_manager() const
{
	return m_header.get_lang_manager();
}
#endif

void Gobby::Folder::obby_start(LocalBuffer& buf)
{
	// Remove existing pages from older session
	while(get_n_pages() )
		remove_page(0);

	set_sensitive(true);
	m_buffer = &buf;
}

void Gobby::Folder::obby_end()
{
	m_buffer = NULL;

	// Insensitive just the text editor to allow to scroll and tab between
	// the documents
	for(int i = 0; i < get_n_pages(); ++ i)
		static_cast<DocWindow*>(get_nth_page(i) )->disable();
}

void Gobby::Folder::obby_user_join(const obby::user& user)
{
}

void Gobby::Folder::obby_user_part(const obby::user& user)
{
}

void Gobby::Folder::obby_user_colour(const obby::user& user)
{
}

namespace
{
	// Escape special HTML entities to prevent that the tab label gets
	// messed
	// TODO: I think this type signature is just plainly wrong -- phil
	std::string escapehtml(std::string str)
	{
		std::string::size_type pos = 0;
		while( (pos = str.find_first_of("&<>", pos))
			!= std::string::npos)
		{
			switch(str[pos])
			{
			case '&':
				str.replace(pos, 1, "&amp;");
				break;
			case '<':
				str.replace(pos, 1, "&lt;");
				break;
			case '>':
				str.replace(pos, 1, "&gt;");
			}
			++pos;
		}
		return str;
	}
}

void Gobby::Folder::obby_document_insert(LocalDocumentInfo& document)
{
	// Document subscription handling
	document.subscribe_event().connect(
		sigc::bind(
			sigc::mem_fun(*this, &Folder::on_document_subscribe),
			sigc::ref(document)
		)
	);

	document.unsubscribe_event().connect(
		sigc::bind(
			sigc::mem_fun(*this, &Folder::on_document_unsubscribe),
			sigc::ref(document)
		)
	);
}

void Gobby::Folder::obby_document_remove(LocalDocumentInfo& document)
{
	// Nothing to do, the unsubscription signal handler has already removed
	// the page from the notebook.
}

// Signals
Gobby::Folder::signal_document_add_type
Gobby::Folder::document_add_event() const
{
	return m_signal_document_add;
}

Gobby::Folder::signal_document_remove_type
Gobby::Folder::document_remove_event() const
{
	return m_signal_document_remove;
}

Gobby::Folder::signal_document_close_request_type
Gobby::Folder::document_close_request_event() const
{
	return m_signal_document_close_request;
}

Gobby::Folder::signal_document_cursor_moved_type
Gobby::Folder::document_cursor_moved_event() const
{
	return m_signal_document_cursor_moved;
}

Gobby::Folder::signal_document_content_changed_type
Gobby::Folder::document_content_changed_event() const
{
	return m_signal_document_content_changed;
}

Gobby::Folder::signal_document_language_changed_type
Gobby::Folder::document_language_changed_event() const
{
	return m_signal_document_language_changed;
}

Gobby::Folder::signal_tab_switched_type
Gobby::Folder::tab_switched_event() const
{
	return m_signal_tab_switched;
}

void Gobby::Folder::set_tab_colour(DocWindow& win, const Glib::ustring& colour)
{
	TabLabel& label = *static_cast<TabLabel*>(get_tab_label(win) );
	label.set_label("<span foreground=\"" + colour + "\">" +
		escapehtml(label.get_label() ) + "</span>");
	label.set_use_markup(true);
}

void Gobby::Folder::on_language_changed(const Glib::RefPtr<Gtk::SourceLanguage>& language)
{
	// TODO: Gobby::Block
	if(m_block_language) return;

	m_block_language = true;
	Gtk::Widget* wnd = get_nth_page(get_current_page() );

	// wnd should not be NULL because the language radio items are only
	// enabled if windows are open
	if(wnd == NULL)
		throw std::logic_error("Gobby::Folder::on_language_changed");

	static_cast<DocWindow*>(wnd)->set_language(language);
	m_block_language = false;
}

void Gobby::Folder::on_document_modified_changed(DocWindow& window)
{
	// Get tab label for this document
	TabLabel& label = *static_cast<TabLabel*>(get_tab_label(window) );
	// Show asterisk as the document's title if it has been modified
	label.set_modified(window.get_modified() );
}

void Gobby::Folder::on_document_close(DocWindow& window)
{
	m_signal_document_close_request.emit(window);
}

void Gobby::Folder::on_document_subscribe(const obby::user& user,
                                          LocalDocumentInfo& info)
{
	if(&info.get_buffer().get_self() == &user)
		on_self_subscribe(info);
}

void Gobby::Folder::on_self_subscribe(LocalDocumentInfo& info)
{
	// Create new document
	DocWindow* new_wnd =
		Gtk::manage(new DocWindow(info, m_preferences) );

	// Watch update signal to emit document_updated signal if a document
	// has been updated.
	new_wnd->cursor_moved_event().connect(
		sigc::bind(
			sigc::mem_fun(*this, &Folder::on_document_cursor_moved),
			sigc::ref(*new_wnd)
		)
	);

	new_wnd->content_changed_event().connect(
		sigc::bind(
			sigc::mem_fun(
				*this,
				&Folder::on_document_content_changed
			),
			sigc::ref(*new_wnd)
		)
	);

	new_wnd->language_changed_event().connect(
		sigc::bind(
			sigc::mem_fun(
				*this,
				&Folder::on_document_language_changed
			),
			sigc::ref(*new_wnd)
		)
	);

	new_wnd->get_document().get_buffer()->signal_modified_changed().connect(
		sigc::bind(
			sigc::mem_fun(
				*this,
				&Folder::on_document_modified_changed
			),
			sigc::ref(*new_wnd)
		)
	);

	// Create label for the tab
	TabLabel* label = Gtk::manage(
		new TabLabel(escapehtml(info.get_suffixed_title()))
	);

	label->set_use_markup(true);
	label->set_modified(new_wnd->get_modified() );

	// Connect close event
	label->close_event().connect(
		sigc::bind(
			sigc::mem_fun(*this, &Folder::on_document_close),
			sigc::ref(*new_wnd)
		)
	);

	// Append document's title as new page to the notebook
	append_page(*new_wnd, *label);

	// Show child
	new_wnd->show_all();

	// Emit add_document signal
	m_signal_document_add.emit(*new_wnd);
}

void Gobby::Folder::on_document_unsubscribe(const obby::user& user,
                                            LocalDocumentInfo& info)
{
	if(&info.get_buffer().get_self() == &user)
		on_self_unsubscribe(info);
}

void Gobby::Folder::on_self_unsubscribe(LocalDocumentInfo& info)
{
	DocWindow* wnd = NULL;
	for(int i = 0; i < get_n_pages(); ++ i)
	{
		DocWindow* cur_wnd = static_cast<DocWindow*>(get_nth_page(i) );
		if(&cur_wnd->get_info() == &info)
		{
			wnd = cur_wnd;
			break;
		}
	}

	if(wnd == NULL)
		throw std::logic_error("Gobby::Folder::on_self_unsubscribe");

	// Remove page from notebook
	remove_page(*wnd);
	// Emit remove_document signal (TODO: Check that wnd still exists, it
	// is Gtk::managed
	m_signal_document_remove.emit(*wnd);

	// The next time the user subscribes the DocWindow will be
	// recreated
	m_conn_unsubscribe.disconnect();
}

void Gobby::Folder::on_switch_page(GtkNotebookPage* page, guint page_num)
{
	Gtk::Notebook::on_switch_page(page, page_num);

	// Do only update statusbar if an obby session is running. A switch_page
	// event is triggered if the currently visible page is removed and
	// anotherone is shown. This may be the case after the obby session
	// has been closed. Therefore, the corresponding obby::documents do
	// not exist anymore, and updating the statusbar accesses these.
	DocWindow& window = *static_cast<DocWindow*>(get_nth_page(page_num));

	// However, if the obby session has been closed the statusbar is empty,
	// there is no need to update anything.
	Glib::RefPtr<Gtk::SourceLanguage> language = window.get_language();

	// Set correct menu item
	if(!m_block_language)
	{
		m_block_language = true;
		for(std::list<Header::LanguageWrapper>::const_iterator iter =
			m_header.action_edit_syntax_languages.begin();
		    iter != m_header.action_edit_syntax_languages.end();
		    ++ iter)
		{
			if(iter->get_language() == language)
				iter->get_action()->set_active(true);

		}
		m_block_language = false;
	}

	if(m_buffer != NULL)
	{
		// Another document has been selected: Emit tabswitched
		m_signal_tab_switched.emit(window);
	}

	// TODO: We should put flags into the labels which specify if the
	// current document is modified.

	// Reset tab colour from red indicating that the user has read the
	// changes in this document.
	set_tab_colour(window, "#000000");
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

void Gobby::Folder::on_document_cursor_moved(DocWindow& window)
{
	// Update in the currently visible document? Cursor position has moved.
	Gtk::Widget* wnd = get_nth_page(get_current_page() );
	if(wnd == &window)
		m_signal_document_cursor_moved.emit(window);
}

void Gobby::Folder::on_document_content_changed(DocWindow& window)
{
	Gtk::Widget* wnd = get_nth_page(get_current_page() );
	if(wnd == &window)
	{
		// Update in the currently visible document? Update statusbar.
		m_signal_document_content_changed.emit(window);
	}
	else
	{
		// Show red tab colour otherwise indicating that someone edited
		// this document.
		set_tab_colour(window, "#CC0000");
	}
}

void Gobby::Folder::on_document_language_changed(DocWindow& window)
{
	// Update in the currently visible document? Update statusbar.
	Gtk::Widget* wnd = get_nth_page(get_current_page() );
	if(wnd == &window)
		m_signal_document_language_changed.emit(window);
}

void Gobby::Folder::select_document(const LocalDocumentInfo& info)
{
	for(int i = 0; i < get_n_pages(); ++i)
	{
		DocWindow* win = static_cast<DocWindow*>(get_nth_page(i));

		if(&info == &win->get_info() )
		{
			set_current_page(i);
			break;
		}
	}
}
