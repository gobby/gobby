/* gobby - A GTKmm driven libobby client
 * Copyright (C) 2005 0x539 dev group
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

#include "docwindow.hpp"

Gobby::DocWindow::DocWindow(obby::local_document_info& doc,
                            const Folder& folder)
 : Gtk::ScrolledWindow(), m_doc(doc, folder)
{
	set_shadow_type(Gtk::SHADOW_IN);
	set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	add(m_doc);
}

Gobby::DocWindow::~DocWindow()
{
}

const Gobby::Document& Gobby::DocWindow::get_document() const
{
	return m_doc;
}

Gobby::Document& Gobby::DocWindow::get_document()
{
	return m_doc;
}

void Gobby::DocWindow::obby_user_join(obby::user& user)
{
	m_doc.obby_user_join(user);
}

void Gobby::DocWindow::obby_user_part(obby::user& user)
{
	m_doc.obby_user_part(user);
}
