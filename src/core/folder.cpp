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

			g_signal_connect(
				document.get_text_view(), "notify::editable",
				G_CALLBACK(on_notify_editable_static), this);
			g_signal_connect(
				document.get_session(), "notify::status",
				G_CALLBACK(on_notify_status_static), this);
			g_signal_connect(
				document.get_session(),
				"notify::subscription-group",
				G_CALLBACK(
					on_notify_subscription_group_static),
				this);

			//m_label.set_ellipsize(Pango::ELLIPSIZE_END);
			m_label.show();

			m_button.set_relief(Gtk::RELIEF_NONE);
			m_button.set_focus_on_click(false);

			GtkRcStyle* rc_style = gtk_rc_style_new();
			rc_style->xthickness = rc_style->ythickness = 0;
			gtk_widget_modify_style(GTK_WIDGET(m_button.gobj()),
			                        rc_style);
			g_object_unref(rc_style);

			Gtk::Image* button_image = Gtk::manage(
				new Gtk::Image(Gtk::Stock::CLOSE,
				               Gtk::ICON_SIZE_MENU));
			button_image->show();
			m_button.add(*button_image);
			m_button.show();

			pack_start(m_icon, Gtk::PACK_SHRINK);
			pack_start(m_label, Gtk::PACK_EXPAND_WIDGET);
			pack_start(m_button, Gtk::PACK_SHRINK);
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
			printf("Notify subscr group\n");
			static_cast<TabLabel*>(data)->update_icon();
		}

		virtual void on_style_changed(
			const Glib::RefPtr<Gtk::Style>& previous_style)
		{
			int width, height;
			gtk_icon_size_lookup_for_settings(
				gtk_widget_get_settings(GTK_WIDGET(gobj())),
				GTK_ICON_SIZE_MENU, &width, &height);

			m_button.set_size_request(width + 2, height + 2);
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
		Gtk::Button m_button;
	};
}

Gobby::Folder::Folder(const Preferences& preferences,
                      GtkSourceLanguageManager* lang_manager):
	m_preferences(preferences), m_lang_manager(lang_manager)
{
	set_scrollable(true);
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

	TabLabel* tablabel = Gtk::manage(new TabLabel(*window, title));
	tablabel->signal_close_request().connect(
		sigc::bind(
			sigc::mem_fun(*this, &Folder::on_tab_close_request),
			sigc::ref(*window)));
	tablabel->show();
	append_page(*window, *tablabel);

	m_signal_document_added.emit(*window);
	return *window;
}

void Gobby::Folder::remove_document(DocWindow& window)
{
	m_signal_document_removed.emit(window);
	inf_session_close(INF_SESSION(window.get_session()));
	remove_page(window);
	// TODO: Emit document_changed with NULL DocWindow if this was the
	// last document, since switch_page is not called.
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

	return false;
}
