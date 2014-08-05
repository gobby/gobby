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

#ifndef _GOBBY_GTK_COMPAT_HPP_
#define _GOBBY_GTK_COMPAT_HPP_

#include <gtkmmconfig.h>

#if GTKMM_MAJOR_VERSION == 3 || \
       (GTKMM_MAJOR_VERSION == 2 && GTKMM_MINOR_VERSION >= 90)
# define USE_GTKMM3
#endif

#include <gtkmm/stock.h>
#include <gtkmm/combobox.h>
#include <gtkmm/notebook.h>
#ifndef USE_GTKMM3
#include <gtkmm/comboboxentry.h>
#include <gtksourceview/gtksourceiter.h>
#endif

#include <gdk/gdkkeysyms.h>
#if !GTK_CHECK_VERSION(2,22,0)
#define GDK_KEY_Up GDK_Up
#define GDK_KEY_Down GDK_Down
#define GDK_KEY_Page_Up GDK_Page_Up
#define GDK_KEY_Page_Down GDK_Page_Down
#define GDK_KEY_0 GDK_0
#define GDK_KEY_1 GDK_1
#define GDK_KEY_2 GDK_2
#define GDK_KEY_3 GDK_3
#define GDK_KEY_4 GDK_4
#define GDK_KEY_5 GDK_5
#define GDK_KEY_6 GDK_6
#define GDK_KEY_7 GDK_7
#define GDK_KEY_8 GDK_8
#define GDK_KEY_9 GDK_9
#define GDK_KEY_m GDK_m
#define GDK_KEY_z GDK_z
#define GDK_KEY_Z GDK_Z
#endif

namespace Gobby
{

namespace GtkCompat
{

#ifdef USE_GTKMM3
class ComboBoxEntry: public Gtk::ComboBox
{
public:
	ComboBoxEntry(): Gtk::ComboBox(true) {}

	void set_text_column(const Gtk::TreeModelColumnBase& column) { set_entry_text_column(column); }
};
#else
typedef Gtk::ComboBoxEntry ComboBoxEntry;
#endif

#ifdef USE_GTKMM3
static const Gtk::Align ALIGN_LEFT = Gtk::ALIGN_START;
static const Gtk::Align ALIGN_RIGHT = Gtk::ALIGN_END;
static const Gtk::Align ALIGN_TOP = Gtk::ALIGN_START;
#else
static const Gtk::AlignmentEnum ALIGN_LEFT = Gtk::ALIGN_LEFT;
static const Gtk::AlignmentEnum ALIGN_RIGHT = Gtk::ALIGN_RIGHT;
static const Gtk::AlignmentEnum ALIGN_TOP = Gtk::ALIGN_TOP;
#endif

inline bool is_realized(const Gtk::Widget& widget)
{
#ifdef USE_GTKMM3
	return widget.get_realized();
#else
	return (widget.get_flags() & Gtk::REALIZED) != 0;
#endif
}

inline bool is_visible(const Gtk::Widget& widget)
{
#ifdef USE_GTKMM3
	return widget.get_visible();
#else
	return widget.is_visible();
#endif
}

#ifndef USE_GTKMM3
class Notebook: public Gtk::Notebook
{
protected:
	virtual void on_switch_page(GtkNotebookPage* page, guint page_num)
	{
		Gtk::Notebook::on_switch_page(page, page_num);
		on_switch_page(get_nth_page(page_num), page_num);
	}

	virtual void on_switch_page(Gtk::Widget* page, guint page_num) {}
};
#else
typedef Gtk::Notebook Notebook;
#endif

inline Glib::RefPtr<Gdk::Pixbuf> render_icon(Gtk::Widget& widget,
                                             const Gtk::StockID& stock_id,
                                             Gtk::IconSize size)
{
#ifdef USE_GTKMM3
	Glib::RefPtr<Gdk::Pixbuf> pixbuf = widget.render_icon_pixbuf(stock_id, size);
	if(!pixbuf) // icon not found
		pixbuf = widget.render_icon_pixbuf(Gtk::Stock::MISSING_IMAGE, size);
	g_assert(pixbuf);
	return pixbuf;
#else
	return widget.render_icon(stock_id, size);
#endif
}

#ifdef USE_GTKMM3
typedef GtkTextSearchFlags TextSearchFlags;
static const TextSearchFlags TEXT_SEARCH_CASE_INSENSITIVE =
	GTK_TEXT_SEARCH_CASE_INSENSITIVE;
#else
typedef GtkSourceSearchFlags TextSearchFlags;
static const TextSearchFlags TEXT_SEARCH_CASE_INSENSITIVE =
	GTK_SOURCE_SEARCH_CASE_INSENSITIVE;
#endif

inline gboolean text_iter_forward_search(const GtkTextIter* iter,
                                         const gchar* str,
                                         TextSearchFlags flags,
                                         GtkTextIter* match_start,
                                         GtkTextIter* match_end,
                                         const GtkTextIter* limit)
{
#ifdef USE_GTKMM3
	return gtk_text_iter_forward_search(iter, str, flags,
	                                    match_start, match_end, limit);
#else
	return gtk_source_iter_forward_search(iter, str, flags,
	                                      match_start, match_end, limit);
#endif
}

inline gboolean text_iter_backward_search(const GtkTextIter* iter,
                                          const gchar* str,
                                          TextSearchFlags flags,
                                          GtkTextIter* match_start,
                                          GtkTextIter* match_end,
                                          const GtkTextIter* limit)
{
#ifdef USE_GTKMM3
	return gtk_text_iter_backward_search(iter, str, flags,
	                                     match_start, match_end, limit);
#else
	return gtk_source_iter_backward_search(iter, str, flags,
	                                       match_start, match_end, limit);
#endif
}

#ifdef USE_GTKMM3
typedef Glib::RefPtr<Gtk::IconSet> IconSet;
inline IconSet create_icon_set() { return Gtk::IconSet::create(); }
#else
class IconSet: public Gtk::IconSet
{
public:
	IconSet* operator->() { return this; }
	const IconSet* operator->() const { return this; }
};
inline IconSet create_icon_set() { return IconSet(); }
#endif

#ifdef USE_GTKMM3
inline Gdk::Color color_from_rgba(const Gdk::RGBA& rgba)
{
	Gdk::Color c;
	c.set_red(rgba.get_red_u());
	c.set_green(rgba.get_green_u());
	c.set_blue(rgba.get_blue_u());
	return c;
}
#endif

} // namespace GtkCompat

} // namespace Gobby

#endif // _GOBBY_GTK_COMPAT_HPP_
