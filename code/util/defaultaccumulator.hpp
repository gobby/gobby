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
