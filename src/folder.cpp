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

#include <gtkmm/box.h>
#include <gtkmm/stock.h>

#include "document.hpp"
#include "folder.hpp"

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

Gobby::Folder::TabLabel::~TabLabel()
{
}

Glib::ustring Gobby::Folder::TabLabel::get_label() const
{
	return m_label.get_text();
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

v v v v v v v
Gobby::Folder::Folder(const Preferences& preferences)
v v v v v v v
 : Gtk::Notebook(), m_preferences(preferences), m_running(false)
*************
v v v v v v v
Gobby::Folder::Folder()
 : Gtk::Notebook(), m_running(false),
   m_lang_manager(Gtk::SourceLanguagesManager::create() )
*************
Gobby::Folder::Folder(const Preferences& preferences)
 : Gtk::Notebook(), m_preferences(preferences), m_running(false)
#ifdef WITH_GTKSOURCEVIEW
   , m_lang_manager(Gtk::SourceLanguagesManager::create() )
#endif
^ ^ ^ ^ ^ ^ ^
^ ^ ^ ^ ^ ^ ^
*************
 : Gtk::Notebook(), m_preferences(preferences), m_running(false),
   m_lang_manager(Gtk::SourceLanguagesManager::create() )
^ ^ ^ ^ ^ ^ ^
{
}

Gobby::Folder::~Folder()
{
}

const Gobby::MimeMap& Gobby::Folder::get_mime_map() const
{
	return m_mime_map;
}

Glib::RefPtr<Gtk::SourceLanguagesManager>
Gobby::Folder::get_lang_manager() const
{
	return m_lang_manager;
}

void Gobby::Folder::obby_start(obby::local_buffer& buf)
{
	while(get_n_pages() )
		remove_page(0);

	set_sensitive(true);
	m_running = true;
}

void Gobby::Folder::obby_end()
{
	// Insensitive just the text editor to allow to scroll and tab between
	// the documents
	for(unsigned int i = 0; i < get_n_pages(); ++ i)
		static_cast<DocWindow*>(
			get_nth_page(i)
		)->get_document().set_sensitive(false);

	m_running = false;
}

void Gobby::Folder::obby_user_join(obby::user& user)
{
	// Pass user join event to documents
	for(unsigned int i = 0; i < get_n_pages(); ++ i)
		static_cast<DocWindow*>(get_nth_page(i) )->obby_user_join(user);
}

void Gobby::Folder::obby_user_part(obby::user& user)
{
	// Pass user part event to documents
	for(unsigned int i = 0; i < get_n_pages(); ++ i)
		static_cast<DocWindow*>(get_nth_page(i) )->obby_user_part(user);
}

void Gobby::Folder::obby_user_colour(obby::user& user)
{
	// Pass user colour event to documents
	for(unsigned int i = 0; i < get_n_pages(); ++ i)
		static_cast<DocWindow*>(
			get_nth_page(i) )->obby_user_colour(user);
}

void Gobby::Folder::obby_document_insert(obby::local_document_info& document)
{
	// Create new document
	DocWindow* new_wnd =
		Gtk::manage(new DocWindow(document, *this, m_preferences) );
	Document& new_doc = new_wnd->get_document();

	// Watch update signal to emit document_updated signal if a document
	// has been updated.
	new_doc.cursor_moved_event().connect(
		sigc::bind(
			sigc::mem_fun(*this, &Folder::on_document_cursor_moved),
			sigc::ref(new_doc)
		)
	);

	new_doc.content_changed_event().connect(
		sigc::bind(
			sigc::mem_fun(
				*this,
				&Folder::on_document_content_changed
			),
			sigc::ref(*new_wnd)
		)
	);

	new_doc.language_changed_event().connect(
		sigc::bind(
			sigc::mem_fun(
				*this,
				&Folder::on_document_language_changed
			),
			sigc::ref(new_doc)
		)
	);

	new_doc.get_buffer()->signal_modified_changed().connect(
		sigc::bind(
			sigc::mem_fun(
				*this,
				&Folder::on_document_modified_changed
			),
			sigc::ref(*new_wnd)
		)
	);

	// Create label for the tab
	TabLabel* label = Gtk::manage(new TabLabel(
		"<span foreground=\"#666666\">" + document.get_title()
		+ "</span>"));
	label->set_use_markup();

	// Connect close event
	label->close_event().connect(
		sigc::bind(
			sigc::mem_fun(*this, &Folder::on_document_close),
			sigc::ref(new_doc)
		)
	);

	// Document subscription handling
	document.subscribe_event().connect(
		sigc::bind(
			sigc::mem_fun(*this, &Folder::on_document_subscribe),
			sigc::ref(*new_wnd)
		)
	);

	document.unsubscribe_event().connect(
		sigc::bind(
			sigc::mem_fun(*this, &Folder::on_document_unsubscribe),
			sigc::ref(*new_wnd)
		)
	);

	// Append document's title as new page to the notebook
	append_page(*new_wnd, *label);

	// Show child
	new_wnd->show_all();
}

void Gobby::Folder::obby_document_remove(obby::local_document_info& document)
{
	// Find corresponding Document widget in notebook
	for(int i = 0; i < get_n_pages(); ++ i)
	{
		DocWindow* doc = static_cast<DocWindow*>(get_nth_page(i) );
		obby::local_document_info& obdoc =
			doc->get_document().get_document();

		if(&obdoc == &document)
		{
			// Remove page
			remove_page(*doc);
			break;
		}
	}
}

// Signals
Gobby::Folder::signal_document_close_type
Gobby::Folder::document_close_event() const
{
	return m_signal_document_close;
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
		label.get_label() + "</span>");
	label.set_use_markup();	
}

void Gobby::Folder::on_document_modified_changed(DocWindow& window)
{
	// Get tab label for this document
	TabLabel& label = *static_cast<TabLabel*>(get_tab_label(window) );
	Document& doc = window.get_document();
	// Show asterisk as the document's title if it has been modified
	label.set_modified(doc.get_buffer()->get_modified() );
}

void Gobby::Folder::on_document_close(Document& document)
{
	m_signal_document_close.emit(document);
}

void Gobby::Folder::on_document_subscribe(const obby::user& user,
                                          DocWindow& window)
{
	if(&window.get_document().get_document().get_buffer().get_self() ==
	   &user)
		set_tab_colour(window, "#000000");
}

void Gobby::Folder::on_document_unsubscribe(const obby::user& user,
                                            DocWindow& window)
{
	if(&window.get_document().get_document().get_buffer().get_self() ==
	   &user)
	   	set_tab_colour(window, "#555555");
}

void Gobby::Folder::on_switch_page(GtkNotebookPage* page, guint page_num)
{
	// Do only update statusbar if an obby session is running. A switch_page
	// event is triggered if the currently visible page is removed and
	// anotherone is shown. This may be the case after the obby session
	// has been closed. Therefore, the corresponding obby::documents do
	// not exist anymore, and updating the statusbar accesses these.

	// However, if the obby session has been closed the statusbar is empty,
	// there is no need to update anything.
	if(m_running)
	{
		// Another document has been selected: Emit tabswitched
		m_signal_tab_switched.emit(
			static_cast<DocWindow*>(
				get_nth_page(page_num)
			)->get_document()
		);
	}

	// TODO: We should put flags into the labels which specify if the
	// current document is subscribed and/or modified.
	DocWindow& window = *static_cast<DocWindow*>(get_nth_page(page_num));
	obby::local_document_info& docinfo =
		window.get_document().get_document();
	if(docinfo.is_subscribed(docinfo.get_buffer().get_self() ))
		set_tab_colour(window, "#000000");
	Gtk::Notebook::on_switch_page(page, page_num);
}

void Gobby::Folder::on_document_cursor_moved(Document& document)
{
	// Update in the currently visible document? Update statusbar.
	Gtk::Widget* wnd = get_nth_page(get_current_page() );
	if(&static_cast<DocWindow*>(wnd)->get_document() == &document)
		m_signal_document_cursor_moved.emit(document);
}

void Gobby::Folder::on_document_content_changed(DocWindow& window)
{
	// Update in the currently visible document? Update statusbar.
	Gtk::Widget* wnd = get_nth_page(get_current_page() );
	if(wnd == &window)
		m_signal_document_content_changed.emit(window.get_document());
	else
	{
		set_tab_colour(window, "#CC0000");
	}
}

void Gobby::Folder::on_document_language_changed(Document& document)
{
	// Update in the currently visible document? Update statusbar.
	Gtk::Widget* wnd = get_nth_page(get_current_page() );
	if(&static_cast<DocWindow*>(wnd)->get_document() == &document)
		m_signal_document_language_changed.emit(document);
}



