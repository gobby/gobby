/* gobby - A GTKmm driven libobby client
 * Copyright (C) 2005 - 2008 0x539 dev group
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

#include "operations/operation-new.hpp"
#include "operations/operations.hpp"

#include "core/noteplugin.hpp"
#include "util/i18n.hpp"

Gobby::Operations::Operation::~Operation() {}

Gobby::Operations::Operations(StatusBar& status_bar):
	m_status_bar(status_bar)
{
}

Gobby::Operations::~Operations()
{
	for(OperationSet::iterator iter = m_operations.begin();
	    iter != m_operations.end(); ++ iter)
	{
		delete *iter;
	}
}

void Gobby::Operations::create_document(InfcBrowser* browser,
                                        InfcBrowserIter* parent,
                                        const Glib::ustring name)
{
	m_operations.insert(new OperationNew(*this, browser, parent, name));
}

void Gobby::Operations::create_document(InfcBrowser* browser,
                                        InfcBrowserIter* parent,
                                        const Glib::ustring name,
                                        const Glib::ustring& from_uri)
{
}

void Gobby::Operations::remove_operation(Operation* operation)
{
	m_operations.erase(operation);
	delete operation;
}
