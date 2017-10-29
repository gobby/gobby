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
#include "util/i18n.hpp"

#include "features.hpp"

#include <gtkmm/toolbar.h>
#include <giomm/settings.h>

#include <gtksourceview/gtksource.h>

#include <libinfinity/common/inf-xmpp-connection.h>
#include <libinfinity/common/inf-keepalive.h>

namespace Gobby
{

template<typename Type>
struct SettingTraits
{
	inline static Type
	get(const Glib::RefPtr<Gio::Settings>& settings,
	    const Glib::ustring& key)
	{
		Glib::Variant<Type> base;
		settings->get_value(key, base);
		return Type(base.get());
	}

	inline static bool
	get_from_entry(const Config::ParentEntry& entry,
	               const Glib::ustring& key,
	               Type& value)
	{
		if(entry.has_value(key))
		{
			value = entry.get_value<Type>(key);
			return true;
		}

		return false;
	}

	inline static void
	set(const Glib::RefPtr<Gio::Settings>& settings,
	    const Glib::ustring& key,
	    const Type& value)
	{
		Glib::Variant<Type> var(Glib::Variant<Type>::create(value));
		settings->set_value(key, var);
	}

	inline static bool equals(const Type& val1, const Type& val2)
	{
		return val1 == val2;
	}
};

// This cannot go through the generic case defined above because
// Glib::Variant<std::string> would map to a byte array, not a string.
template<>
struct SettingTraits<std::string>
{
	inline static std::string
	get(const Glib::RefPtr<Gio::Settings>& settings,
	    const Glib::ustring& key)
	{
		return settings->get_string(key);
	}

	inline static bool
	get_from_entry(const Config::ParentEntry& entry,
	               const Glib::ustring& key,
	               std::string& value)
	{
		if(entry.has_value(key))
		{
			value = entry.get_value<std::string>(key);
			return true;
		}

		return false;
	}

	inline static void
	set(const Glib::RefPtr<Gio::Settings>& settings,
	    const Glib::ustring& key,
	    const std::string& value)
	{
		settings->set_string(key, value);
	}

	inline static bool equals(const std::string& val1,
	                          const std::string& val2)
	{
		return val1 == val2;
	}
};



template<>
struct SettingTraits<Pango::FontDescription>
{
	inline static Pango::FontDescription
	get(const Glib::RefPtr<Gio::Settings>& settings,
	    const Glib::ustring& key)
	{
		return Pango::FontDescription(
			settings->get_string(key));
	}

	inline static bool
	get_from_entry(const Config::ParentEntry& entry,
	               const Glib::ustring& key,
	               Pango::FontDescription& value)
	{
		if(entry.has_value(key))
		{
			value = Pango::FontDescription(
				entry.get_value<Glib::ustring>(key));
			return true;
		}

		return false;
	}

	inline static void
	set(const Glib::RefPtr<Gio::Settings>& settings,
	    const Glib::ustring& key,
	    const Pango::FontDescription& value)
	{
		settings->set_string(key, value.to_string());
	}

	inline static bool equals(const Pango::FontDescription& val1,
	                          const Pango::FontDescription& val2)
	{
		return val1 == val2;
	}
};

template<>
struct SettingTraits<InfKeepalive>
{
	inline static InfKeepalive
	get(const Glib::RefPtr<Gio::Settings>& settings,
	    const Glib::ustring& key)
	{
		// Decode (asbuu)
		Glib::VariantBase base;
		settings->get_value(key, base);

		Glib::VariantContainerBase container =
			Glib::VariantBase::cast_dynamic<
				Glib::VariantContainerBase>(base);

		Glib::Variant<std::vector<Glib::ustring> > mask =
			Glib::VariantBase::cast_dynamic<
				Glib::Variant<std::vector<Glib::ustring> > >(
					container.get_child(0));
		Glib::Variant<bool> enabled =
			Glib::VariantBase::cast_dynamic<
				Glib::Variant<bool> >(
					container.get_child(1));
		Glib::Variant<guint> interval =
			Glib::VariantBase::cast_dynamic<
				Glib::Variant<guint> >(
					container.get_child(2));
		Glib::Variant<guint> time =
			Glib::VariantBase::cast_dynamic<
				Glib::Variant<guint> >(
					container.get_child(3));

		InfKeepalive keepalive;
		keepalive.mask = static_cast<InfKeepaliveMask>(0);

		std::vector<Glib::ustring> mask_values = mask.get();

		GFlagsClass* klass = G_FLAGS_CLASS(
			g_type_class_ref(INF_TYPE_KEEPALIVE_MASK));

		for(std::vector<Glib::ustring>::const_iterator iter =
			mask_values.begin();
		    iter != mask_values.end(); ++iter)
		{
			guint i;
			for(i = 0; i < klass->n_values; ++i)
			{
				const int is_all = strcmp(
					klass->values[i].value_nick, "all");
				if(is_all == 0)
					continue;

				if(*iter == klass->values[i].value_nick)
					break;
			}

			if(i == klass->n_values)
			{
				g_type_class_unref(klass);
				throw std::runtime_error(
					Glib::ustring::compose(
						_("'%1' is not a valid "
						  "keepalive mask value"),
						*iter));
			}

			keepalive.mask = static_cast<InfKeepaliveMask>(
				static_cast<int>(keepalive.mask) |
				klass->values[i].value);
		}

		g_type_class_unref(klass);

		keepalive.enabled = enabled.get();
		keepalive.time = time.get();
		keepalive.interval = interval.get();
		return keepalive;
	}

	inline static bool
	get_from_entry(const Config::ParentEntry& entry,
	               const Glib::ustring& key,
	               InfKeepalive& value)
	{
		bool result = false;

		if(entry.has_value(key + "-mask"))
		{
			value.mask = static_cast<InfKeepaliveMask>(
				entry.get_value<int>(key + "-mask"));
			result = true;
		}

		if(entry.has_value(key + "-enabled"))
		{
			value.enabled = entry.get_value<bool>(
				key + "-enabled");
			result = true;
		}

		if(entry.has_value(key + "-time"))
		{
			value.time = entry.get_value<guint>(
				key + "-time");
			result = true;
		}

		if(entry.has_value(key + "-interval"))
		{
			value.interval = entry.get_value<guint>(
				key + "-interval");
			result = true;
		}

		return result;
	}

	inline static void
	set(const Glib::RefPtr<Gio::Settings>& settings,
	    const Glib::ustring& key,
	    const InfKeepalive& value)
	{
		std::vector<Glib::ustring> mask_values;

		GFlagsClass* klass = G_FLAGS_CLASS(
			g_type_class_ref(INF_TYPE_KEEPALIVE_MASK));

		for(guint i = 0; i < klass->n_values; ++i)
		{
			if(strcmp(klass->values[i].value_nick, "all") == 0)
				continue;

			const guint mask = static_cast<guint>(value.mask);
			if(mask & klass->values[i].value)
			{
				mask_values.push_back(
					klass->values[i].value_nick);
			}
		}

		g_type_class_unref(klass);

		Glib::Variant<std::vector<Glib::ustring> > mask =
			Glib::Variant<std::vector<Glib::ustring> >::create(
				mask_values);
		Glib::Variant<bool> enabled =
			Glib::Variant<bool>::create(value.enabled);
		Glib::Variant<guint> time =
			Glib::Variant<guint>::create(value.time);
		Glib::Variant<guint> interval =
			Glib::Variant<guint>::create(value.interval);

		std::vector<Glib::VariantBase> tuple_children;
		tuple_children.push_back(mask);
		tuple_children.push_back(enabled);
		tuple_children.push_back(time);
		tuple_children.push_back(interval);

		Glib::VariantContainerBase entry =
			Glib::VariantContainerBase::create_tuple(
				tuple_children);

		settings->set_value(key, entry);
	}

	inline static bool equals(const InfKeepalive& val1,
	                          const InfKeepalive& val2)
	{
		return val1.mask == val2.mask &&
		       val1.enabled == val2.enabled &&
		       val1.time == val2.time &&
		       val1.interval == val2.interval;
	}
};

template<typename EnumType>
struct SettingTraitsEnum
{
	inline static EnumType
	get(const Glib::RefPtr<Gio::Settings>& settings,
	    const Glib::ustring& key)
	{
		return static_cast<EnumType>(settings->get_enum(key));
	}

	inline static bool
	get_from_entry(const Config::ParentEntry& entry,
	               const Glib::ustring& key,
	               EnumType& value)
	{
		if(entry.has_value(key))
		{
			value = static_cast<EnumType>(
				entry.get_value<int>(key));
			return true;
		}

		return false;
	}

	inline static void
	set(const Glib::RefPtr<Gio::Settings>& settings,
	    const Glib::ustring& key,
	    const EnumType& value)
	{
		g_settings_set_enum(
			settings->gobj(), key.c_str(),
			static_cast<int>(value));
	}

	inline static bool equals(const EnumType& val1, const EnumType& val2)
	{
		return val1 == val2;
	}
};

template<typename FlagsType>
struct SettingTraitsFlags
{
	inline static FlagsType
	get(const Glib::RefPtr<Gio::Settings>& settings,
	    const Glib::ustring& key)
	{
		return static_cast<FlagsType>(settings->get_flags(key));
	}

	inline static bool
	get_from_entry(const Config::ParentEntry& entry,
	               const Glib::ustring& key,
	               FlagsType& value)
	{
		if(entry.has_value(key))
		{
			value = static_cast<FlagsType>(
				entry.get_value<int>(key));
			return true;
		}

		return false;
	}

	inline static void
	set(const Glib::RefPtr<Gio::Settings>& settings,
	    const Glib::ustring& key,
	    const FlagsType& value)
	{
		g_settings_set_flags(
			settings->gobj(), key.c_str(),
			static_cast<int>(value));
	}

	inline static bool equals(const FlagsType& val1, const FlagsType& val2)
	{
		return val1 == val2;
	}
};

template<>
struct SettingTraits<Gtk::WrapMode>:
	SettingTraitsEnum<Gtk::WrapMode> {};

template<>
struct SettingTraits<Gtk::ToolbarStyle>:
	SettingTraitsEnum<Gtk::ToolbarStyle> {};

template<>
struct SettingTraits<InfXmppConnectionSecurityPolicy>:
	SettingTraitsEnum<InfXmppConnectionSecurityPolicy> {};

template<>
struct SettingTraits<GtkSourceDrawSpacesFlags>:
	SettingTraitsFlags<GtkSourceDrawSpacesFlags> {};

class Preferences
{
public:
	// TODO: This class should handle writability
	template<typename Type>
	class Option
	{
	public:
		typedef sigc::signal<void> signal_changed_type;

		Option(const Type& initial_value):
			m_value(initial_value), m_changed_handler(0) {} // TODO: Remove this constructor

		Option(const Glib::RefPtr<Gio::Settings>& settings,
		       const Glib::ustring& key):
			m_settings(settings), m_key(key),
			m_value(SettingTraits<Type>::get(settings, key))
		{
			m_changed_handler = g_signal_connect(
				G_OBJECT(settings->gobj()),
				Glib::ustring::compose(
					"changed::%1", key).c_str(),
				G_CALLBACK(on_changed_static),
				this);
		}

		Option(const Glib::RefPtr<Gio::Settings>& settings,
		       const Config::ParentEntry& entry,
		       const Glib::ustring& key):
			m_settings(settings), m_key(key),
			m_value(SettingTraits<Type>::get(settings, key))
		{
			m_changed_handler = g_signal_connect(
				G_OBJECT(settings->gobj()),
				Glib::ustring::compose(
					"changed::%1", key).c_str(),
				G_CALLBACK(on_changed_static),
				this);

			if(SettingTraits<Type>::get_from_entry(entry, key, m_value))
				set_settings();
		}

		~Option()
		{
			if(m_changed_handler != 0)
			{
				g_signal_handler_disconnect(
					m_settings->gobj(),
					m_changed_handler);
			}
		}

		bool is_default() const
		{
			Glib::VariantBase current_variant;
			Glib::VariantBase default_variant;
			m_settings->get_value(m_key, current_variant);
			m_settings->get_default_value(m_key, default_variant);
			return current_variant.equal(default_variant);
		}

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

	private:
		static void on_changed_static(GSettings* settings,
		                              const gchar* key,
		                              gpointer user_data)
		{
			static_cast<Option*>(user_data)->on_changed();
		}

		void on_changed()
		{
			const Type new_value(
				SettingTraits<Type>::get(m_settings, m_key));
			if(!SettingTraits<Type>::equals(new_value, m_value))
			{
				m_value = new_value;
				m_signal_changed.emit();
			}
		}

		void notify() const
		{
			// Notify GSettings about the changed value
			set_settings();

			// Notify other users about the changed value
			m_signal_changed.emit();
		}

		void set_settings() const
		{
			if(m_settings)
			{
				g_signal_handler_block(m_settings->gobj(),
				                       m_changed_handler);

				SettingTraits<Type>::set(
					m_settings, m_key, m_value);

				g_signal_handler_unblock(m_settings->gobj(),
				                         m_changed_handler);
			}
		}

	protected:
		Glib::RefPtr<Gio::Settings> m_settings;
		const Glib::ustring m_key;
		gulong m_changed_handler;

		Type m_value;
		signal_changed_type m_signal_changed;
	};

	Preferences(Config& m_config);

	class User
	{
	public:
		User(const Glib::RefPtr<Gio::Settings>& settings,
		     Config::ParentEntry& entry);

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
		Editor(const Glib::RefPtr<Gio::Settings>& settings,
		       Config::ParentEntry& entry);

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
		View(const Glib::RefPtr<Gio::Settings>& settings,
		     Config::ParentEntry& entry);

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
		Appearance(const Glib::RefPtr<Gio::Settings>& settings,
		           Config::ParentEntry& entry);

		// TODO: Option<bool> use_system_default_toolbar_style
		// (sets toolbar_style to default).
		Option<Gtk::ToolbarStyle> toolbar_style;
		Option<Pango::FontDescription> font;

		Option<Glib::ustring> scheme_id;

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
		Security(const Glib::RefPtr<Gio::Settings>& settings,
		         Config::ParentEntry& entry);

		Option<bool> use_system_trust;
		Option<std::string> trusted_cas;
		Option<InfXmppConnectionSecurityPolicy> policy;

		Option<bool> authentication_enabled;
		Option<std::string> certificate_file;
		Option<std::string> key_file;
	};

	class Network
	{
	public:
		Network(const Glib::RefPtr<Gio::Settings>& settings,
		        Config::ParentEntry& entry);

		Option<InfKeepalive> keepalive;
	};

private:
	Glib::RefPtr<Gio::Settings> m_settings;

public:
	User user;
	Editor editor;
	View view;
	Appearance appearance;
	Security security;
	Network network;
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
