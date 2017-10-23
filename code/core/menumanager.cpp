/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2015 Armin Burgmeier <armin@arbur.net>
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

#include "menumanager.hpp"

#include <gtkmm/builder.h>

#include <gtksourceview/gtksource.h>

namespace
{
	bool language_sort_func(GtkSourceLanguage* lang1,
	                        GtkSourceLanguage* lang2)
	{
		// TODO: Speedup by using collation keys?
		// We should profile first.
		gchar* casefold1 = g_utf8_casefold(
			gtk_source_language_get_name(lang1), -1);
		gchar* casefold2 = g_utf8_casefold(
			gtk_source_language_get_name(lang2), -1);

		int ret = g_utf8_collate(casefold1, casefold2);

		g_free(casefold1);
		g_free(casefold2);

		return ret < 0;
	}
} // anonymous namespace

Gobby::MenuManager::MenuManager(GtkSourceLanguageManager* language_manager)
{
	Glib::RefPtr<Gtk::Builder> builder =
		Gtk::Builder::create_from_resource(
			"/de/0x539/gobby/ui/menu.ui");

	m_app_menu = Glib::RefPtr<Gio::Menu>::cast_dynamic(
		builder->get_object("appmenu"));
	m_menu = Glib::RefPtr<Gio::Menu>::cast_dynamic(
		builder->get_object("winmenu"));

	Glib::RefPtr<Gio::Menu> highlight_mode_menu = get_highlight_mode_menu();

	const gchar* const* language_ids =
		gtk_source_language_manager_get_language_ids(
			language_manager);
	if(language_ids != NULL)
	{
		typedef std::list<GtkSourceLanguage*> LanguageList;
		typedef std::map<std::string, LanguageList> LanguageMap;
		LanguageMap languages;

		for(const gchar* const* id = language_ids; *id != NULL; ++id)
		{
			GtkSourceLanguage* language =
				gtk_source_language_manager_get_language(
					language_manager, *id);
			if(gtk_source_language_get_hidden(language)) continue;

			const std::string section =
				gtk_source_language_get_section(language);

			languages[section].push_back(language);
		}

		for(LanguageMap::iterator iter = languages.begin();
		    iter != languages.end(); ++iter)
		{
			Glib::RefPtr<Gio::Menu> submenu(Gio::Menu::create());

			LanguageList& list = iter->second;
			list.sort(language_sort_func);

			for(LanguageList::const_iterator liter = list.begin();
			    liter != list.end(); ++liter)
			{
				GtkSourceLanguage* language = *liter;

				const std::string id =
					gtk_source_language_get_id(language);
				const std::string name =
					gtk_source_language_get_name(language);

				Glib::RefPtr<Gio::MenuItem> item(
					Gio::MenuItem::create(
						name, Glib::ustring::compose(
							"win.highlight-mode"
							"('%1')", id)));
				submenu->append_item(item);
			}

			highlight_mode_menu->append_submenu(
				iter->first, submenu);
		}
	}
}

Glib::RefPtr<Gio::Menu> Gobby::MenuManager::get_highlight_mode_menu()
{
	// TODO: Is there a better way, like finding this by some
	// sort of ID?
	// TODO: This should be failsafe
	Glib::RefPtr<Gio::MenuModel> view_menu =
		m_menu->get_item_link(2, Gio::MENU_LINK_SUBMENU);

	Glib::RefPtr<Gio::MenuModel> last_section =
		view_menu->get_item_link(
			view_menu->get_n_items() - 1, Gio::MENU_LINK_SECTION);

	Glib::RefPtr<Gio::MenuModel> highlight_mode =
		last_section->get_item_link(0, Gio::MENU_LINK_SUBMENU);

	return Glib::RefPtr<Gio::Menu>::cast_dynamic(highlight_mode);
}
