/* gobby - A GTKmm driven libobby client
 * Copyright (C) 2005, 2006 0x539 dev group
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

#include "preferences.hpp"

namespace
{
  GtkSourceLanguage*
  get_language_from_mime_type(GtkSourceLanguageManager* manager,
                              const gchar* mime_type)
  {
#ifdef WITH_GTKSOURCEVIEW2
    const GSList* list = gtk_source_language_manager_get_available_languages(
      manager);

    for(; list != NULL; list = list->next)
    {
      gchar** mime_types = gtk_source_language_get_mime_types(
        GTK_SOURCE_LANGUAGE(list->data));

      if(mime_types != NULL)
      {
        for(gchar** type = mime_types; *type != NULL; ++ type)
        {
          if(strcmp(mime_type, *type) == 0)
          {
            g_strfreev(mime_types);
            return GTK_SOURCE_LANGUAGE(list->data);
          }
        }

        g_strfreev(mime_types);
      }
    }

    return NULL;
#else
    return gtk_source_languages_manager_get_language_from_mime_type(
      manager, mime_type);
#endif
  }
}

Gobby::Preferences::Editor::Editor()
{
}

Gobby::Preferences::Editor::Editor(Config::ParentEntry& entry):
	tab_width(entry["tab"].get_value<unsigned int>("width", 8)),
	tab_spaces(entry["tab"].get_value<bool>("spaces", false)),
	indentation_auto(entry["indentation"].get_value<bool>("auto", true)),
	homeend_smart(entry["homeend"].get_value<bool>("smart", true) )
{
}

void Gobby::Preferences::Editor::serialise(Config::ParentEntry& entry) const
{
	entry["tab"].set_value("width", tab_width);
	entry["tab"].set_value("spaces", tab_spaces);
	entry["indentation"].set_value("auto", indentation_auto);
	entry["homeend"].set_value("smart", homeend_smart);
}

Gobby::Preferences::View::View()
{
}

Gobby::Preferences::View::View(Config::ParentEntry& entry):
	wrap_text(entry["wrap"].get_value<bool>("text", true) ),
	wrap_words(entry["wrap"].get_value<bool>("words", true) ),
	linenum_display(entry["linenum"].get_value<bool>("display", true) ),
	curline_highlight(
		entry["curline"].get_value<bool>("highlight", true)
	),
	margin_display(entry["margin"].get_value<bool>("display", true) ),
	margin_pos(entry["margin"].get_value<unsigned int>("pos", 80) ),
	bracket_highlight(entry["bracket"].get_value<bool>("highlight", true) )
{
}

void Gobby::Preferences::View::serialise(Config::ParentEntry& entry) const
{
	entry["wrap"].set_value("text", wrap_text);
	entry["wrap"].set_value("words", wrap_words);
	entry["linenum"].set_value("display", linenum_display);
	entry["curline"].set_value("highlight", curline_highlight);
	entry["margin"].set_value("display", margin_display);
	entry["margin"].set_value("pos", margin_pos);
	entry["bracket"].set_value("highlight", bracket_highlight);
}

Gobby::Preferences::Appearance::Appearance()
{
}

Gobby::Preferences::Appearance::Appearance(Config::ParentEntry& entry):
	toolbar_show(
		static_cast<Gtk::ToolbarStyle>(
			entry["toolbar"].get_value<int>(
				"show",
				static_cast<int>(Gtk::TOOLBAR_BOTH)
			)
		)
	),
	remember(entry["windows"].get_value<bool>("remember", true) ),
	urgency_hint(entry["windows"].get_value<bool>("urgency_hint", true) )
{
}

void Gobby::Preferences::Appearance::
	serialise(Config::ParentEntry& entry) const
{
	entry["toolbar"].set_value("show", static_cast<int>(toolbar_show) );
	entry["windows"].set_value("remember", remember);
	entry["windows"].set_value("urgency_hint", urgency_hint);
}

Gobby::Preferences::Font::Font()
{
}

Gobby::Preferences::Font::Font(Config::ParentEntry& entry):
	desc(entry.get_value<Glib::ustring>("desc", "Monospace 10") )
{
}

void Gobby::Preferences::Font::serialise(Config::ParentEntry& entry) const
{
	entry.set_value("desc", desc.to_string());
}

Gobby::Preferences::Behaviour::Behaviour()
{
}

Gobby::Preferences::Behaviour::Behaviour(Config::ParentEntry& entry):
	auto_open_new_documents(entry.get_value<bool>(
		"auto_open_new_documents", false))
{
}

void Gobby::Preferences::Behaviour::serialise(Config::ParentEntry& entry) const
{
	entry.set_value("auto_open_new_documents", auto_open_new_documents);
}

Gobby::Preferences::FileList::iterator::iterator(const base_iterator iter):
	m_iter(iter)
{
}

Gobby::Preferences::FileList::iterator&
Gobby::Preferences::FileList::iterator::operator++()
{
	++ m_iter;
	return *this;
}

Gobby::Preferences::FileList::iterator
Gobby::Preferences::FileList::iterator::operator++(int)
{
	iterator temp(m_iter);
	++ *this;
	return temp;
}

bool Gobby::Preferences::FileList::iterator::
	operator==(const iterator& other) const
{
	return m_iter == other.m_iter;
}

bool Gobby::Preferences::FileList::iterator::
	operator!=(const iterator& other) const
{
	return m_iter != other.m_iter;
}

const Glib::ustring& Gobby::Preferences::FileList::iterator::pattern() const
{
	return m_iter->first;
}

GtkSourceLanguage*
Gobby::Preferences::FileList::iterator::language() const
{
	return m_iter->second;
}

Gobby::Preferences::FileList::FileList()
{
}

Gobby::Preferences::FileList::FileList(Config::ParentEntry& entry,
                                       GtkSourceLanguageManager* lang_mgr)
{
	if(entry.begin() != entry.end() )
	{
		for(Config::ParentEntry::iterator iter = entry.begin();
		    iter != entry.end();
		    ++ iter)
		{
			Config::Entry& ent = *iter;
			Config::ParentEntry* parent_entry =
				dynamic_cast<Config::ParentEntry*>(&ent);

			// Don't know what it is...
			if(parent_entry == NULL) continue;

			Glib::ustring pattern = parent_entry->get_value<
				Glib::ustring
			>("pattern", "unknown");

			Glib::ustring mime = parent_entry->get_value<
				Glib::ustring
			>("mime_type", "unknown");

      GtkSourceLanguage* lang = get_language_from_mime_type(
        lang_mgr, mime.c_str());

			if(lang)
      {
        m_files[pattern] = lang;
        g_object_ref(G_OBJECT(lang));
      }
		}
	}
	else
	{
#ifdef WITH_GTKSOURCEVIEW2
    const GSList* list = gtk_source_language_manager_get_available_languages(
      lang_mgr);

    for(; list != NULL; list = list->next)
    {
      GtkSourceLanguage* language = GTK_SOURCE_LANGUAGE(list->data);
      gchar** globs = gtk_source_language_get_globs(language);
      if(globs != NULL)
      {
        for(gchar** glob = globs; *glob != NULL; ++ glob)
        {
          add(*glob, language);
        }

        g_strfreev(globs);
      }
    }
#else
		// Default list
		add_by_mime_type("*.ada", "text/x-ada", lang_mgr);
		add_by_mime_type("*.c", "text/x-csrc", lang_mgr);
		add_by_mime_type("*.h", "text/x-chdr", lang_mgr);
		add_by_mime_type("*.cpp", "text/x-c++src", lang_mgr);
		add_by_mime_type("*.hpp", "text/x-c++hdr", lang_mgr);
		add_by_mime_type("*.cc", "text/x-c++src", lang_mgr);
		add_by_mime_type("*.hh", "text/x-c++hdr", lang_mgr);
		add_by_mime_type("*.cs", "text/x-csharp", lang_mgr);
		add_by_mime_type("*.css", "text/css", lang_mgr);
		add_by_mime_type("*.diff", "text/x-diff", lang_mgr);
		add_by_mime_type("*.patch", "text/x-diff", lang_mgr);
		add_by_mime_type("*.f", "text/x-fortran", lang_mgr);
		add_by_mime_type("*.f77", "text/x-fortran", lang_mgr);
		add_by_mime_type("*.hs", "text/x-haskell", lang_mgr);
		add_by_mime_type("*.htm", "text/html", lang_mgr);
		add_by_mime_type("*.html", "text/html", lang_mgr);
		add_by_mime_type("*.xhtml", "text/html", lang_mgr);
		add_by_mime_type("*.idl", "text/x-idl", lang_mgr);
		add_by_mime_type("*.java", "text/x-java", lang_mgr);
		add_by_mime_type("*.js", "application/x-javascript", lang_mgr);
		add_by_mime_type("*.tex", "text/x-tex", lang_mgr);
		add_by_mime_type("*.latex", "text/x-tex", lang_mgr);
		add_by_mime_type("*.lua", "text/x-lua", lang_mgr);
		add_by_mime_type("*.dpr", "text/x-pascal", lang_mgr);
		add_by_mime_type("*.pas", "text/x-pascal", lang_mgr);
		add_by_mime_type("*.pl", "text/x-perl", lang_mgr);
		add_by_mime_type("*.pm", "text/x-perl", lang_mgr);
		add_by_mime_type("*.php", "text/x-php", lang_mgr);
		add_by_mime_type("*.php3", "text/x-php", lang_mgr);
		add_by_mime_type("*.php4", "text/x-php", lang_mgr);
		add_by_mime_type("*.php5", "text/x-php", lang_mgr);
		add_by_mime_type(
			"*.po",
			"text/x-gettext-translation",
			lang_mgr
		);
		add_by_mime_type("*.py", "text/x-python", lang_mgr);
		add_by_mime_type("*.rb", "text/x-ruby", lang_mgr);
		add_by_mime_type("*.sql", "text/x-sql", lang_mgr);
		add_by_mime_type("*.texi", "text/x-texinfo", lang_mgr);
		add_by_mime_type("*.bas", "text/x-vb", lang_mgr);
		add_by_mime_type("*.vbs", "text/x-vb", lang_mgr);
		add_by_mime_type("*.v", "text/x-verilog-src", lang_mgr);
		add_by_mime_type("*.xml", "text/xml", lang_mgr);
		add_by_mime_type(
			"*.desktop",
			"application/x-gnome-app-info",
			lang_mgr
		);
		add_by_mime_type("*.tcl", "text/x-tcl", lang_mgr);
		add_by_mime_type("Makefile", "text/x-makefile", lang_mgr);
#endif
	}
}

Gobby::Preferences::FileList::FileList(const FileList& src):
  m_files(src.m_files)
{
  // TODO: It would also be great if we would not need to ref all the
  // languages.
  for(map_type::iterator iter = m_files.begin();
      iter != m_files.end();
      ++ iter)
  {
    g_object_ref(G_OBJECT(iter->second));
  }
}

Gobby::Preferences::FileList::~FileList()
{
  for(map_type::iterator iter = m_files.begin();
      iter != m_files.end();
      ++ iter)
  {
    g_object_unref(G_OBJECT(iter->second));
  }
}

void Gobby::Preferences::FileList::serialise(Config::ParentEntry& entry) const
{
	int num = 0;

	for(map_type::const_iterator iter = m_files.begin();
	    iter != m_files.end();
	    ++ iter)
	{
		std::stringstream stream;
		stream << "file" << (++num);

    gchar* mime_type = NULL;
#ifdef WITH_GTKSOURCEVIEW2
    gchar** mime_types = gtk_source_language_get_mime_types(iter->second);
    if(mime_types != NULL && *mime_types != NULL)
      mime_type = g_strdup(*mime_types);
    g_strfreev(mime_types);
#else
    GSList* mime_types = gtk_source_language_get_mime_types(iter->second);
    for(GSList* cur = mime_types; cur != NULL; cur = cur->next)
    {
      if(!mime_type)
        mime_type = static_cast<gchar*>(cur->data);
      else
        g_free(cur->data);
    }
    g_slist_free(mime_types);
#endif

		Config::ParentEntry& main = entry.set_parent(stream.str());

		main.set_value("pattern", iter->first);
    if(mime_type != NULL)
      main.set_value("mime_type", mime_type);

    g_free(mime_type);
	}
}

Gobby::Preferences::FileList::iterator
Gobby::Preferences::FileList::add(const Glib::ustring& pattern,
                                  GtkSourceLanguage* lang)
{
	//map_type::iterator iter = m_files.find(pattern);
	//if(iter != m_files.end() ) return iter;
  g_object_ref(G_OBJECT(lang));
	return iterator(m_files.insert(std::make_pair(pattern, lang) ).first);
}

#ifndef WITH_GTKSOURCEVIEW2
Gobby::Preferences::FileList::iterator
Gobby::Preferences::FileList::add_by_mime_type(const Glib::ustring& pattern,
                                               const Glib::ustring& mime_type,
                                               GtkSourceLanguageManager* lang_mgr)
{
  GtkSourceLanguage* lang =
    gtk_source_languages_manager_get_language_from_mime_type(
      lang_mgr, mime_type.c_str());

	if(lang != NULL)
		return add(pattern, lang);
	else
		return iterator(m_files.end());
}
#endif

Gobby::Preferences::FileList::iterator
Gobby::Preferences::FileList::begin() const
{
	return iterator(m_files.begin() );
}

Gobby::Preferences::FileList::iterator
Gobby::Preferences::FileList::end() const
{
	return iterator(m_files.end() );
}

Gobby::Preferences::Preferences()
{
	// Uninitialised preferences
}

Gobby::Preferences::Preferences(Config& config, GtkSourceLanguageManager* mgr):
	editor(config.get_root()["editor"]),
	view(config.get_root()["view"]),
	appearance(config.get_root()["appearance"]),
	font(config.get_root()["font"]),
	behaviour(config.get_root()["behaviour"]),
	files(config.get_root()["files"], mgr)
{
}

void Gobby::Preferences::serialise(Config& config) const
{
	// Serialise into config
	editor.serialise(config.get_root()["editor"]);
	view.serialise(config.get_root()["view"]);
	appearance.serialise(config.get_root()["appearance"]);
	font.serialise(config.get_root()["font"]);
	behaviour.serialise(config.get_root()["behaviour"]);
	files.serialise(config.get_root()["files"]);
}
