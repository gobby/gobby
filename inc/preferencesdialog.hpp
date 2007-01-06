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

#ifndef _GOBBY_PREFERENCESDIALOG_HPP_
#define _GOBBY_PREFERENCESDIALOG_HPP_

#include <gtkmm/dialog.h>
#include <gtkmm/frame.h>
#include <gtkmm/box.h>
#include <gtkmm/expander.h>
#include <gtkmm/label.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/notebook.h>
#include <gtkmm/tooltips.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treeview.h>
#include <gtkmm/cellrenderercombo.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/fontselection.h>
#include "preferences.hpp"

namespace Gobby
{

class PreferencesDialog : public Gtk::Dialog
{
public:
	typedef Glib::RefPtr<Gtk::SourceLanguagesManager> LangManager;

	class Page: public Gtk::Frame
	{
	public:
		Page();

	protected:
	};

	class Editor: public Page
	{
	public:
		Editor(const Preferences& preferences,
		       Gtk::Tooltips& tooltips);

		void set(Preferences::Editor& editor) const;

	protected:
		Gtk::VBox m_box;
		Gtk::Frame m_frame_tab;
		Gtk::Frame m_frame_indentation;
		Gtk::Frame m_frame_homeend;

		Gtk::VBox m_box_tab;
		Gtk::HBox m_box_tab_width;
		Gtk::Label m_lbl_tab_width;
		Gtk::SpinButton m_ent_tab_width;
		Gtk::CheckButton m_btn_tab_spaces;

		Gtk::VBox m_box_indentation;
		Gtk::CheckButton m_btn_indentation_auto;

		Gtk::VBox m_box_homeend;
		Gtk::CheckButton m_btn_homeend_smart;
	};

	class View: public Page
	{
	public:
		View(const Preferences& preferences);
		void set(Preferences::View& view) const;

	protected:
		virtual void on_margin_display_toggled();

		Gtk::VBox m_box;
		Gtk::Frame m_frame_wrap;
		Gtk::Frame m_frame_linenum;
		Gtk::Frame m_frame_curline;
		Gtk::Frame m_frame_margin;
		Gtk::Frame m_frame_bracket;

		Gtk::VBox m_box_wrap;
		Gtk::CheckButton m_btn_wrap_text;
		Gtk::CheckButton m_btn_wrap_words;

		Gtk::VBox m_box_linenum;
		Gtk::CheckButton m_btn_linenum_display;

		Gtk::VBox m_box_curline;
		Gtk::CheckButton m_btn_curline_highlight;

		Gtk::VBox m_box_margin;
		Gtk::CheckButton m_btn_margin_display;
		Gtk::HBox m_box_margin_pos;
		Gtk::Label m_lbl_margin_pos;
		Gtk::SpinButton m_ent_margin_pos;

		Gtk::VBox m_box_bracket;
		Gtk::CheckButton m_btn_bracket_highlight;
	};

	class Appearance: public Page
	{
	public:
		Appearance(const Preferences& preferences);
		void set(Preferences::Appearance& appearance) const;

	protected:
		Gtk::VBox m_box;
		Gtk::Frame m_frame_toolbar;
		Gtk::Frame m_frame_windows;

		Gtk::VBox m_box_toolbar;
		Gtk::ComboBoxText m_cmb_toolbar_style;

		Gtk::VBox m_box_windows;
		Gtk::CheckButton m_btn_remember;
		Gtk::CheckButton m_btn_urgency_hint;
	};

	class Font: public Page
	{
	public:
		Font(const Preferences& preferences);
		void set(Preferences::Font& font) const;

	protected:
		void on_fontsel_realize();

		Gtk::FontSelection m_font_sel;
		Glib::ustring m_init_font;
	};

	class FileList: public Page
	{
	public:
		typedef Glib::RefPtr<Gtk::SourceLanguage> Language;

		// List of languages. TODO: Should be somewhere else
		class LanguageColumns: public Gtk::TreeModel::ColumnRecord
		{
		public:
			LanguageColumns();

			Gtk::TreeModelColumn<Language> language;
			Gtk::TreeModelColumn<Glib::ustring> language_name;
		};

		class FileColumns: public Gtk::TreeModel::ColumnRecord
		{
		public:
			FileColumns();

			Gtk::TreeModelColumn<Glib::ustring> pattern;
			Gtk::TreeModelColumn<Glib::ustring> mime_type;
			Gtk::TreeModelColumn<Gtk::TreeIter> language;
		};

		FileList(Gtk::Window& parent,
		         const Preferences& preferences,
		         const LangManager& lang_mgr);

		void set(Preferences::FileList& files) const;

		const LanguageColumns lang_columns;
		const FileColumns file_columns;

	protected:
		struct LangCompare
		{
			bool operator()(const Language& first,
			                const Language& second)
			{
				return first->gobj() < second->gobj();
			}
		};

		typedef std::map<
			Glib::RefPtr<Gtk::SourceLanguage>,
			Gtk::TreeIter,
			LangCompare
		> map_type;

		void cell_data_file_language(Gtk::CellRenderer* renderer,
		                             const Gtk::TreeIter& iter);

		void on_pattern_edited(const Glib::ustring& path,
		                       const Glib::ustring& new_text);
		void on_mimetype_edited(const Glib::ustring& path,
		                        const Glib::ustring& new_text);
		void on_language_edited(const Glib::ustring& path,
		                        const Glib::ustring& new_text);

		void on_selection_changed();

		void on_file_add();
		void on_file_remove();

		void set_language(const Gtk::TreeIter& row,
		                  const Language& lang);

		Gtk::Window& m_parent;
		const LangManager& m_lang_mgr;

		Gtk::CellRendererText* m_renderer_pattern;
		Gtk::CellRendererCombo m_renderer_lang;
		Gtk::CellRendererText* m_renderer_mimetype;

		Gtk::TreeViewColumn m_viewcol_pattern;
		Gtk::TreeViewColumn m_viewcol_lang;
		Gtk::TreeViewColumn m_viewcol_mimetype;

		Gtk::VBox m_vbox;
		Gtk::Label m_intro;
		Gtk::ScrolledWindow m_wnd;
		Gtk::TreeView m_view;

		Gtk::HButtonBox m_hbox;
		Gtk::Button m_btn_add;
		Gtk::Button m_btn_remove;

		// Map for better access to iterators to the language list
		map_type m_lang_map;

		Glib::RefPtr<Gtk::ListStore> m_lang_list;
		Glib::RefPtr<Gtk::ListStore> m_file_list;
	};

	PreferencesDialog(Gtk::Window& parent,
	                  const Preferences& preferences,
	                  const LangManager& lang_mgr,
	                  bool local);

	void set(Preferences& preferences) const;

#if 0
	const Editor& editor() const;
	const View& view() const;
	const Appearance& appearance() const;
#endif

protected:
	Gtk::Notebook m_notebook;
	Gtk::Tooltips m_tooltips;

	Editor m_page_editor;
	View m_page_view;
	Appearance m_page_appearance;
	Font m_page_font;
	FileList m_page_files;
};

}

#endif // _GOBBY_PREFERENCESDIALOG_HPP_

