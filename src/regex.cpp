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

// Contributed by Benjamin Herr, <ben@0x539.de>. Not used for now.

#include <glib/gunicode.h>
#include <regex.h>
#include "regex.hpp"

const regex::compile_options regex::compile_options::NONE
	= regex::compile_options(0);
const regex::compile_options regex::compile_options::EXTENDED
	= regex::compile_options(REG_EXTENDED);
const regex::compile_options regex::compile_options::IGNORE_CASE
	= regex::compile_options(REG_ICASE);
const regex::compile_options regex::compile_options::NO_OFFSETS
	= regex::compile_options(REG_NOSUB);
const regex::compile_options regex::compile_options::NEWLINE
	= regex::compile_options(REG_NEWLINE);

const regex::match_options regex::match_options::NONE
	= regex::match_options(0);
const regex::match_options regex::match_options::NOT_BOL
	= regex::match_options(REG_NOTBOL);
const regex::match_options regex::match_options::NOT_EOL
	= regex::match_options(REG_NOTEOL);

/* It would be fun if these were actually useful,
 * but I really do not know what to do with them.
const regex::compile_error regex::compile_error::BAD_BACKREF
	= regex::compile_error(REG_BADBR);
const regex::compile_error regex::compile_error::BAD_PATTERN
	= regex::compile_error(REG_BADPAT);
const regex::compile_error regex::compile_error::BAD_REPEAT
	= regex::compile_error(REG_BADRPT);
const regex::compile_error regex::compile_error::UNMATCHED_BRACE
	= regex::compile_error(REG_EBRACE);
const regex::compile_error regex::compile_error::UNMATCHED_BRACKET
	= regex::compile_error(REG_EBRACKET);
const regex::compile_error regex::compile_error::INVALID_COLLATION
	= regex::compile_error(REG_ECOLLATE);
const regex::compile_error regex::compile_error::BAD_CHARCLASS
	= regex::compile_error(REG_ECTYPE);
const regex::compile_error regex::compile_error::NON_SPECIFIC
	= regex::compile_error(REG_EEND);
const regex::compile_error regex::compile_error::TRAILING_BACKSLASH
	= regex::compile_error(REG_EESCAPE);
const regex::compile_error regex::compile_error::UNMATCHED_PAREN
	= regex::compile_error(REG_EPAREN);
const regex::compile_error regex::compile_error::INVALID_RANGE
	= regex::compile_error(REG_ERANGE);
const regex::compile_error regex::compile_error::BUFFER_TOO_BIG
	= regex::compile_error(REG_ESIZE);
const regex::compile_error regex::compile_error::OUT_OF_MEMORY
	= regex::compile_error(REG_ESPACE);
const regex::compile_error regex::compile_error::INVALID_SUBEXP_REF
	= regex::compile_error(REG_ESUBREG);

*/

#if 0
namespace
{
	std::string make_message(void* pregex, int value)
	{
		const regex_t * const regex = static_cast<regex_t*>(pregex);

		std::size_t bufsize = regerror(value, regex, 0, 0);
		char* buf = new char[bufsize];

		regerror(value, regex, buf, bufsize);
		std::string result = buf;

		delete[] buf;
		return result;
	}
}

regex::compile_error::compile_error(void* regex, int value):
	runtime_error(make_message(regex, value) )
{
}

regex::regex(const char* regex_string, compile_options cflags) {
	m_regex = new regex_t;
	int errcode = regcomp(static_cast<regex_t*>(m_regex), regex_string, cflags.get_value());
	if (errcode != 0) {
		this->~regex();
		throw compile_error(m_regex, errcode);
	}
}

regex::~regex() {
	regfree(static_cast<regex_t*>(m_regex) );
	delete static_cast<regex_t*>(m_regex);
}

void regex::reset(const char* regex_string, compile_options cflags) {
	regfree(static_cast<regex_t*>(m_regex) );
	int errcode = regcomp(static_cast<regex_t*>(m_regex), regex_string, cflags.get_value());
	if (errcode != 0) {
		throw compile_error(m_regex, errcode);
	}
}

bool regex::match(const char* string, match_options eflags) {
	return regexec(static_cast<regex_t*>(m_regex), string, 0, 0, eflags) == 0;
}

bool regex::find(const char* string,
                 match_positions matches,
                 match_options eflags) {
	std::vector<regmatch_t> pmatch(matches.size());
	if (regexec(static_cast<regex_t*>(m_regex), string, matches.size(), &pmatch[0], eflags) != 0)
		return false;

	std::vector<regmatch_t>::iterator j = pmatch.begin();
	for (match_positions::iterator i = matches.begin();
	     i != matches.end();
	     ++i, ++j) {
		i->first =
			g_utf8_pointer_to_offset(string, string + j->rm_so);
		i->second =
			g_utf8_pointer_to_offset(string, string + j->rm_eo);
	}

	return true;
}

bool regex::find(const char* string, std::pair<size_t, size_t>& matchpos,
          regex::match_options eflags) {
	regmatch_t pmatch;
	if (regexec(static_cast<regex_t*>(m_regex), string, 1, &pmatch, eflags) != 0)
		return false;

	matchpos.first =
		g_utf8_pointer_to_offset(string, string + pmatch.rm_so);
	matchpos.second =
		g_utf8_pointer_to_offset(string, string + pmatch.rm_eo);

	return true;
}
#endif

// Simple stubs to not to depend on the GNU regex library
regex::compile_error::compile_error(void* regex, int value):
	std::runtime_error("You will never see this")
{
	throw std::logic_error("regex::compile_error::compile_error");
}

regex::regex(const char* regex_string, compile_options cflags)
{
}

regex::~regex()
{
}

void regex::reset(const char* regex_string, compile_options cflags)
{
	throw std::logic_error("regex::reset");
}

bool regex::match(const char* string, match_options eflags)
{
	throw std::logic_error("regex::match");
	return false;
}

bool regex::find(const char* string,
                 match_positions matches,
                 match_options eflags)
{
	throw std::logic_error("regex::find");
	return false;
}

bool regex::find(const char* string, std::pair<size_t, size_t>& matchpos,
          regex::match_options eflags)
{
	throw std::logic_error("regex::find");
	return false;
}
