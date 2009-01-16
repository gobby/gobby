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

#include "commands/file-commands.hpp"

#include "commands/file-tasks/task-new.hpp"
#include "commands/file-tasks/task-open-file.hpp"
#include "commands/file-tasks/task-open-location.hpp"
#include "commands/file-tasks/task-save.hpp"
#include "commands/file-tasks/task-save-all.hpp"

Gobby::FileCommands::Task::Task(FileCommands& file_commands):
	m_file_commands(file_commands)
{
}

Gobby::FileCommands::Task::~Task()
{
}

void Gobby::FileCommands::Task::finish()
{
	// Note this could delete this:
	m_signal_finished.emit();
}

Gtk::Window& Gobby::FileCommands::Task::get_parent()
{
	return m_file_commands.m_parent;
}

Gobby::Folder& Gobby::FileCommands::Task::get_folder()
{
	return m_file_commands.m_folder;
}

Gobby::StatusBar& Gobby::FileCommands::Task::get_status_bar()
{
	return m_file_commands.m_status_bar;
}

Gobby::FileChooser& Gobby::FileCommands::Task::get_file_chooser()
{
	return m_file_commands.m_file_chooser;
}

Gobby::Operations& Gobby::FileCommands::Task::get_operations()
{
	return m_file_commands.m_operations;
}

const Gobby::DocumentInfoStorage&
Gobby::FileCommands::Task::get_document_info_storage()
{
	return m_file_commands.m_document_info_storage;
}

Gobby::Preferences& Gobby::FileCommands::Task::get_preferences()
{
	return m_file_commands.m_preferences;
}

Gobby::DocumentLocationDialog&
Gobby::FileCommands::Task::get_document_location_dialog()
{
	if(m_file_commands.m_location_dialog.get() == NULL)
	{
		m_file_commands.m_location_dialog.reset(
			new DocumentLocationDialog(
				m_file_commands.m_parent,
				INF_GTK_BROWSER_MODEL(
					m_file_commands.m_browser.
						get_store())));
	}

	return *m_file_commands.m_location_dialog;
}

Gobby::FileCommands::FileCommands(Gtk::Window& parent, Header& header,
                                  Browser& browser, Folder& folder,
				  StatusBar& status_bar,
                                  FileChooser& file_chooser,
                                  Operations& operations,
				  const DocumentInfoStorage& info_storage,
                                  Preferences& preferences):
	m_parent(parent), m_header(header), m_browser(browser),
	m_folder(folder), m_status_bar(status_bar),
	m_file_chooser(file_chooser), m_operations(operations),
	m_document_info_storage(info_storage), m_preferences(preferences)
{
	header.action_file_new->signal_activate().connect(
		sigc::mem_fun(*this, &FileCommands::on_new));
	header.action_file_open->signal_activate().connect(
		sigc::mem_fun(*this, &FileCommands::on_open));
	header.action_file_open_location->signal_activate().connect(
		sigc::mem_fun(*this, &FileCommands::on_open_location));
	header.action_file_save->signal_activate().connect(
		sigc::mem_fun(*this, &FileCommands::on_save));
	header.action_file_save_as->signal_activate().connect(
		sigc::mem_fun(*this, &FileCommands::on_save_as));
	header.action_file_save_all->signal_activate().connect(
		sigc::mem_fun(*this, &FileCommands::on_save_all));
	header.action_file_export_html->signal_activate().connect(
		sigc::mem_fun(*this, &FileCommands::on_export_html));
	header.action_file_close->signal_activate().connect(
		sigc::mem_fun(*this, &FileCommands::on_close));
	header.action_file_quit->signal_activate().connect(
		sigc::mem_fun(*this, &FileCommands::on_quit));

	folder.signal_document_changed().connect(
		sigc::mem_fun(*this, &FileCommands::on_document_changed));

	InfGtkBrowserStore* store = browser.get_store();
	m_row_inserted_handler =
		g_signal_connect(G_OBJECT(store), "row-inserted",
		                 G_CALLBACK(on_row_inserted_static), this);
	m_row_deleted_handler =
		g_signal_connect(G_OBJECT(store), "row-deleted",
		                 G_CALLBACK(on_row_deleted_static), this);

	update_sensitivity();	
}

Gobby::FileCommands::~FileCommands()
{
	InfGtkBrowserStore* store = m_browser.get_store();
	g_signal_handler_disconnect(G_OBJECT(store), m_row_inserted_handler);
	g_signal_handler_disconnect(G_OBJECT(store), m_row_deleted_handler);
}

void Gobby::FileCommands::set_task(Task* task)
{
	task->signal_finished().connect(sigc::mem_fun(
		*this, &FileCommands::on_task_finished));
	m_task.reset(task);
  task->run();
}

void Gobby::FileCommands::on_document_changed(DocWindow* document)
{
	update_sensitivity();
}

void Gobby::FileCommands::on_row_inserted()
{
	update_sensitivity();
}

void Gobby::FileCommands::on_row_deleted()
{
	update_sensitivity();
}

void Gobby::FileCommands::on_task_finished()
{
	m_task.reset(NULL);
}

void Gobby::FileCommands::on_new()
{
	set_task(new TaskNew(*this));
}

void Gobby::FileCommands::on_open()
{
	set_task(new TaskOpenFile(*this));
}

void Gobby::FileCommands::on_open_location()
{
	set_task(new TaskOpenLocation(*this));
}

void Gobby::FileCommands::on_save()
{
	// TODO: Encoding selection in file chooser
	DocWindow* document = m_folder.get_current_document();
	g_assert(document != NULL);

	const DocumentInfoStorage::Info* info =
		m_document_info_storage.get_info(
			document->get_info_storage_key());

	if(info != NULL && !info->uri.empty())
	{
		m_operations.save_document(
			*document, m_folder, info->uri, info->encoding,
			info->eol_style);
	}
	else
	{
		on_save_as();
	}
}

void Gobby::FileCommands::on_save_as()
{
	DocWindow* document = m_folder.get_current_document();
	g_assert(document != NULL);

	set_task(new TaskSave(*this, *document));
}

void Gobby::FileCommands::on_save_all()
{
	set_task(new TaskSaveAll(*this));
}

#include <iomanip>

#include <ctime>
#include <cstring>
#include <cmath>

#include <gtkmm/textbuffer.h>
#include <libxml++/libxml++.h>

#include <libinftextgtk/inf-text-gtk-buffer.h>

// write the Gtk::TextBuffer from document into content, inserting <span/>s for
// line breaks and authorship of chunks of text, also save all users
// encountered and the total number of lines dumped
void dump_buffer(DocWindow& document,
                 xmlpp::Element* content,
                 std::set<InfTextUser*>& users,
                 unsigned int& line_counter) {
  users.clear();
  line_counter = 0;
	xmlpp::Element* last_node = content;
	last_node->add_child("span")->set_attribute("class", "line_no");

	GtkTextBuffer* buffer = GTK_TEXT_BUFFER(document.get_text_buffer());
	InfTextGtkBuffer* inf_buffer
		= INF_TEXT_GTK_BUFFER(
				inf_session_get_buffer(INF_SESSION(document.get_session())));

	InfTextUser* user = 0;

	GtkTextIter begin;
	gtk_text_buffer_get_start_iter(buffer, &begin);
	
	if (gtk_text_iter_is_end(&begin))
		return;

	for (;;) {
		GtkTextIter next = begin;
		gtk_text_iter_forward_to_tag_toggle(&next, 0);

		// split text by newlines so we can insert line number elements
		gchar* text = gtk_text_iter_get_text(&begin, &next);
		try {
			gchar const* last_pos = text;
			for (gchar const* i = text; *i; ++i) {
				if (*i != '\n')
					continue;

				++line_counter;

				gchar const* next_pos = i;
				++next_pos;
				last_node->add_child_text(Glib::ustring(last_pos, next_pos));
				last_pos = next_pos;

				// drop author <span/> for a moment for the line number <span/>
				if (user)
					last_node = last_node->get_parent();
			 	last_node
			 	  ->add_child("span")
			 	  ->set_attribute("class", "line_no");
				if (user) {
					last_node = last_node->add_child("span");
					last_node->set_attribute(
						"class",
						Glib::ustring::compose("user_%1",
						                       inf_user_get_id(INF_USER(user))));
					last_node->set_attribute(
					  "title",
					  Glib::ustring::compose(_("written by: %1"),
					                         inf_user_get_name(INF_USER(user))));
				}
			}
			last_node->add_child_text(Glib::ustring(last_pos));
		} catch (...) {
			g_free(text);
			throw;
		}
		g_free(text);

		begin = next;

		if (gtk_text_iter_is_end(&begin))
			break;

		// check author, insert new <span/> if necessary
		InfTextUser* new_user
			= inf_text_gtk_buffer_get_author(inf_buffer, &next);

		if (new_user) {
			guint new_id = inf_user_get_id(INF_USER(new_user));
			if (!user || user != new_user) {
				if (user)
					last_node = last_node->get_parent();

				last_node = last_node->add_child("span");
				last_node->set_attribute(
					"class",
					Glib::ustring::compose("user_%1", new_id));
				last_node->set_attribute(
				  "title",
				  Glib::ustring::compose(_("written by: %1"),
				                         inf_user_get_name(INF_USER(new_user))));
			}

			user = new_user;
			users.insert(user);
		} else {
			if (user)
				last_node = last_node->get_parent();
			user = 0;
		}
	}
}

// some random interesting information/advertisement to be put at the end of
// the html output
void dump_info(xmlpp::Element* node, DocWindow& document) {
	// put current time
  char const* time_str;
	int const n = 128;
	char buf[n];
  {
    std::time_t now;
	  std::time(&now);
	  // TODO: localtime is not threadsafe, use boost.date_time!!
		if (std::strftime(buf, n, "%c", localtime(&now)))
			time_str = buf;
		else
			time_str = _("<unable to print date>");
	}

	// put document metadata, like path, hostname of infinoted
	// TODO: figure out what interesting info we can pull out of the session
	char session_info[] = "<placeholder for session info and path>";

	// %1 is information about the document's location, session name,
	// %2 is current date as formatted by %c,
	// %3 is a link to the gobby site
	char const* translated = _("Document generated from %1 at %2 by %3");
	char const* p = std::strstr(translated, "%3");
	g_assert(p);
	node->add_child_text(
		Glib::ustring::compose(Glib::ustring(translated, p), session_info, time_str));

	xmlpp::Element* link = node->add_child("a");
	link->set_attribute("href", "http://gobby.0x539.de/");
	link->add_child_text(PACKAGE_STRING);

	if (*p != 0)
		node->add_child_text(
		  Glib::ustring::compose(p+2 , session_info, time_str));
}

// put author colours into the css, list each author before the actual text
void add_user_colours(xmlpp::Element* css,
                      xmlpp::Element* list,
                      const std::set<InfTextUser*>& users) {
	for (std::set<InfTextUser*>::const_iterator i = users.begin();
	     i != users.end();
			 ++i) {
		guint id = inf_user_get_id(INF_USER(*i));
		gdouble hue = inf_text_user_get_hue(*i);
		hue = std::fmod(hue, 1);

		Gdk::Color c;
		c.set_hsv(360.0 * hue, 0.35, 1.0);
		gchar const* name = inf_user_get_name(INF_USER(*i));
		unsigned int rgb =
			  ((c.get_red()   & 0xff00) << 8)
			|  (c.get_green() & 0xff00)
			| ((c.get_blue()  & 0xff00) >> 8);

		css->add_child_text(Glib::ustring::compose(
			".user_%1 {\n"
			"  background-color:       #%2;\n"
			"}\n",
			id,
			Glib::ustring::format(std::hex,
			                      std::setw(6),
			                      std::setfill(L'0'), rgb)));
		xmlpp::Element* item = list->add_child("li");
		item->set_attribute("class", Glib::ustring::compose("user_%1", id));
		item->add_child_text(name);
	}
}

// generate xhtml representation of the document and write it to the
// specified location in the filesystem
void export_html(DocWindow& document, const Glib::ustring& output_path) {
	xmlpp::Document output;

  output.set_internal_subset("html",
    "-//W3C//DTD XHTML 1.1//EN",
    "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd");

	xmlpp::Element
		* root      = output.create_root_node("html", "http://www.w3.org/1999/xhtml"),
		* head      = root->add_child("head"),
		* body      = root->add_child("body"),
		* title     = head->add_child("title"),
		* style     = head->add_child("style"),
		* h1        = body->add_child("h1"),
		* h2        = body->add_child("h2"),
		* user_list = body->add_child("ul"),
		* content   = body->add_child("pre"),
		* info      = body->add_child("p");

	const Glib::ustring& document_name = document.get_title();
	title->add_child_text(document_name + " - infinote document");

	h1->add_child_text(document_name);

	content->set_attribute("class", "document");

	std::set<InfTextUser*> users;
	unsigned int line_counter;
	dump_buffer(document, content, users, line_counter);

	h2->add_child_text(_("Participants"));

	info->set_attribute("class", "info");
  dump_info(info, document);

	style->set_attribute("type", "text/css");
	add_user_colours(style, user_list, users);
	if (!user_list->cobj()->children) {
		body->remove_child(h2);
		body->remove_child(user_list);
	}

	style->add_child_text(
			"h1 {\n"
			"  font-family:\n"
			"}\n"
			".document {\n"
			"  border-top:             1px solid gray;\n"
			"  border-bottom:          1px solid black;\n"
			"  padding-bottom:         1.2em;\n"
			"  counter-reset:          line;\n"
			"}\n"
			".line_no:before {\n"
			"  content:                counter(line);\n"
			"  counter-increment:      line;\n"
			"}\n"
			".info {\n"
			"  font-size:              small;\n"
			"}\n");

	style->add_child_text(
		Glib::ustring::compose(
			".line_no {\n"
			"  position:               absolute;\n"
			"  float:                  left;\n"
			"  clear:                  left;\n"
			"  margin-left:            -%1em;\n"
			"  color:                  gray;\n"
			"}\n"
			".document {\n"
			"  padding-left:            %1em\n"
			"}\n",
	  static_cast<unsigned int>(std::log(line_counter)/std::log(10))+1));

	output.write_to_file(output_path, "utf-8");
}

void Gobby::FileCommands::on_export_html() {
	DocWindow* document = m_folder.get_current_document();
	g_assert(document != NULL);

	// TODO: filechooser, possibly remember choice
	// what is that whole operation/task thing?
	export_html(*document, "output.xhtml");
}


void Gobby::FileCommands::on_close()
{
	DocWindow* document = m_folder.get_current_document();
	g_assert(document != NULL);

	m_folder.remove_document(*document);
}

void Gobby::FileCommands::on_quit()
{
	m_parent.hide();
}

void Gobby::FileCommands::update_sensitivity()
{
	GtkTreeIter dummy_iter;
	bool create_sensitivity = gtk_tree_model_get_iter_first(
		GTK_TREE_MODEL(m_browser.get_store()), &dummy_iter);
	gboolean active_sensitivity = m_folder.get_current_document() != NULL;

	m_header.action_file_new->set_sensitive(create_sensitivity);
	m_header.action_file_open->set_sensitive(create_sensitivity);
	m_header.action_file_open_location->set_sensitive(create_sensitivity);

	m_header.action_file_save->set_sensitive(active_sensitivity);
	m_header.action_file_save_as->set_sensitive(active_sensitivity);
	m_header.action_file_save_all->set_sensitive(active_sensitivity);
	m_header.action_file_export_html->set_sensitive(active_sensitivity);
	m_header.action_file_close->set_sensitive(active_sensitivity);
}