/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2014 Armin Burgmeier <armin@arbur.net>
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

// Include this first because some of the other headers include X11.h which,
// among others, #defines "None".
#include <libxml++/libxml++.h>

#include "operations/operation-export-html.hpp"

#include "util/i18n.hpp"

#include <gtkmm/textbuffer.h>
#include <gtksourceview/gtksourcebuffer.h>

#include <libinftextgtk/inf-text-gtk-buffer.h>

#include <iomanip>
#include <ctime>
#include <cstring>
#include <cmath>
#include <cerrno>

namespace
{
	char const gobby_icon[] = "data:image/png;base64,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";

	// Sort tags so that CSS declaration order corresponds to priority
	struct TagComparator
	{
		bool operator()(GtkTextTag* first, GtkTextTag* second) const
		{
			return gtk_text_tag_get_priority(first) <
			       gtk_text_tag_get_priority(second);
		}
	};

	typedef std::set<GtkTextTag*, TagComparator> priority_tag_set;

	// We don't use Glib::ustring::compose for now because
	// it's formatting support does not compile properly under
	// Windows. See https://bugzilla.gnome.org/show_bug.cgi?id=599340
	Glib::ustring uprintf(gchar const* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		gchar* str = g_strdup_vprintf(fmt, args);
		va_end(args);
		Glib::ustring result;
		try
		{
			result = str;
		}
		catch (...)
		{
			g_free(str);
			throw;
		}
		g_free(str);
		return result;
	}

	unsigned int color_to_rgb(GdkColor* color)
	{
		return ((color->red   & 0xff00) << 8)
		     |  (color->green & 0xff00)
		     | ((color->blue  & 0xff00) >> 8);
	}

	Glib::ustring get_current_tags(priority_tag_set& tags,
	                               GtkTextIter* iter)
	{
		GSList* current_tags = gtk_text_iter_get_tags(iter);
		// make sure to free current_tags in an exception-safe manner:
		Glib::SListHandle<Glib::RefPtr<Gtk::TextTag> > handle(
			current_tags, Glib::OWNERSHIP_SHALLOW);
		Glib::ustring classes;
		for(GSList* tag = current_tags;
		    tag != 0;
		    tag = tag->next)
		{
			if(!classes.empty())
				classes += ' ';
			classes += uprintf(
				"tag_%p",
				static_cast<void*>(tag->data));
			tags.insert(GTK_TEXT_TAG(tag->data));
		}

		return classes;
	}

	// write the Gtk::TextBuffer from document into content, inserting
	// <span/>s for line breaks and authorship of chunks of text, also
	// save all users and tags encountered and the total number of
	// lines dumped
	void dump_buffer(Gobby::TextSessionView& view,
		         xmlpp::Element* content,
		         std::set<InfTextUser*>& users,
		         priority_tag_set& tags,
		         unsigned int& line_counter)
	{
		using namespace Gobby;
		users.clear();
		tags.clear();
		line_counter = 1;
		xmlpp::Element* last_node = content;
		xmlpp::Element* line_no = last_node->add_child("span");
		line_no->set_attribute("class", "line_no");
		line_no->set_attribute("id", "line_1");

		GtkTextBuffer* buffer = GTK_TEXT_BUFFER(
			view.get_text_buffer());
		InfTextGtkBuffer* inf_buffer
			= INF_TEXT_GTK_BUFFER(
				inf_session_get_buffer(
					INF_SESSION(view.get_session())));

		GtkTextIter begin;
		gtk_text_buffer_get_start_iter(buffer, &begin);
		{
			GtkTextIter end;
			gtk_text_buffer_get_end_iter(buffer, &end);
			gtk_source_buffer_ensure_highlight(
				GTK_SOURCE_BUFFER(buffer),
				&begin,
				&end);
		}

		// iterate through chunks of text during which the currently
		// set tags do not change, write each as a <span/>
		while(!gtk_text_iter_is_end(&begin))
		{
			// add current tags as classes for CSS formatting
			// (both for author of text and syntax highlighting)
			Glib::ustring classes = get_current_tags(tags, &begin);
			if(!classes.empty())
			{
				last_node = last_node->add_child("span");
				last_node->set_attribute("class", classes);

				// add mouseover "written by" popup
				// this only needs to happen when there are tags,
				// because the presence of an author implies a tag
				InfTextUser* user
					= inf_text_gtk_buffer_get_author(
						inf_buffer,
						&begin);
				if(user)
				{
					char const* user_name =
						inf_user_get_name(
							INF_USER(user));
					last_node->set_attribute(
						"title",
						uprintf(_("written by: %s"),
							user_name));
					users.insert(user);
				}
			}

			GtkTextIter next = begin;
			gtk_text_iter_forward_to_tag_toggle(&next, 0);

			// split text by newlines so we can
			// insert line number elements
			gchar* text = gtk_text_iter_get_text(&begin, &next);
			try
			{
				gchar const* last_pos = text;
				for(gchar const* i = last_pos; *i; ++i)
				{
					if(*i != '\n')
						continue;

					++line_counter;

					gchar const* next_pos = i;
					++next_pos;
					last_node->add_child_text(
						Glib::ustring(last_pos,
						              next_pos));
					last_pos = next_pos;

					line_no = last_node->add_child("span");
					line_no->set_attribute("class",
					                       "line_no");
					line_no->set_attribute(
						"id",
						uprintf("line_%d",
						        line_counter));
				}

				last_node->add_child_text(
					Glib::ustring(last_pos));
			}
			catch(...)
			{
				g_free(text);
				throw;
			}
			g_free(text);

			// if we do not have any tags, we did not add classes
			// and consequently did not go into a new span
			if(!classes.empty())
				last_node = last_node->get_parent();

			begin = next;
		}
	}

	// some random interesting information/advertisement to be put at
	// the end of the html output
	void dump_info(xmlpp::Element* node, Gobby::TextSessionView& view)
	{
		using namespace Gobby;
		// put current time
		char const* time_str;
		int const n = 128;
		char buf[n];
		{
			std::time_t now;
			std::time(&now);
			// TODO: localtime is not threadsafe
			if(std::strftime(buf, n, "%c", localtime(&now)))
				time_str = buf;
			else
				time_str = _("<unable to print date>");
		}

		char const* hostname = view.get_hostname().c_str();
		char const* path     = view.get_path().c_str();

		char const* translated =
		// %1$s is session name/hostname
		// %2$s is path within the session
		// %3$s is current date as formatted by %c,
		// %4$s is a link to the gobby site, it must be present because
		//   we need to handle that manually to insert a hyperlink
		//   instead of just printf'ing it.
			_("Document generated from %1$s:%2$s at %3$s by %4$s");
		char const* p = std::strstr(translated, "%4$s");
		g_assert(p);
		node->add_child_text(
			uprintf(Glib::ustring(translated, p).c_str(),
			        hostname, path, time_str));

		xmlpp::Element* link = node->add_child("a");
		link->set_attribute("href", "http://gobby.0x539.de/");
		link->add_child_text(PACKAGE_STRING);

		if(*p != '\0')
			node->add_child_text(
			  uprintf(p+4 , hostname, path, time_str));
	}

	// list each author before the actual text
	void dump_user_list(xmlpp::Element* list,
		            const std::set<InfTextUser*>& users)
	{
		for(std::set<InfTextUser*>::const_iterator i = users.begin();
		    i != users.end();
		    ++i)
		{
			gdouble hue = inf_text_user_get_hue(*i);
			hue = std::fmod(hue, 1);

			Gdk::Color c;
			c.set_hsv(360.0 * hue, 0.35, 1.0);
			gchar const* name = inf_user_get_name(INF_USER(*i));
			const unsigned int rgb = color_to_rgb(c.gobj());

			xmlpp::Element* item = list->add_child("li");
			item->add_child_text(name);
			item->set_attribute(
				"style",
				uprintf("background-color: #%06x;\n", rgb));
		}
	}

	void dump_tags_style(xmlpp::Element* css,
		             const priority_tag_set& tags)
	{
		for(priority_tag_set::const_iterator i = tags.begin();
		    i != tags.end();
		    ++i)
		{
			GdkColor* fg, * bg;
			gint weight;
			gboolean underline;
			PangoStyle style;
			gboolean fg_set, bg_set, weight_set,
				underline_set, style_set;
			g_object_get(G_OBJECT(*i),
				"background-gdk", &bg,
				"foreground-gdk", &fg,
				"weight",         &weight,
				"underline",      &underline,
				"style",          &style,
				"background-set", &bg_set,
				"foreground-set", &fg_set,
				"weight-set",     &weight_set,
				"underline-set",  &underline_set,
				"style-set",      &style_set,
				NULL);
			const unsigned int bg_rgb = color_to_rgb(bg);
			const unsigned int fg_rgb = color_to_rgb(fg);
			gdk_color_free(fg);
			gdk_color_free(bg);
			css->add_child_text(
				uprintf(".tag_%p {\n",
				        static_cast<void*>(*i)));
			if(fg_set)
				css->add_child_text(uprintf(
					"  color:                  #%06x;\n",
					fg_rgb));
			if(bg_set)
				css->add_child_text(uprintf(
					"  background-color:       #%06x;\n",
					bg_rgb));
			if(weight_set)
				css->add_child_text(uprintf(
					"  font-weight:            %d;\n",
					weight));
			if(underline_set)
				css->add_child_text(uprintf(
					"  text-decoration:        %s;\n",
					underline ? "underline" : "none"));
			if(style_set)
				css->add_child_text(uprintf(
					"  font-style:             %s;\n",
					(style == PANGO_STYLE_ITALIC) ?
						"italic" : "none"));
			css->add_child_text("}\n");
		}
	}

	// generate xhtml representation of the document and write it to the
	// specified location in the filesystem
	std::string export_html(Gobby::TextSessionView& view)
	{
		using namespace Gobby;
		xmlpp::Document output;

		output.set_internal_subset("html",
			"-//W3C//DTD XHTML 1.1//EN",
			"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd");

		xmlpp::Element
			* root      = output.create_root_node(
					"html",
					"http://www.w3.org/1999/xhtml"),
			* head      = root->add_child("head"),
			* body      = root->add_child("body"),
			* title     = head->add_child("title"),
			* style     = head->add_child("style"),
			* h1        = body->add_child("h1"),
			* h2        = body->add_child("h2"),
			* user_list = body->add_child("ul"),
			* content   = body->add_child("pre"),
			* info      = body->add_child("p"),
			* icon      = h1->add_child("img");

		icon->set_attribute("src",    gobby_icon);
		icon->set_attribute("width",  "48");
		icon->set_attribute("height", "48");
		icon->set_attribute("alt",    "a gobby document:");
		icon->set_attribute("class",  "icon");

		const Glib::ustring& document_name = view.get_title();
		title->add_child_text(document_name + " - infinote document");

		h1->add_child_text(document_name);

		content->set_attribute("class", "document");

		std::set<InfTextUser*> users;
		priority_tag_set tags;
		unsigned int line_counter;
		dump_buffer(view, content, users, tags, line_counter);

		h2->add_child_text(_("Participants"));

		info->set_attribute("class", "info");
		dump_info(info, view);

		style->set_attribute("type", "text/css");
		dump_user_list(user_list, users);
		dump_tags_style(style, tags);
		if(!user_list->cobj()->children)
		{
			body->remove_child(h2);
			body->remove_child(user_list);
		}

		style->add_child_text(
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
			uprintf(
				".line_no {\n"
				"  position:               absolute;\n"
				"  float:                  left;\n"
				"  clear:                  left;\n"
				"  margin-left:            -%1$uem;\n"
				"  color:                  gray;\n"
				"}\n"
				".document {\n"
				"  padding-left:            %1$uem\n"
				"}\n",
				static_cast<unsigned int>(
					std::log(line_counter)
					/ std::log(10))+1));

		return output.write_to_string("utf-8");
	}
} // anonymous namespace

Gobby::OperationExportHtml::OperationExportHtml(Operations& operations,
                                                TextSessionView& view,
                                                const std::string& uri):
	Operation(operations), m_title(view.get_title()), m_uri(uri),
	m_xml(export_html(view)), m_index(0)
{
}

Gobby::OperationExportHtml::~OperationExportHtml()
{
	if(m_file)
	{
		// TODO: Cancel outstanding async operations?
		get_status_bar().remove_message(m_message_handle);

		// Reset file explicitely before closing stream so that, on
		// failure, existing files are not overriden with the
		// temporary files we actually wrote to, at least for local
		// files.
		m_file.reset();
	}
}

void Gobby::OperationExportHtml::start()
{
	m_file = Gio::File::create_for_uri(m_uri);
	m_file->replace_async(
		sigc::mem_fun(*this, &OperationExportHtml::on_file_replace));

	m_message_handle = get_status_bar().add_info_message(
		Glib::ustring::compose(
			_("Exporting document \"%1\" to \"%2\" in HTML..."),
			m_title, m_uri));
}

void Gobby::OperationExportHtml::on_file_replace(
	const Glib::RefPtr<Gio::AsyncResult>& result)
{
	try
	{
		m_stream = m_file->replace_finish(result);

		m_stream->write_async(
			m_xml.c_str(),
			m_xml.length(),
			sigc::mem_fun(
				*this,
				&OperationExportHtml::on_stream_write));
	}
	catch(const Glib::Exception& ex)
	{
		error(ex.what());
	}
}

void Gobby::OperationExportHtml::on_stream_write(
	const Glib::RefPtr<Gio::AsyncResult>& result)
{
	try
	{
		gssize size = m_stream->write_finish(result);
		// On size < 0 an exception should have been thrown.
		g_assert(size >= 0);

		m_index += size;
		if(m_index < m_xml.length())
		{
			// Write next chunk
			m_stream->write_async(
				m_xml.c_str() + m_index,
				m_xml.length() - m_index,
				sigc::mem_fun(
					*this,
					&OperationExportHtml::
						on_stream_write));
		}
		else
		{
			m_stream->close();
			finish();
		}
	}
	catch(const Glib::Exception& ex)
	{
		error(ex.what());
	}
}

void Gobby::OperationExportHtml::error(const Glib::ustring& message)
{
	get_status_bar().add_error_message(
		Glib::ustring::compose(
			_("Failed to export document \"%1\" to HTML"), m_file->get_uri()),
		message);

	fail();
}
