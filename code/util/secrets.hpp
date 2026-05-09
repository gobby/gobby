/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2024 Philipp Kern
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

#ifndef _GOBBY_UTIL_SECRETS_HPP_
#define _GOBBY_UTIL_SECRETS_HPP_

#include <functional>
#include <optional>
#include <string>

namespace Gobby
{

using LookupCallback = std::function<void(std::optional<std::string> password)>;

// lookup_secret asynchronously attempts to lookup the password for the given
// server and username combination in the keyring. Once the lookup attempt
// completes, the callback is called.
void lookup_secret(const std::string& server, const std::string& username, LookupCallback password_callback);

// store_secret asynchronously attempts to store the password for the given server
// and username in the keyring.
void store_secret(const std::string& server, const std::string& username, const std::string& password);

} // namespace Gobby

#endif // _GOBBY_UTIL_SECRETS_HPP_
