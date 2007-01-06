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

#include "application_state.hpp"

Gobby::ApplicationState::ApplicationState(ApplicationFlags initial_flags):
	m_state(initial_flags)
{
}

void Gobby::ApplicationState::modify(ApplicationFlags inc_flags,
                                     ApplicationFlags exc_flags)
{
	(m_state |= inc_flags) &= ~exc_flags;
	m_signal_state_changed.emit(m_state);
}

bool Gobby::ApplicationState::query(ApplicationFlags inc_flags,
                                    ApplicationFlags exc_flags) const
{
	return ((m_state & inc_flags) == inc_flags) &&
	       ((m_state & exc_flags) == 0);
}

Gobby::ApplicationState::signal_state_changed_type
Gobby::ApplicationState::state_changed_event() const
{
	return m_signal_state_changed;
}
