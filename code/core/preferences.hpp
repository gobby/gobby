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

#ifndef _GOBBY_PREFERENCES_HPP_
#define _GOBBY_PREFERENCES_HPP_

#include "util/config.hpp"

#include "features.hpp"

#include <gtksourceview/gtksourceview.h>
#include <gtksourceview/gtksourcestyleschememanager.h>

#include <gtkmm/toolbar.h>

#include <libinfinity/common/inf-xmpp-connection.h>

namespace Gobby
{

class Preferences
{
public:
	template<typename Type>
	class Option
	{
	public:
		typedef sigc::signal<void> signal_changed_type;

		Option(const Type& initial_value):
			m_value(initial_value) {}

		const Option<Type>& operator=(const Type& new_value)
		{
			m_value = new_value;
			notify();
			return *this;
		}

		void set(const Type& new_value)
		{
			*this = new_value;
		}

		const Type& get() const
		{
			return m_value;
		}

		operator const Type&() const
		{
			return m_value;
		}

		/*operator Type&()
		{
			return m_value;
		}*/

		signal_changed_type signal_changed() const
		{
			return m_signal_changed;
		}

		void notify() const
		{
			m_signal_changed.emit();
		}

	protected:
		Type m_value;
		signal_changed_type m_signal_changed;
	};

	/** Reads preferences values out of a config, using default values
	 * for values that do not exist in the config.
	 */
	Preferences(Config& m_config);

	/** Serialises preferences back to config.
	 */
	void serialize(Config& config) const;

	class User
	{
	public:
		User(Config::ParentEntry& entry);
		void serialize(Config::ParentEntry& entry) const;

		Option<Glib::ustring> name;
		Option<double> hue;
		Option<double> alpha;

		Option<bool> show_remote_cursors;
		Option<bool> show_remote_selections;
		Option<bool> show_remote_current_lines;
		Option<bool> show_remote_cursor_positions;

		Option<bool> allow_remote_access;
		Option<bool> require_password;
		Option<std::string> password;
		Option<unsigned int> port;
		Option<bool> keep_local_documents;
		Option<std::string> host_directory;
	};

	class Editor
	{
	public:
		Editor(Config::ParentEntry& entry);
		void serialize(Config::ParentEntry& entry) const;

		Option<unsigned int> tab_width;
		Option<bool> tab_spaces;
		Option<bool> indentation_auto;
		Option<bool> homeend_smart;
		Option<bool> autosave_enabled;
		Option<unsigned int> autosave_interval;
	};

	class View
	{
	public:
		View(Config::ParentEntry& entry);
		void serialize(Config::ParentEntry& entry) const;

		Option<Gtk::WrapMode> wrap_mode;
		Option<bool> linenum_display;
		Option<bool> curline_highlight;
		Option<bool> margin_display;
		Option<unsigned int> margin_pos;
		Option<bool> bracket_highlight;
		Option<GtkSourceDrawSpacesFlags> whitespace_display;
	};

	class Appearance
	{
	public:
		Appearance(Config::ParentEntry& entry);
		void serialize(Config::ParentEntry& entry) const;

		// TODO: Option<bool> use_system_default_toolbar_style
		// (sets toolbar_style by gconf). At least WITH_GNOME.
		Option<Gtk::ToolbarStyle> toolbar_style;
		Option<Pango::FontDescription> font;

		Option<Glib::ustring> scheme_id;

		Option<unsigned int> document_userlist_width;
		Option<unsigned int> chat_userlist_width;

		Option<bool> show_toolbar;
		Option<bool> show_statusbar;
		Option<bool> show_browser;
		Option<bool> show_chat;
		Option<bool> show_document_userlist;
		Option<bool> show_chat_userlist;
	};

	class Security
	{
	public:
		Security(Config::ParentEntry& entry);
		void serialize(Config::ParentEntry& entry) const;

		Option<std::string> trust_file;
		Option<InfXmppConnectionSecurityPolicy> policy;

		Option<bool> authentication_enabled;
		Option<std::string> key_file;
		Option<std::string> certificate_file;
	};

	User user;
	Editor editor;
	View view;
	Appearance appearance;
	Security security;
};

template<typename Type>
std::ostream& operator<<(std::ostream& stream,
                         const Preferences::Option<Type>& option)
{
	stream << static_cast<const Type&>(option);
	return stream;
}

}

#endif // _GOBBY_PREFERENCES_HPP_
