/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2013 Armin Burgmeier <armin@arbur.net>
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

#include "features.hpp"
#include "window.hpp"

#include "commands/file-tasks/task-open.hpp"
#include "commands/file-tasks/task-open-multiple.hpp"
#include "core/iconmanager.hpp"
#include "core/noteplugin.hpp"

#include "util/i18n.hpp"
#include "util/gtk-compat.hpp"

#include <gtkmm/frame.h>

Gobby::Window::Window(unsigned int argc, const char* const argv[],
                      Config& config,
                      Preferences& preferences,
                      const IconManager& icon_manager,
                      CertificateManager& cert_manager
#ifdef WITH_UNIQUE
                      , UniqueApp* app
#endif
                      ):
	Gtk::Window(Gtk::WINDOW_TOPLEVEL),
	m_argc(argc), m_argv(argv), m_config(config),
	m_lang_manager(gtk_source_language_manager_get_default()),
	m_preferences(preferences), m_cert_manager(cert_manager),
	m_icon_manager(icon_manager),
#ifdef WITH_UNIQUE
	m_app(app),
#endif
	m_header(m_preferences, m_lang_manager),
	m_browser(*this, Plugins::TEXT, m_statusbar, m_cert_manager,
	          m_preferences),
	m_text_folder(false, m_preferences, m_lang_manager),
	m_chat_folder(true, m_preferences, m_lang_manager),
	m_chat_frame(_("Chat"), IconManager::STOCK_CHAT,
	             m_preferences.appearance.show_chat),
	m_statusbar(*this, m_text_folder, m_preferences),
	m_info_storage(INF_GTK_BROWSER_MODEL(m_browser.get_store())),
	m_operations(m_info_storage, m_browser, m_statusbar),
	m_browser_commands(m_browser, m_text_folder, m_statusbar),
	m_browser_context_commands(*this, m_browser, m_file_chooser,
	                           m_operations, m_preferences),
	m_auth_commands(*this, m_browser, m_statusbar, m_preferences),
	m_autosave_commands(m_text_folder, m_operations, m_info_storage,
	                    m_preferences),
	m_subscription_commands(m_browser, m_text_folder,
	                        m_chat_folder, m_info_storage),
	m_synchronization_commands(m_subscription_commands),
	m_user_join_commands(m_subscription_commands, m_preferences),
	m_text_folder_commands(m_text_folder),
	m_chat_folder_commands(m_chat_folder),
	m_file_commands(*this, m_header, m_browser, m_text_folder,
	                m_statusbar, m_file_chooser, m_operations,
	                m_info_storage, m_preferences),
	m_edit_commands(*this, m_header, m_text_folder, m_statusbar,
	                m_preferences, m_cert_manager),
	m_view_commands(m_header, m_text_folder, m_chat_frame, m_chat_folder,
	                m_preferences),
	m_help_commands(*this, m_header, m_icon_manager),
	m_title_bar(*this, m_text_folder)
{
#ifdef WITH_UNIQUE
	g_object_ref(app);

	unique_app_watch_window(app, gobj());
	g_signal_connect(app, "message-received",
	                 G_CALLBACK(on_message_received_static), this);
#endif // WITH_UNIQUE

	m_chat_frame.signal_show().connect(
		sigc::mem_fun(*this, &Window::on_chat_show), true);
	m_chat_frame.signal_hide().connect(
		sigc::mem_fun(*this, &Window::on_chat_hide), false);

	m_header.show();
	m_browser.show();
	m_text_folder.show();
	m_chat_folder.show();

	// Build UI
	Glib::RefPtr<Gtk::AccelGroup> group = m_header.get_accel_group();
	// Add focus shortcuts; unfortunately gtkmm does not wrap that API
	GClosure* closure = g_cclosure_new(
		G_CALLBACK(on_switch_to_chat_static), this, NULL);
	gtk_accel_group_connect(group->gobj(), GDK_KEY_m, GDK_CONTROL_MASK,
	                        static_cast<GtkAccelFlags>(0), closure);
	//g_closure_unref(closure);
	GClosure* closure2 = g_cclosure_new(
		G_CALLBACK(on_switch_to_text_static), this, NULL);
	gtk_accel_group_connect(group->gobj(), GDK_KEY_m,
	                        static_cast<GdkModifierType>(
					GDK_CONTROL_MASK | GDK_SHIFT_MASK),
	                        static_cast<GtkAccelFlags>(0), closure2);
	//g_closure_unref(closure2);
	add_accel_group(group);

	Gtk::Frame* frame_browser = Gtk::manage(new ClosableFrame(
		_("Document Browser"), IconManager::STOCK_DOCLIST,
		m_preferences.appearance.show_browser));
	frame_browser->set_shadow_type(Gtk::SHADOW_IN);
	frame_browser->add(m_browser);
	// frame_browser manages visibility itself

	Gtk::Frame* frame_text = Gtk::manage(new Gtk::Frame);
	frame_text->set_shadow_type(Gtk::SHADOW_IN);
	frame_text->add(m_text_folder);
	frame_text->show();

	m_chat_frame.set_shadow_type(Gtk::SHADOW_IN);
	m_chat_frame.add(m_chat_folder);
	// frame_chat manages visibility itself

	m_chat_paned.pack1(*frame_text, true, false);
	m_chat_paned.pack2(m_chat_frame, false, false);
	m_chat_paned.show();

	m_paned.pack1(*frame_browser, false, false);
	m_paned.pack2(m_chat_paned, true, false);
	m_paned.show();

	m_mainbox.pack_start(m_header, Gtk::PACK_SHRINK);
	m_mainbox.pack_start(m_paned, Gtk::PACK_EXPAND_WIDGET);
	m_mainbox.pack_start(m_statusbar, Gtk::PACK_SHRINK);
	m_mainbox.show();

	// Give initial focus to the browser, which will in turn give focus
	// to the "Direct Connection" expander, so people can quickly
	// get going.
	set_focus_child(m_browser);
	add(m_mainbox);

	set_default_size(800, 600);
	set_role("Gobby");
}

Gobby::Window::~Window()
{
#ifdef WITH_UNIQUE
	g_object_unref(m_app);
#endif
}

bool Gobby::Window::on_delete_event(GdkEventAny* event)
{
#if 0
	if(m_buffer.get() == NULL) return false;
	if(!m_buffer->is_open() ) return false;

	Gtk::MessageDialog dlg(
		*this,
		_("You are still connected to a session"),
		false,
		Gtk::MESSAGE_WARNING,
		Gtk::BUTTONS_NONE,
		true
	);

	dlg.set_secondary_text(
		_("Do you want to close Gobby nevertheless?")
	);

	Gtk::Image* img = Gtk::manage(new Gtk::Image(Gtk::Stock::CANCEL,
	                                             Gtk::ICON_SIZE_BUTTON));
	Gtk::Button* cancel_button
		= dlg.add_button(_("C_ancel"), Gtk::RESPONSE_CANCEL);
	dlg.add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_YES);
	cancel_button->set_image(*img);
	cancel_button->grab_focus();

	return dlg.run() != Gtk::RESPONSE_YES;
#endif
	return false;
}

// GtkWindow catches keybindings for the menu items _before_ passing them to
// the focused widget. This is unfortunate and means that pressing ctrl+V
// in an entry on the browser ends up pasting text in the TextView.
// Here we override GtkWindow's handler to do the same things that it
// does, but in the opposite order and then we chain up to the grand
// parent handler, skipping Gtk::Window::key_press_event().
// This code is basically stolen from gedit, but ported to C++.
bool Gobby::Window::on_key_press_event(GdkEventKey* event)
{
	// We can't let GtkSourceView handle this, since we override
	// Undo/Redo. TODO: This is a bit of a hack. A proper solution would
	// perhaps be to subclass GtkSourceView, and either
	// unregister/reregister the keybinding there, or making sure the
	// key-press-event default handler returns false.
	// TODO: An even properer solution is to use the new GtkSourceView3
	// feature which allows to plug a different undo manager to the
	// SourceView. This would also make the Undo/Redo context menu items
	// work.
	if(event->keyval == GDK_KEY_z || event->keyval == GDK_KEY_Z)
		return Gtk::Window::on_key_press_event(event);

	bool handled = gtk_window_propagate_key_event(gobj(), event);
	if(!handled) handled = gtk_window_activate_key(gobj(), event);

	// Skip Gtk::Window default handler here:
	if(!handled) handled = Gtk::Container::on_key_press_event(event);

	return handled;
}

void Gobby::Window::on_realize()
{
	Gtk::Window::on_realize();

	m_paned.set_position(m_paned.get_width() * 2 / 5);
	m_chat_paned.set_position(m_chat_paned.get_height() * 7 / 10);
}

void Gobby::Window::on_show()
{
	Gtk::Window::on_show();

	if(!m_config.get_root()["initial"].get_value<bool>("run", false))
	{
		m_initial_dlg.reset(new InitialDialog(*this, m_preferences,
		                                      m_icon_manager));
		m_initial_dlg->present();
		m_initial_dlg->signal_hide().connect(
			sigc::mem_fun(*this,
			              &Window::on_initial_dialog_hide));
	}

	// Open files passed on the command line
	// TODO: Only do this when the Window is shown the first time
	if(m_argc == 1)
	{
		Glib::RefPtr<Gio::File> file(
			Gio::File::create_for_commandline_arg(m_argv[0]));
		m_file_commands.set_task(new TaskOpen(m_file_commands, file));
	}
	else if(m_argc > 1)
	{
		TaskOpenMultiple* task =
			new TaskOpenMultiple(m_file_commands);

		const char* const* arg = m_argv;
		do
		{
			Glib::RefPtr<Gio::File> file(
				Gio::File::create_for_commandline_arg(*arg));
			task->add_file(file->get_uri());
		} while(*++arg);

		m_file_commands.set_task(task);
	}

}

void Gobby::Window::on_initial_dialog_hide()
{
	m_initial_dlg.reset(NULL);
	// Don't show again
	m_config.get_root()["initial"].set_value("run", true);
}

bool Gobby::Window::on_switch_to_chat()
{
	SessionView* view = m_chat_folder.get_current_document();
	if(!view) return false;

	ChatSessionView* chat_view = dynamic_cast<ChatSessionView*>(view);
	if(!chat_view) return false;

	m_preferences.appearance.show_chat = true;
	InfGtkChat* chat = chat_view->get_chat();
	GtkWidget* entry = inf_gtk_chat_get_entry(chat);
	gtk_widget_grab_focus(GTK_WIDGET(entry));
	return true;
}

bool Gobby::Window::on_switch_to_text()
{
	SessionView* view = m_text_folder.get_current_document();
	if(!view) return false;

	TextSessionView* text_view = dynamic_cast<TextSessionView*>(view);
	if(!text_view) return false;

	GtkSourceView* gtk_view = text_view->get_text_view();
	gtk_widget_grab_focus(GTK_WIDGET(gtk_view));
	// TODO: Turn chat back off if previously activated
	// via on_switch_to_chat()?
	return true;
}

void Gobby::Window::on_chat_hide()
{
	Gtk::Widget* focus = get_focus();
	// Actually this always returns NULL if m_chat_frame has focus,
	// because the focus is removed again. I think it's good enough
	// though.
	if(focus == NULL || focus == &m_chat_frame ||
	   focus->is_ancestor(m_chat_frame))
	{
		on_switch_to_text();
	}
}

void Gobby::Window::on_chat_show()
{
	Gtk::Widget* focus = get_focus();
	if(!focus) on_switch_to_chat();
}

#ifdef WITH_UNIQUE
UniqueResponse Gobby::Window::on_message_received(UniqueCommand command,
                                                  UniqueMessageData* message,
                                                  guint time)
try
{
	struct uris_holder
	{
		uris_holder(gchar** uris): uris(uris) {}
		~uris_holder() { if(uris) g_strfreev(uris); }
		operator gchar* const*() const { return uris; }

		gchar** uris;

	private:
		uris_holder(const uris_holder&);
		uris_holder& operator=(const uris_holder&);
	};

	// Cast to int to suppress a warning about UNIQUE_GOBBY_CONNECT not
	// being a member of the UniqueCommand enum.
	switch (static_cast<signed int>(command))
	{
	case UNIQUE_ACTIVATE:
		gtk_window_set_screen(gobj(),
			unique_message_data_get_screen(message));
		present(time);
		return UNIQUE_RESPONSE_OK;
	case UNIQUE_OPEN:
		{
			uris_holder uris(
				unique_message_data_get_uris(message));
			if(!uris || !uris[0])
				return UNIQUE_RESPONSE_FAIL;
			if(uris[1]) // multiple files?
			{
				TaskOpenMultiple* task =
					new TaskOpenMultiple(m_file_commands);
				for(const gchar* const* p = uris; *p; ++p)
					task->add_file(*p);
				m_file_commands.set_task(task);
			}
			else
			{
				TaskOpen* task = new TaskOpen(
					m_file_commands,
					Gio::File::create_for_uri(*uris));
				m_file_commands.set_task(task);
			}
			return UNIQUE_RESPONSE_OK;
		}
	case UNIQUE_GOBBY_CONNECT:
		{
			uris_holder uris(unique_message_data_get_uris(message));
			if(!uris || !uris[0])
				return UNIQUE_RESPONSE_FAIL;
			for(const gchar* const* p = uris; *p; ++p)
			{
				const gchar protocol[] = "infinote://";
				if(!g_str_has_prefix(*p, protocol))
					return UNIQUE_RESPONSE_FAIL;
				connect_to_host(*p + sizeof(protocol) - 1);
			}
			return UNIQUE_RESPONSE_OK;
		}
	default:
		return UNIQUE_RESPONSE_PASSTHROUGH;
	}
}
// For example, connect_to_host might throw Glib::ThreadError
catch(const Glib::Exception& error)
{
	// TODO: Do we want to show a dialog here?
	g_warning("Failed to process IPC message: %s", error.what().c_str());
	return UNIQUE_RESPONSE_FAIL;
}
catch (...)
{
	g_assert_not_reached();
	return UNIQUE_RESPONSE_FAIL;
}
#endif // WITH_UNIQUE
