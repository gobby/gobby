/* gobby - A GTKmm driven libobby client
 * Copyright (C) 2005 0x539 dev group
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <cstring>
#include <cerrno>
#include <stdexcept>

#include <glibmm/miscutils.h>
#include <glibmm/fileutils.h>
#include <glibmm/exception.h>
#include <libxml++/nodes/textnode.h>
#include <libxml++/parsers/domparser.h>
#include <libxml++/exceptions/exception.h>

#include "config_.hpp"

Gobby::Config::Error::Error(Code error_code, const Glib::ustring& error_message)
 : Glib::Error(g_quark_from_static_string("GOBBY_CONFIG_ERROR"),
               static_cast<int>(error_code), error_message)
{
}

Gobby::Config::Error::Code Gobby::Config::Error::code() const
{
	return static_cast<Code>(gobject_->code);
}

Gobby::Config::Entry::Entry()
{
}

Gobby::Config::Entry::Entry(const Entry& other)
 : m_table(other.m_table), m_value(other.m_value)
{
}

Gobby::Config::Entry::~Entry()
{
}

Gobby::Config::Entry& Gobby::Config::Entry::operator=(const Entry& other)
{
	if(&other == this)
		return *this;

	m_table = other.m_table;
	m_value = other.m_value;

	return *this;
}

void Gobby::Config::Entry::load(const xmlpp::Element& element)
{
	if(element.get_child_text() )
		if(!element.get_child_text()->is_white_space() )
			m_value = element.get_child_text()->get_content();

	xmlpp::Node::NodeList list = element.get_children();
	xmlpp::Node::NodeList::iterator iter;
	for(iter = list.begin(); iter != list.end(); ++ iter)
	{
		xmlpp::Element* child = dynamic_cast<xmlpp::Element*>(*iter);
		if(!child) continue;
		
		m_table[child->get_name()].load(*child);
	}
}

void Gobby::Config::Entry::save(xmlpp::Element& element) const
{
	element.set_child_text(m_value);

	std::map<Glib::ustring, Entry>::const_iterator i;
	for(i = m_table.begin(); i != m_table.end(); ++ i)
	{
		xmlpp::Element* child = element.add_child(i->first);
		i->second.save(*child);
	}
}

Gobby::Config::Entry&
Gobby::Config::Entry::operator[](const Glib::ustring& index)
{
	return m_table[index];
}

Gobby::Config::Config(const Glib::ustring& file)
 : m_filename(file)
{
	xmlpp::DomParser parser;

	try
	{
		parser.parse_file(file);
	}
	catch(xmlpp::exception& e)
	{
		// Empty config
		return;
	}

	xmlpp::Document* document = parser.get_document();
	xmlpp::Element* root = document->get_root_node();

	xmlpp::Node::NodeList list = root->get_children();
	xmlpp::Node::NodeList::iterator iter;
	for(iter = list.begin(); iter != list.end(); ++ iter)
	{
		xmlpp::Element* child = dynamic_cast<xmlpp::Element*>(*iter);
		if(!child) continue;

		m_table[child->get_name()].load(*child);
	}
}

Gobby::Config::~Config()
{
	xmlpp::Document document;
	xmlpp::Element* root = document.create_root_node("gobby_config");

	std::map<Glib::ustring, Entry>::iterator i;
	for(i = m_table.begin(); i != m_table.end(); ++ i)
	{
		xmlpp::Element* child = root->add_child(i->first);
		i->second.save(*child);
	}

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
		g_warning("Could not write conifg file: %s", e.what() );
	}
}

Gobby::Config::Entry& Gobby::Config::operator[](const Glib::ustring& index)
{
	return m_table[index];
}

void Gobby::Config::create_path_to(const Glib::ustring& to)
{
	// Directory exists, nothing to do
	if(Glib::file_test(to, Glib::FILE_TEST_IS_DIR) )
		return;

	// Find path to the directory to create
	Glib::ustring path_to = Glib::path_get_dirname(to);

	// Create this path, if it doesn't exists
	create_path_to(path_to);

	// Create new directory
	if(mkdir(to.c_str(), 0755) == -1)
		throw Error(Error::PATH_CREATION_FAILED,
		            "Could not create directory " + to + ": " +
		            strerror(errno) );
}

std::ostream& Gobby::operator<<(std::ostream& out, const Gdk::Color& color)
{
	unsigned int red = color.get_red() * 255 / 65535;
	unsigned int green = color.get_green() * 255 / 65535;
	unsigned int blue = color.get_blue() * 255 / 65535;

	out << std::hex << ( (red << 16) | (green << 8) | blue);
	return out;
}

std::istream& Gobby::operator>>(std::istream& in, Gdk::Color& color)
{
	unsigned int rgb_color;
	in >> std::hex >> rgb_color;

	color.set_red( ((rgb_color >> 16) & 0xff) * 65535 / 255);
	color.set_green( ((rgb_color >> 8) & 0xff) * 65535 / 255);
	color.set_blue( ((rgb_color) & 0xff) * 65535 / 255);
	return in;
}

