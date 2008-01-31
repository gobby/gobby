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

// For mkdir / CreateDirectory
#ifdef WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <cerrno>
#endif

#include <cstring>
#include <stdexcept>

#include <glibmm/miscutils.h>
#include <glibmm/fileutils.h>
#include <glibmm/exception.h>
#include <libxml++/parsers/domparser.h>
#include <libxml++/exceptions/exception.h>

#include "config.hpp"

namespace
{
	// Creates a new directory
	void create_directory(const char* path)
	{
#ifdef WIN32
		if(CreateDirectoryA(path, NULL) == FALSE)
		{
			LPVOID msgbuf;
			DWORD err = GetLastError();

			FormatMessageA(
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM,
				NULL,
				err,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				reinterpret_cast<LPSTR>(&msgbuf),
				0,
				NULL
			);

			std::string error_message = static_cast<LPSTR>(msgbuf);
			LocalFree(msgbuf);

			// TODO: Convert to UTF-8?

			throw Gobby::Config::Error(
				Gobby::Config::Error::PATH_CREATION_FAILED,
				"Could not create directory " +
				std::string(path) + ": " + error_message
			);
		}
#else
		if(mkdir(path, 0755) == -1)
		{
			throw Gobby::Config::Error(
				Gobby::Config::Error::PATH_CREATION_FAILED,
				"Could not create directory " +
				std::string(path) + ": " + strerror(errno)
			);
		}
#endif
	}

	void create_path_to(const std::string& to)
	{
		// Directory exists, nothing to do
		if(Glib::file_test(to, Glib::FILE_TEST_IS_DIR) )
			return;

		// Find path to the directory to create
		Glib::ustring path_to = Glib::path_get_dirname(to);

		// Create this path, if it doesn't exists
		create_path_to(path_to);

		// Create new directory
		create_directory(to.c_str() );
	}

	template<typename Map>
	typename Map::mapped_type ptrmap_find(const Map& map,
	                                      const typename Map::key_type& key)
	{
		typename Map::const_iterator iter = map.find(key);
		if(iter == map.end() ) return NULL;
		return iter->second;
	}
}

Gobby::Config::Error::Error(Code error_code,
                            const Glib::ustring& error_message):
	Glib::Error(
		g_quark_from_static_string("GOBBY_CONFIG_ERROR"),
		static_cast<int>(error_code),
		error_message
	)
{
}

Gobby::Config::Error::Code Gobby::Config::Error::code() const
{
	return static_cast<Code>(gobject_->code);
}

Gobby::Config::Entry::Entry(const Glib::ustring& name):
	m_name(name)
{
}

const Glib::ustring& Gobby::Config::Entry::get_name() const
{
	return m_name;
}

Gobby::Config::ParentEntry::ParentEntry(const Glib::ustring& name):
	Entry(name)
{
}

Gobby::Config::ParentEntry::ParentEntry(const xmlpp::Element& elem):
	Entry(elem.get_name() )
{
	xmlpp::Node::NodeList list = elem.get_children();
	for(xmlpp::Node::NodeList::iterator iter = list.begin();
	    iter != list.end();
	    ++ iter)
	{
		xmlpp::Element* child = dynamic_cast<xmlpp::Element*>(*iter);
		if(child == NULL) continue;

		if(child->get_child_text() &&
		   !child->get_child_text()->is_white_space())
		{
			ValueEntry* entry = new TypedValueEntry<Glib::ustring>(
				*child
			);

			m_map[child->get_name()] = entry;
		}
		else
		{
			m_map[child->get_name()] = new ParentEntry(*child);
		}
	}
}

Gobby::Config::ParentEntry::~ParentEntry()
{
	for(map_type::iterator iter = m_map.begin();
	    iter != m_map.end();
	    ++ iter)
	{
		delete iter->second;
	}
}

void Gobby::Config::ParentEntry::save(xmlpp::Element& elem) const
{
	for(map_type::const_iterator iter = m_map.begin();
	    iter != m_map.end();
	    ++ iter)
	{
		Entry* entry = iter->second;
		xmlpp::Element* child = elem.add_child(entry->get_name() );
		entry->save(*child);
	}
}

Gobby::Config::Entry* Gobby::Config::ParentEntry::
	get_child(const Glib::ustring& name)
{
	return ptrmap_find(m_map, name);
}

const Gobby::Config::Entry* Gobby::Config::ParentEntry::
	get_child(const Glib::ustring& name) const
{
	return ptrmap_find(m_map, name);
}

Gobby::Config::ParentEntry* Gobby::Config::ParentEntry::
	get_parent_child(const Glib::ustring& name)
{
	return dynamic_cast<ParentEntry*>(get_child(name) );
}

const Gobby::Config::ParentEntry* Gobby::Config::ParentEntry::
	get_parent_child(const Glib::ustring& name) const
{
	return dynamic_cast<const ParentEntry*>(get_child(name) );
}

Gobby::Config::ValueEntry* Gobby::Config::ParentEntry::
	get_value_child(const Glib::ustring& name)
{
	return dynamic_cast<ValueEntry*>(get_child(name) );
}

const Gobby::Config::ValueEntry* Gobby::Config::ParentEntry::
	get_value_child(const Glib::ustring& name) const
{
	return dynamic_cast<const ValueEntry*>(get_child(name) );
}

Gobby::Config::ParentEntry& Gobby::Config::ParentEntry::
	operator[](const Glib::ustring& name)
{
	ParentEntry* entry = get_parent_child(name);
	if(entry != NULL) return *entry;
	return set_parent(name);
}

Gobby::Config::ParentEntry& Gobby::Config::ParentEntry::
	set_parent(const Glib::ustring& name)
{
	Entry* entry = get_child(name);
	if(entry != NULL) delete entry;

	ParentEntry* child = new ParentEntry(name);
	m_map[name] = child;
	return *child;
}

Gobby::Config::ParentEntry::iterator Gobby::Config::ParentEntry::begin()
{
	return iterator(m_map.begin() );
}

Gobby::Config::ParentEntry::const_iterator Gobby::Config::ParentEntry::
	begin() const
{
	return const_iterator(m_map.begin() );
}

Gobby::Config::ParentEntry::iterator Gobby::Config::ParentEntry::end()
{
	return iterator(m_map.end() );
}

Gobby::Config::ParentEntry::const_iterator Gobby::Config::ParentEntry::
	end() const
{
	return const_iterator(m_map.end() );
}

Gobby::Config::Config(const Glib::ustring& file):
	m_filename(file)
{
	xmlpp::DomParser parser;

	if(!Glib::file_test(file, Glib::FILE_TEST_IS_REGULAR))
	{
		m_root.reset(new ParentEntry("gobby_config") );
		return;
	}

	try
	{
		parser.parse_file(file);
	}
	catch(xmlpp::exception& e)
	{
		// Empty config
		m_root.reset(new ParentEntry("gobby_config") );
		return;
	}

	xmlpp::Document* document = parser.get_document();
	if(document == NULL)
	{
		m_root.reset(new ParentEntry("gobby_config") );
		return;
	}

	xmlpp::Element* root = document->get_root_node();

	// Config is present, but contains no root node
	if(root == NULL)
	{
		m_root.reset(new ParentEntry("gobby_config") );
		return;
	}

	m_root.reset(new ParentEntry(*root) );
}

Gobby::Config::~Config()
{
	xmlpp::Document document;
	xmlpp::Element* root = document.create_root_node("gobby_config");
	m_root->save(*root);

	try
	{
		Glib::ustring dirname = Glib::path_get_dirname(m_filename);
		create_path_to(dirname);

		document.write_to_file_formatted(m_filename, "UTF-8");
	}
	catch(Glib::Exception& e)
	{
		g_warning("Could not write config file: %s", e.what().c_str() );
	}
	catch(std::exception& e)
	{
		g_warning("Could not write config file: %s", e.what() );
	}
}

Gobby::Config::ParentEntry& Gobby::Config::get_root()
{
	return *m_root;
}

const Gobby::Config::ParentEntry& Gobby::Config::get_root() const
{
	return *m_root;
}

std::string serialise::default_context_to<Gdk::Color>::
	to_string(const data_type& from) const
{
	unsigned int red = from.get_red() * 255 / 65535;
	unsigned int green = from.get_green() * 255 / 65535;
	unsigned int blue = from.get_blue() * 255 / 65535;

	std::stringstream stream;
	stream << std::hex << ( (red << 16) | (green << 8) | blue);
	return stream.str();
}

serialise::default_context_from<Gdk::Color>::data_type
serialise::default_context_from<Gdk::Color>::
	from_string(const std::string& from) const
{
	unsigned int rgb_color;
	std::stringstream stream(from);
	stream >> std::hex >> rgb_color;

	if(stream.bad() )
	{
		throw conversion_error(
			from + " should be hexadecimal color triple"
		);
	}

	Gdk::Color color;
	color.set_red( ((rgb_color >> 16) & 0xff) * 65535 / 255);
	color.set_green( ((rgb_color >> 8) & 0xff) * 65535 / 255);
	color.set_blue( ((rgb_color) & 0xff) * 65535 / 255);
	return color;
}

std::string serialise::default_context_to<Glib::ustring>::
	to_string(const data_type& from) const
{
	return from;
}

serialise::default_context_from<Glib::ustring>::data_type
serialise::default_context_from<Glib::ustring>::
	from_string(const std::string& from) const
{
	return from;
}
