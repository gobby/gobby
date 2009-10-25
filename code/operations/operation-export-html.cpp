/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008, 2009 Armin Burgmeier <armin@arbur.net>
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

#include "operations/operation-export-html.hpp"

#include "util/i18n.hpp"

#include <gtkmm/textbuffer.h>
#include <libxml++/libxml++.h>

#include <libinftextgtk/inf-text-gtk-buffer.h>

#include <iomanip>
#include <ctime>
#include <cstring>
#include <cmath>
#include <cerrno>

namespace
{
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
	void dump_buffer(Gobby::DocWindow& document,
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
			document.get_text_buffer());
		InfTextGtkBuffer* inf_buffer
			= INF_TEXT_GTK_BUFFER(
				inf_session_get_buffer(
					INF_SESSION(document.get_session())));

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
			last_node = last_node->add_child("span");

			// add current tags as classes for CSS formatting
			// (both for author of text and syntax highlighting)
			Glib::ustring classes = get_current_tags(tags, &begin);
			if(!classes.empty())
				last_node->set_attribute("class", classes);

			// add mouseover "written by" popup
			InfTextUser* new_user
				= inf_text_gtk_buffer_get_author(
					inf_buffer,
					&begin);
			if(new_user)
			{
				last_node->set_attribute(
					"title",
					uprintf(_("written by: %s"),
						inf_user_get_name(
							INF_USER(new_user))));
				users.insert(new_user);
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
			last_node = last_node->get_parent();

			begin = next;
		}
	}

	// some random interesting information/advertisement to be put at
	// the end of the html output
	void dump_info(xmlpp::Element* node, Gobby::DocWindow& document)
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

		char const* hostname = document.get_hostname().c_str();
		char const* path     = document.get_path().c_str();

		// %1$s is session name/hostname
		// %2$s is path within the session
		// %3$s is current date as formatted by %c,
		// %4$s is a link to the gobby site, it must be present because
		//   we need to handle that manually to insert a hyperlink
		//   instead of just printf'ing it.
		char const* translated =
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
	std::string export_html(Gobby::DocWindow& document)
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
			* info      = body->add_child("p");

		const Glib::ustring& document_name = document.get_title();
		title->add_child_text(document_name + " - infinote document");

		h1->add_child_text(document_name);

		content->set_attribute("class", "document");

		std::set<InfTextUser*> users;
		priority_tag_set tags;
		unsigned int line_counter;
		dump_buffer(document, content, users, tags, line_counter);

		h2->add_child_text(_("Participants"));

		info->set_attribute("class", "info");
		dump_info(info, document);

		style->set_attribute("type", "text/css");
		dump_user_list(user_list, users);
		dump_tags_style(style, tags);
		if(!user_list->cobj()->children)
		{
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
                                                DocWindow& document,
                                                const std::string& uri):
	Operation(operations), m_index(0),
	m_xml(export_html(document))
{
	m_file = Gio::File::create_for_uri(uri);
	m_file->replace_async(
		sigc::mem_fun(*this, &OperationExportHtml::on_file_replace));

	m_message_handle = get_status_bar().add_message(
		StatusBar::INFO,
		Glib::ustring::compose(
			_("Exporting document %1 to %2 in HTML..."),
			document.get_title(), uri), 0);
}

Gobby::OperationExportHtml::~OperationExportHtml()
{
	// TODO: Cancel outstanding async operations?
	get_status_bar().remove_message(m_message_handle);

	// Reset file explicitely before closing stream so that, on failure,
	// existing files are not overriden with the temporary files we
	// actually wrote to, at least for local files.
	m_file.reset();
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
	get_status_bar().add_message(
		StatusBar::ERROR,
		Glib::ustring::compose(
			_("Failed to export document %1 to HTML: %2"),
			m_file->get_uri(), message), 5);

	fail();
}