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

#ifndef _GOBBY_DEFAULT_ACCUMULATOR_HPP_
#define _GOBBY_DEFAULT_ACCUMULATOR_HPP_

namespace Gobby
{

/** Accumulator for signals with return type that defaults to a value if no
 * signal handler is connected.
 */

template<typename return_type, return_type default_return>
class default_accumulator {
public:
	typedef return_type result_type;

	template<typename iterator>
	result_type operator()(iterator begin, iterator end) const {
		return_type result = default_return;
		for(; begin != end; ++ begin)
			result = *begin;
		return result;
	}
};

} // namespace Gobby

#endif // _GOBBY_DEFAULT_ACCUMULATOR_HPP_
