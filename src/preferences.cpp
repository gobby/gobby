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

#include "preferences.hpp"

Gobby::Preferences::Editor::Editor()
{
}

Gobby::Preferences::Editor::Editor(Config::Entry& entry):
	tab_width(entry["tab"]["width"].get<unsigned int>(8)),
	tab_spaces(entry["tab"]["spaces"].get<bool>(false)),
	indentation_auto(entry["indentation"]["auto"].get<bool>(true)),
	homeend_smart(entry["homeend"]["smart"].get<bool>(true) )
{
}

void Gobby::Preferences::Editor::serialise(Config::Entry& entry) const
{
	entry["tab"]["width"].set(tab_width);
	entry["tab"]["spaces"].set(tab_spaces);
	entry["indentation"]["auto"].set(indentation_auto);
	entry["homeend"]["smart"].set(homeend_smart);
}

Gobby::Preferences::View::View()
{
}

Gobby::Preferences::View::View(Config::Entry& entry):
	wrap_text(entry["wrap"]["text"].get<bool>(true) ),
	wrap_words(entry["wrap"]["words"].get<bool>(true) ),
	linenum_display(entry["linenum"]["display"].get<bool>(true) ),
	curline_highlight(entry["curline"]["highlight"].get<bool>(true) ),
	margin_display(entry["margin"]["display"].get<bool>(true) ),
	margin_pos(entry["margin"]["pos"].get<unsigned int>(80) ),
	bracket_highlight(entry["bracket"]["highlight"].get<bool>(true) )
{
}

void Gobby::Preferences::View::serialise(Config::Entry& entry) const
{
	entry["wrap"]["text"].set(wrap_text);
	entry["wrap"]["words"].set(wrap_words);
	entry["linenum"]["display"].set(linenum_display);
	entry["curline"]["highlight"].set(curline_highlight);
	entry["margin"]["display"].set(margin_display);
	entry["margin"]["pos"].set(margin_pos);
	entry["bracket"]["highlight"].set(bracket_highlight);
}

Gobby::Preferences::Appearance::Appearance()
{
}

Gobby::Preferences::Appearance::Appearance(Config::Entry& entry):
	toolbar_show(
		static_cast<Gtk::ToolbarStyle>(
			entry["toolbar"]["show"].get<int>(
				static_cast<int>(Gtk::TOOLBAR_BOTH)
			)
		)
	),
	remember(entry["windows"]["remember"].get<bool>(true) )
{
}

void Gobby::Preferences::Appearance::serialise(Config::Entry& entry) const
{
	entry["toolbar"]["show"].set(static_cast<int>(toolbar_show) );
	entry["windows"]["remember"].set(remember);
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

const Glib::ustring& Gobby::Preferences::FileList::iterator::pattern() const
{
	return m_iter->first;
}

const Gobby::Preferences::Language&
Gobby::Preferences::FileList::iterator::language() const
{
	return m_iter->second;
}

Gobby::Preferences::FileList::FileList()
{
}

Gobby::Preferences::FileList::FileList(Config::Entry& entry,
                                       const LangManager& lang_mgr)
{
	if(entry.begin() != entry.end() )
	{
		for(Config::Entry::iterator iter = entry.begin();
		    iter != entry.end();
		    ++ iter)
		{
			Config::Entry& ent = iter.entry();

			Glib::ustring pattern =
				ent["pattern"].get<Glib::ustring>("unknown");
			Glib::ustring mime =
				ent["mime_type"].get<Glib::ustring>("unknown");

			Glib::RefPtr<Gtk::SourceLanguage> lang =
				lang_mgr->get_language_from_mime_type(mime);

			if(lang) m_files[pattern] = lang;
		}
	}
	else
	{
		// Default list
		add_by_mime_type("*.ada", "text/x-ada", lang_mgr);
		add_by_mime_type("*.ada", "text/x-ada", lang_mgr);
		add_by_mime_type("*.c", "text/x-c", lang_mgr);
		add_by_mime_type("*.h", "text/x-c++", lang_mgr);
		add_by_mime_type("*.hh", "text/x-c++", lang_mgr);
		add_by_mime_type("*.cpp", "text/x-c++", lang_mgr);
		add_by_mime_type("*.hpp", "text/x-c++", lang_mgr);
		add_by_mime_type("*.cc", "text/x-c++", lang_mgr);
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
		add_by_mime_type("*.js", "text/x-javascript", lang_mgr);
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
		add_by_mime_type("Makefile", "text/x-Maxefile", lang_mgr);
	}
}

void Gobby::Preferences::FileList::serialise(Config::Entry& entry) const
{
	int num = 0;

	for(map_type::const_iterator iter = m_files.begin();
	    iter != m_files.end();
	    ++ iter)
	{
		std::stringstream stream;
		stream << "file" << (++num);

		std::list<Glib::ustring> mime_types =
			iter->second->get_mime_types();

		Config::Entry& main = entry[stream.str()];

		main["pattern"].set(iter->first);
		main["mime_type"].set(mime_types.front());
	}
}

Gobby::Preferences::FileList::iterator
Gobby::Preferences::FileList::add(const Glib::ustring& pattern,
                                  const Language& lang)
{
	//map_type::iterator iter = m_files.find(pattern);
	//if(iter != m_files.end() ) return iter;
	return iterator(m_files.insert(std::make_pair(pattern, lang) ).first);
}

Gobby::Preferences::FileList::iterator
Gobby::Preferences::FileList::add_by_mime_type(const Glib::ustring& pattern,
                                               const Glib::ustring& mime_type,
                                               const LangManager& lang_mgr)
{
	Glib::RefPtr<Gtk::SourceLanguage> lang =
		lang_mgr->get_language_from_mime_type(mime_type);

	if(lang)
		return add(pattern, lang);
	else
		return iterator(m_files.end());
}

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

Gobby::Preferences::Preferences(Config& config, const LangManager& lang_mgr):
	editor(config["editor"]),
	view(config["view"]),
	appearance(config["appearance"]),
	files(config["files"], lang_mgr)
{
}

void Gobby::Preferences::serialise(Config& config) const
{
	// Serialise into config
	editor.serialise(config["editor"]);
	view.serialise(config["view"]);
	appearance.serialise(config["appearance"]);
	files.serialise(config["files"]);
}
