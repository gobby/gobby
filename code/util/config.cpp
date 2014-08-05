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

#include "util/config.hpp"
#include "util/file.hpp"
#include "util/i18n.hpp"

#include <glibmm/miscutils.h>
#include <glibmm/fileutils.h>
#include <glibmm/exception.h>
#include <libxml++/parsers/domparser.h>
#include <libxml++/exceptions/exception.h>

#include <stdexcept>

namespace
{
	template<typename Map>
	typename Map::mapped_type ptrmap_find(const Map& map,
	                                      const typename Map::key_type& key)
	{
		typename Map::const_iterator iter = map.find(key);
		if(iter == map.end() ) return NULL;
		return iter->second;
	}
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

bool Gobby::Config::ParentEntry::has_value(const Glib::ustring& name)
{
	return get_value_child(name) != NULL;
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
		m_root.reset(new ParentEntry("gobby-config") );
		return;
	}

	try
	{
		parser.parse_file(file);
	}
	catch(xmlpp::exception& e)
	{
		// Empty config
		m_root.reset(new ParentEntry("gobby-config") );
		return;
	}

	xmlpp::Document* document = parser.get_document();
	if(document == NULL)
	{
		m_root.reset(new ParentEntry("gobby-config") );
		return;
	}

	xmlpp::Element* root = document->get_root_node();

	// Config is present, but contains no root node
	if(root == NULL)
	{
		m_root.reset(new ParentEntry("gobby-config") );
		return;
	}

	m_root.reset(new ParentEntry(*root) );
}

Gobby::Config::~Config()
{
	xmlpp::Document document;
	xmlpp::Element* root = document.create_root_node("gobby-config");
	m_root->save(*root);

	try
	{
		Glib::ustring dirname = Glib::path_get_dirname(m_filename);
		create_directory_with_parents(dirname, 0700);

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

std::string Gobby::serialize::default_context_to<Gdk::Color>::
	to_string(const data_type& from) const
{
	unsigned int red = from.get_red() * 255 / 65535;
	unsigned int green = from.get_green() * 255 / 65535;
	unsigned int blue = from.get_blue() * 255 / 65535;

	std::stringstream stream;
	stream << std::hex << ( (red << 16) | (green << 8) | blue);
	return stream.str();
}

Gobby::serialize::default_context_from<Gdk::Color>::data_type
Gobby::serialize::default_context_from<Gdk::Color>::
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

std::string Gobby::serialize::default_context_to<Glib::ustring>::
	to_string(const data_type& from) const
{
	return from;
}

Gobby::serialize::default_context_from<Glib::ustring>::data_type
Gobby::serialize::default_context_from<Glib::ustring>::
	from_string(const std::string& from) const
{
	return from;
}
