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

#ifndef _GOBBY_APPLICATION_STATE_HPP_
#define _GOBBY_APPLICATION_STATE_HPP_

#include <sigc++/signal.h>
#include <net6/enum_ops.hpp>

namespace Gobby
{

/** @brief Flags that describe the application's current state.
 */
enum ApplicationFlags {
	APPLICATION_NONE     = 0x00, // None of the below
	APPLICATION_INITIAL  = 0x01, // Application has just been initialised
	APPLICATION_SESSION  = 0x02, // Session has been opened
	APPLICATION_DOCUMENT = 0x04, // At least one doucment is opened
	APPLICATION_HOST     = 0x08, // Application is hosting the session
	APPLICATION_SELECTED = 0x10  // Text is selected
};

NET6_DEFINE_ENUM_OPS(ApplicationFlags)

/** @brief Class that notifies about application state changes.
 */
class ApplicationState
{
public:
	typedef sigc::signal<void, ApplicationState> signal_state_changed_type;

	ApplicationState(ApplicationFlags initial_flags);

	void modify(ApplicationFlags inc_flags,
	            ApplicationFlags exc_flags);

	bool query(ApplicationFlags inc_flags,
	           ApplicationFlags exc_flags) const;

	signal_state_changed_type state_changed_event() const;
private:
	signal_state_changed_type m_signal_state_changed;

	ApplicationFlags m_state;
};

} // namespace Gobby

#endif // _GOBBY_APPLICATION_STATE_HPP_
