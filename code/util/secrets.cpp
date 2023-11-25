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

#include "features.hpp"
#include "secrets.hpp"

#ifdef HAVE_LIBSECRET
# include <libsecret/secret.h>
#endif

#include <iostream>
#include <memory>
#include <string>

namespace Gobby {

namespace {

#ifdef HAVE_LIBSECRET
const SecretSchema* get_schema()
{
	static const SecretSchema secret_schema = {
		"de.0x539.gobby.Password", SECRET_SCHEMA_NONE,
		{
			{ "server", SECRET_SCHEMA_ATTRIBUTE_STRING },
			{ "username", SECRET_SCHEMA_ATTRIBUTE_STRING },
			{ "NULL", SECRET_SCHEMA_ATTRIBUTE_STRING },
		}
	};
	return &secret_schema;
}

extern "C"
{

void on_password_stored(GObject* source, GAsyncResult* result, gpointer unused)
{
	GError* error = nullptr;
	secret_password_store_finish(result, &error);
	if (error != nullptr)
	{
		std::cerr << "Failed to store secret: " << error->message << std::endl;
		g_error_free(error);
	}
}

void on_password_lookup(GObject* source, GAsyncResult* result, gpointer user_data)
{
	std::unique_ptr<LookupCallback> password_callback{reinterpret_cast<LookupCallback*>(user_data)};
	GError *error = nullptr;
	gchar* password = secret_password_lookup_finish(result, &error);
	if (error != nullptr)
	{
		std::cerr << "Failed to lookup secret: " << error->message << std::endl;
		g_error_free(error);
		(*password_callback)(std::nullopt);
	}
	else if (password == nullptr)
	{
		(*password_callback)(std::nullopt);
	}
	else
	{
		(*password_callback)(password);
		secret_password_free(password);
	}
}

} // extern "C"
#endif

} // namespace

void store_secret(const std::string& server, const std::string& username, const std::string& password)
{
#ifdef HAVE_LIBSECRET
	const std::string label = "Gobby password for \"" + username + "\" on \"" + server + "\"";
	secret_password_store(get_schema(),
			SECRET_COLLECTION_DEFAULT,
			label.c_str(),
			password.c_str(),
			/*cancellable=*/NULL,
			on_password_stored,
			/*user_data=*/NULL,
			"server", server.c_str(),
			"username", username.c_str(),
			NULL);
#endif
}

void lookup_secret(const std::string& server, const std::string& username, LookupCallback password_callback)
{
#ifdef HAVE_LIBSECRET
	auto callback = std::make_unique<LookupCallback>(password_callback);
	secret_password_lookup(get_schema(),
		/*cancellable=*/NULL,
		on_password_lookup,
		/*user_data=*/static_cast<void*>(callback.release()),
		"server", server.c_str(),
		"username", username.c_str(),
		NULL);
#else
	password_callback(std::nullopt);
#endif
}

} // namespace Gobby
