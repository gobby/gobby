/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2013 Armin Burgmeier <armin@arbur.net>
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

#ifndef _GOBBY_HISTORYENTRY_HPP_
#define _GOBBY_HISTORYENTRY_HPP_

#include <gtkmm/entry.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/liststore.h>

#include "util/gtk-compat.hpp"

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

class HistoryComboBoxEntry: public GtkCompat::ComboBoxEntry
{
public:
  HistoryComboBoxEntry(const std::string& history_file, unsigned int length);
  HistoryComboBoxEntry(unsigned int length);

  void commit();

  Glib::RefPtr<Atk::Object> get_accessible();

protected:
  bool on_entry_key_press_event(GdkEventKey* event);

  History m_history;
};

} // namespace Gobby

#endif // _GOBBY_HISTORYENTRY_HPP_
