/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2014 Armin Burgmeier <armin@arbur.net>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "features.hpp"
#include "window.hpp"

#include "commands/file-tasks/task-open.hpp"
#include "commands/file-tasks/task-open-multiple.hpp"

#include "util/i18n.hpp"

#include <gtkmm/frame.h>

Gobby::Window::Window(Config& config,
                      GtkSourceLanguageManager* language_manager,
                      FileChooser& file_chooser,
                      Preferences& preferences,
                      CertificateManager& cert_manager):
	m_config(config),
	m_lang_manager(language_manager),
	m_file_chooser(file_chooser),
	m_preferences(preferences), m_cert_manager(cert_manager),
	m_connection_manager(cert_manager, preferences),
	m_text_folder(false, m_preferences, m_lang_manager),
	m_chat_folder(true, m_preferences, m_lang_manager),
	m_statusbar(m_text_folder, m_preferences),
	m_toolbar(m_preferences),
	m_browser(*this, m_statusbar, m_connection_manager),
	m_chat_frame(_("Chat"), "chat", m_preferences.appearance.show_chat),
	m_actions(*this, m_preferences),
	m_info_storage(INF_GTK_BROWSER_MODEL(m_browser.get_store())),
	m_folder_manager(m_browser, m_info_storage,
	                 m_text_folder, m_chat_folder),
	m_operations(m_info_storage, m_browser,
	             m_folder_manager, m_statusbar),
	m_auth_commands(*this, m_browser, m_statusbar,
	                m_connection_manager, m_preferences),
	m_self_hoster(m_connection_manager.get_io(),
	              m_connection_manager.get_communication_manager(),
	              m_connection_manager.get_publisher(),
	              m_auth_commands.get_sasl_context(),
	              m_statusbar, m_cert_manager, m_preferences),
	m_browser_commands(m_browser, m_folder_manager, m_statusbar,
	                   m_operations, m_preferences),
	m_browser_context_commands(*this, m_connection_manager.get_io(),
	                           m_browser, m_file_chooser, m_operations,
	                           m_cert_manager, m_preferences),
	m_autosave_commands(m_text_folder, m_operations,
	                    m_info_storage, m_preferences),
	m_subscription_commands(m_text_folder, m_chat_folder),
	m_synchronization_commands(m_text_folder, m_chat_folder),
	m_user_join_commands(m_folder_manager, m_preferences),
	m_text_folder_commands(m_text_folder),
	m_chat_folder_commands(m_chat_folder),
	m_file_commands(*this, m_actions, m_browser, m_folder_manager,
	                m_statusbar, m_file_chooser, m_operations,
	                m_info_storage, m_preferences),
	m_edit_commands(*this, m_actions, m_text_folder, m_statusbar),
	m_view_commands(*this, m_actions, m_lang_manager, m_text_folder,
	                m_chat_frame, m_chat_folder, m_preferences),
	m_title_bar(*this, m_text_folder)
{
	m_chat_frame.signal_show().connect(
		sigc::mem_fun(*this, &Window::on_chat_show), true);
	m_chat_frame.signal_hide().connect(
		sigc::mem_fun(*this, &Window::on_chat_hide), false);

	m_browser.add_browser(INF_BROWSER(m_self_hoster.get_directory()),
	                      _("This Computer"));

	m_toolbar.show();
	m_browser.show();
	m_text_folder.show();
	m_chat_folder.show();

	// Build UI
	Glib::RefPtr<Gtk::AccelGroup> group = Gtk::AccelGroup::create();
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
		_("Document Browser"), "document-list",
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

	m_mainbox.pack_start(m_toolbar, Gtk::PACK_SHRINK);
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

void Gobby::Window::subscribe(const Glib::ustring& uri)
{
	m_operations.subscribe_path(uri);
}

void Gobby::Window::open_files(const Operations::file_list& files)
{
	if(files.size() == 1)
	{
		m_file_commands.set_task(
			new TaskOpen(m_file_commands, files[0]));
	}
	else if(files.size() > 1)
	{
		m_file_commands.set_task(
			new TaskOpenMultiple(m_file_commands, files));
	}
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

	bool handled =
		gtk_window_propagate_key_event(GTK_WINDOW(gobj()), event);
	if(!handled)
		handled = gtk_window_activate_key(GTK_WINDOW(gobj()), event);

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
		m_initial_dlg.reset(
			new InitialDialog(
				*this, m_statusbar, m_preferences,
				m_cert_manager));
		m_initial_dlg->present();
		m_initial_dlg->signal_hide().connect(
			sigc::mem_fun(*this,
			              &Window::on_initial_dialog_hide));
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
