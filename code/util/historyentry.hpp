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

#ifndef _GOBBY_HISTORYENTRY_HPP_
#define _GOBBY_HISTORYENTRY_HPP_

#include <gtkmm/entry.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/liststore.h>
#include <gtkmm/combobox.h>
#include <gtkmm/builder.h>

#include <memory>

namespace Gobby
{

class History
{
public:
  class Columns: public Gtk::TreeModelColumnRecord
  {
  public:
    Columns() { add(text); }
    Gtk::TreeModelColumn<Glib::ustring> text;
  };

  History(const std::string& history_file, unsigned int length);
  History(unsigned int length);
  ~History();

  Glib::RefPtr<Gtk::ListStore> get_store();
  const Columns& get_columns() const;

  bool up(const Glib::ustring& current, Glib::ustring& entry);
  bool down(const Glib::ustring& current, Glib::ustring& entry);
  void commit(const Glib::ustring& str);

protected:
  void commit_noscroll(const Glib::ustring& str);

  const unsigned int m_length;

  const Columns m_history_columns;
  Glib::RefPtr<Gtk::ListStore> m_history;
  Gtk::TreeIter m_current;
  std::string m_history_file;

private:
  class Loader;
  std::auto_ptr<Loader> m_loader;
};

class HistoryEntry: public Gtk::Entry
{
public:
  HistoryEntry(const std::string& history_file, unsigned int length);
  HistoryEntry(unsigned int length);

  void commit();

protected:
  virtual bool on_key_press_event(GdkEventKey* event);

  History m_history;
};

class HistoryComboBox: public Gtk::ComboBox
{
public:
  HistoryComboBox(const Glib::RefPtr<Gtk::Builder>& builder, const char* id, const std::string& history_file, unsigned int length);
  HistoryComboBox(const std::string& history_file, unsigned int length);
  HistoryComboBox(unsigned int length);

  void commit();

  Glib::RefPtr<Atk::Object> get_accessible();

protected:
  bool on_entry_key_press_event(GdkEventKey* event);

  History m_history;
};

} // namespace Gobby

#endif // _GOBBY_HISTORYENTRY_HPP_
