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

#ifndef _GOBBY_CONFIG_HPP_
#define _GOBBY_CONFIG_HPP_

#include <map>
#include <memory>
#include <net6/non_copyable.hpp>
#include <net6/serialise.hpp>
#include <glibmm/error.h>
#include <glibmm/ustring.h>
#include <gdkmm/color.h>

// TODO: Use registry on windows, gconf with gnome
#include <libxml++/nodes/element.h>
#include <libxml++/nodes/textnode.h>

namespace Gobby
{

class Config: private net6::non_copyable
{
public:
	class Error: public Glib::Error
	{
	public:
		enum Code {
			PATH_CREATION_FAILED
		};

		Error(Code error_code, const Glib::ustring& error_message);
		Code code() const;
	};

	/** @brief Abstract base class for configuration file entries.
	 */
	class Entry: private net6::non_copyable
	{
	public:
		Entry(const Glib::ustring& name);
		virtual ~Entry() {} // compiler complains without

		/** @brief Saves this entry into the given element.
		 */
		virtual void save(xmlpp::Element& elem) const = 0;

		/** @brief Returns the name of this entry.
		 */
		const Glib::ustring& get_name() const;
	protected:
		Glib::ustring m_name;
	};

	/** @brief Entry that contains a value.
	 */
	class ValueEntry: public Entry
	{
	public:
		/** @brief Stores an object of the given type into this
		 * entry.
		 */
		template<typename DataType>
		ValueEntry(
			const Glib::ustring& name,
			const DataType& value,
			const serialise::context_base_to<DataType>& ctx =
				serialise::default_context_to<DataType>()
		);

		/** @brief Returns the value of this entry and tries to
		 * convert it to the requested type.
		 */
		template<typename DataType>
		DataType get(
			const serialise::context_base_from<DataType>& ctx =
			serialise::default_context_from<DataType>()
		) const;

	protected:
		serialise::data m_data;
	};

	/** Value entry with type information. Useful for future storage
	 * backends like gconf or windows registry.
	 */
	template<typename DataType>
	class TypedValueEntry: public ValueEntry
	{
	public:
		/** @brief Creates a new typed value entry that is converted
		 * to a string using the given context.
		 */
		TypedValueEntry(
			const Glib::ustring& name,
			const DataType& value,
			const serialise::context_base_to<DataType>& ctx =
				serialise::default_context_to<DataType>()
		);

		/** @brief Reads a value entry from a xml element.
		 */
		TypedValueEntry(const xmlpp::Element& elem);

		/** @brief Stores this entry into the given element.
		 */
		virtual void save(xmlpp::Element& elem) const;
	};

	/** @brief Entry containing child entries.
	 */
	class ParentEntry: public Entry
	{
	protected:
		typedef std::map<Glib::ustring, Entry*> map_type;

	public:
		template<typename BaseIterator, typename Entry>
		class iterator_base
		{
		public:
			typedef BaseIterator base_iterator;

			iterator_base(const base_iterator& iter);

			iterator_base& operator++();
			iterator_base operator++(int);

			bool operator==(const iterator_base& other) const;
			bool operator!=(const iterator_base& other) const;

			Entry& operator*() const;
			Entry* operator->() const;

		protected:
			base_iterator m_iter;
		};

		typedef iterator_base<map_type::iterator, Entry>
			iterator;
		typedef iterator_base<map_type::const_iterator, const Entry>
			const_iterator;

		/** @brief Creates a new ParentEntry of the given name with
		 * no children.
		 */
		ParentEntry(const Glib::ustring& name);

		/** @brief a new ParentEntry from the given xml element.
		 */
		ParentEntry(const xmlpp::Element& elem);
		virtual ~ParentEntry();

		/** @brief Stores this ParentEntry into the given xml element.
		 */
		virtual void save(xmlpp::Element& elem) const;

		/** @brief Returns a child entry with the given name.
		 *
		 * Returns NULL if there is no such child.
		 */
		Entry* get_child(const Glib::ustring& name);

		/** @brief Returns a child entry with the given name.
		 *
		 * Returns NULL if there is no such child.
		 */
		const Entry* get_child(const Glib::ustring& name) const;

		/** @brief Returns a child that is another parent entry
		 * and has the given name.
		 *
		 * Returns NULL if there is no such child.
		 */
		ParentEntry* get_parent_child(const Glib::ustring& name);

		/** @brief Returns a child that is another parent entry
		 * and has the given name.
		 *
		 * Returns NULL if there is no such child.
		 */
		const ParentEntry*
		get_parent_child(const Glib::ustring& name) const;

		/** @brief Returns a child that is a value entry and has
		 * the given name.
		 *
		 * Returns NULL if there is no such child.
		 */
		ValueEntry* get_value_child(const Glib::ustring& name);

		/** @brief Returns a child that is a value entry and has
		 * the given name.
		 *
		 * Returns NULL if there is no such child.
		 */
		const ValueEntry*
		get_value_child(const Glib::ustring& name) const;

		/** @brief Returns the value from the child with the given
		 * name.
		 *
		 * If there is no such child (or it is not a ValueEntry), the
		 * given default value is returned.
		 */
		template<typename DataType>
		DataType get_value(
			const Glib::ustring& name,
			const DataType& default_value = DataType(),
			const serialise::context_base_from<DataType>& ctx =
				serialise::default_context_from<DataType>()
		) const;

		/** @brief Returns the value from the child with the given
		 * name.
		 *
		 * If there is no such child (or it is not a ValueEntry), a
		 * new child will be created (replacing a potential old one)
		 * and assigned the given default value.
		 */
		template<typename DataType>
		DataType supply_value(
			const Glib::ustring& name,
			const DataType& default_value = DataType(),
			const serialise::context_base_from<DataType>& ctx_from =
				serialise::default_context_from<DataType>(),
			const serialise::context_base_to<DataType>& ctx_to =
				serialise::default_context_to<DataType>()
		);

		/** @brief Creates a new child ValueEntry with the given name
		 * and value.
		 *
		 * If there is already a child with this name, it will be
		 * removed.
		 */
		template<typename DataType>
		void set_value(
			const Glib::ustring& name,
			const DataType& value,
			const serialise::context_base_to<DataType>& ctx =
				serialise::default_context_to<DataType>()
		);

		/** @brief Returns the parent entry at name.
		 *
		 * If there is no parent node, a new one will be created
		 * that overwrites the current entry (if any).
		 */
		ParentEntry& operator[](const Glib::ustring& name);

		/** @brief Creates a new ParentEntry with the given name.
		 *
		 * If there is already a child with this name, it will be
		 * removed.
		 */
		ParentEntry& set_parent(const Glib::ustring& name);

		/** @brief Returns an iterator to the beginning of the
		 * child entry sequence.
		 */
		iterator begin();

		/** @brief Returns an iterator to the beginning of the
		 * child entry sequence.
		 */
		const_iterator begin() const;

		/** @brief Returns an iterator to the end of the child
		 * entry sequence.
		 */
		iterator end();

		/** @brief Returns an iterator to the end of the child
		 * entry sequence.
		 */
		const_iterator end() const;

	protected:
		map_type m_map;
	};

	Config(const Glib::ustring& file, const Glib::ustring& old_file);
	~Config();

	ParentEntry& get_root();
	const ParentEntry& get_root() const;

protected:
	Glib::ustring m_filename;
	std::auto_ptr<ParentEntry> m_root;
};

template<typename DataType>
Config::ValueEntry::
	ValueEntry(const Glib::ustring& name,
                   const DataType& value,
                   const serialise::context_base_to<DataType>& ctx):
	Entry(name), m_data(value, ctx)
{
}

template<typename DataType>
DataType Config::ValueEntry::
	get(const serialise::context_base_from<DataType>& from) const
{
	return m_data.::serialise::data::as<DataType>(from);
}

template<typename DataType>
Config::TypedValueEntry<DataType>::
	TypedValueEntry(const Glib::ustring& name,
	                const DataType& value,
                        const serialise::context_base_to<DataType>& ctx):
	ValueEntry(name, value, ctx)
{
}

template<typename DataType>
Config::TypedValueEntry<DataType>::TypedValueEntry(const xmlpp::Element& elem):
	ValueEntry(elem.get_name(), elem.get_child_text()->get_content() )
{
}

template<typename DataType>
void Config::TypedValueEntry<DataType>::save(xmlpp::Element& elem) const
{
	elem.set_child_text(m_data.serialised() );
}

template<typename BaseIterator, typename Entry>
Config::ParentEntry::iterator_base<BaseIterator, Entry>::
	iterator_base(const base_iterator& iter):
	m_iter(iter)
{
}

template<typename BaseIterator, typename Entry>
Config::ParentEntry::iterator_base<BaseIterator, Entry>&
Config::ParentEntry::iterator_base<BaseIterator, Entry>::operator++()
{
	++ m_iter;
	return *this;
}

template<typename BaseIterator, typename Entry>
Config::ParentEntry::iterator_base<BaseIterator, Entry>
Config::ParentEntry::iterator_base<BaseIterator, Entry>::operator++(int)
{
	iterator_base<BaseIterator, Entry> temp(*this);
	++ m_iter;
	return temp;
}

template<typename BaseIterator, typename Entry>
bool Config::ParentEntry::iterator_base<BaseIterator, Entry>::
	operator==(const iterator_base& other) const
{
	return m_iter == other.m_iter;
}

template<typename BaseIterator, typename Entry>
bool Config::ParentEntry::iterator_base<BaseIterator, Entry>::
	operator!=(const iterator_base& other) const
{
	return m_iter != other.m_iter;
}

template<typename BaseIterator, typename Entry>
Entry& Config::ParentEntry::iterator_base<BaseIterator, Entry>::
	operator*() const
{
	return *m_iter->second;
}

template<typename BaseIterator, typename Entry>
Entry* Config::ParentEntry::iterator_base<BaseIterator, Entry>::
	operator->() const
{
	return m_iter->second;
}

template<typename DataType>
DataType Config::ParentEntry::
	get_value(const Glib::ustring& name,
                  const DataType& default_value,
                  const serialise::context_base_from<DataType>& ctx) const
{
	const ValueEntry* entry = get_value_child(name);
	if(entry == NULL) return default_value;
	return entry->ValueEntry::get<DataType>(ctx);
}

template<typename DataType>
DataType Config::ParentEntry::
	supply_value(const Glib::ustring& name,
	             const DataType& default_value,
	             const serialise::context_base_from<DataType>& ctx_from,
	             const serialise::context_base_to<DataType>& ctx_to)
{
	ValueEntry* entry = get_value_child(name);
	if(entry != NULL) return entry->get(ctx_from);

	set_value(name, default_value, ctx_to);
	return default_value;
}

template<typename DataType>
void Config::ParentEntry::
	set_value(const Glib::ustring& name,
	          const DataType& value,
	          const serialise::context_base_to<DataType>& ctx)
{
	Entry* entry = get_child(name);
	if(entry != NULL) delete entry;

	m_map[name] = new TypedValueEntry<DataType>(name, value, ctx);
}

} // namespace Gobby

namespace serialise
{

/** @brief Used to convert Gdk::Color to a string.
 */
template<>
class default_context_to<Gdk::Color>: public context_base_to<Gdk::Color>
{
public:
	typedef Gdk::Color data_type;

	virtual std::string to_string(const data_type& from) const;
};

/** @brief Used to convert a string to Gdk::Color.
 */
template<>
class default_context_from<Gdk::Color>: public context_base_from<Gdk::Color>
{
public:
	typedef Gdk::Color data_type;

	virtual data_type from_string(const std::string& from) const;
};

template<>
class default_context_to<Glib::ustring>: public context_base_to<Glib::ustring>
{
public:
	typedef Glib::ustring data_type;

	virtual std::string to_string(const data_type& from) const;
};

template<>
class default_context_from<Glib::ustring>:
	public context_base_from<Glib::ustring>
{
public:
	typedef Glib::ustring data_type;

	virtual data_type from_string(const std::string& from) const;
};

} // namespace serialise

#endif // _GOBBY_CONFIG_HPP_
