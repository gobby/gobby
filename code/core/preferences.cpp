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

#include "features.hpp"
#include "core/preferences.hpp"
#include "util/file.hpp"

#include <libinfinity/common/inf-protocol.h>

#include <glibmm/miscutils.h>
#include <glibmm/fileutils.h>
#include <glibmm/random.h>

Gobby::Preferences::User::User(
	const Glib::RefPtr<Gio::Settings>& settings,
	Config::ParentEntry& entry)
:
	name(settings, entry, "name"),
	hue(settings, entry, "hue"),
	alpha(settings, entry, "alpha"),
	show_remote_cursors(settings, entry, "show-remote-cursors"),
	show_remote_selections(settings, entry, "show-remote-selections"),
	show_remote_current_lines(
		settings, entry, "show-remote-current-lines"),
	show_remote_cursor_positions(
		settings, entry, "show-remote-cursor-positions"),
	allow_remote_access(settings, entry, "allow-remote-access"),
	require_password(settings, entry, "require-password"),
	password(settings, entry, "password"),
	port(settings, entry, "port"),
	keep_local_documents(settings, entry, "keep-local-documents"),
	host_directory(settings, entry, "host-directory")
{
	if(name.is_default())
		name = Glib::get_user_name();
	if(hue.is_default())
		hue = Glib::Rand().get_double();
	if(host_directory.is_default())
		host_directory = config_filename("local-documents");
}

Gobby::Preferences::Editor::Editor(
	const Glib::RefPtr<Gio::Settings>& settings,
	Config::ParentEntry& entry)
:
	tab_width(settings, entry, "tab-width"),
	tab_spaces(settings, entry, "tab-insert-spaces"),
	indentation_auto(settings, entry, "auto-indentation"),
	homeend_smart(settings, entry, "smart-homeend"),
	autosave_enabled(settings, entry, "autosave-enabled"),
	autosave_interval(settings, entry, "autosave-interval")
{
}

Gobby::Preferences::View::View(
	const Glib::RefPtr<Gio::Settings>& settings,
	Config::ParentEntry& entry)
:
	wrap_mode(settings, entry, "wrap-mode"),
	linenum_display(settings, entry, "display-line-numbers"),
	curline_highlight(settings, entry, "highlight-current-line"),
	margin_display(settings, entry, "margin-display"),
	margin_pos(settings, entry, "margin-position"),
	bracket_highlight(settings, entry, "highlight-matching-brackets"),
	whitespace_display(settings, entry, "display-whitespace")
{
}

Gobby::Preferences::Appearance::Appearance(
	const Glib::RefPtr<Gio::Settings>& settings,
	Config::ParentEntry& entry)
:
	toolbar_style(settings, entry, "toolbar-style"),
	font(settings, entry, "font"),
	scheme_id(settings, entry, "scheme-id"),
	show_toolbar(settings, entry, "show-toolbar"),
	show_statusbar(settings, entry, "show-statusbar"),
	show_browser(settings, entry, "show-browser"),
	show_chat(settings, entry, "show-chat"),
	show_document_userlist(settings, entry, "show-document-userlist"),
	show_chat_userlist(settings, entry, "show-chat-userlist")
{
}

Gobby::Preferences::Security::Security(
	const Glib::RefPtr<Gio::Settings>& settings,
	Config::ParentEntry& entry)
:
	trust_file(settings, entry, "trust-file"),
	policy(settings, entry, "policy"),
	authentication_enabled(settings, entry, "authentication-enabled"),
	certificate_file(settings, entry, "certificate-file"),
	key_file(settings, entry, "key-file")
{
	// Load default trust-file. As this accesses the filesystem, only do
	// it when we really need it, i.e. when starting Gobby the first time.
	if(trust_file.is_default())
	{
#ifdef G_OS_WIN32
		gchar* package_directory =
			g_win32_get_package_installation_directory_of_module(
				NULL);

		trust_file = Glib::build_filename(
			Glib::build_filename(package_directory, "certs"),
			"ca-certificates.crt");

		g_free(package_directory);
#else
		// This seems to be the default location for both
		// Debian and Gentoo. I don't know about other distributions.
		// Maybe they need a distro-patch for this.
		const std::string DEFAULT_TRUST_FILE =
			"/etc/ssl/certs/ca-certificates.crt";
		if(Glib::file_test(DEFAULT_TRUST_FILE,
		                   Glib::FILE_TEST_IS_REGULAR))
		{
			trust_file = DEFAULT_TRUST_FILE;
		}
#endif
	}
}

Gobby::Preferences::Network::Network(
	const Glib::RefPtr<Gio::Settings>& settings,
	Config::ParentEntry& entry)
:
	keepalive(settings, entry, "keepalive")
{
}

Gobby::Preferences::Preferences(Config& config):
	m_settings(Gio::Settings::create("de.0x539.gobby.preferences")),
	user(m_settings->get_child("user"),
	     config.get_root()["user"]),
	editor(m_settings->get_child("editor"),
	       config.get_root()["editor"]),
	view(m_settings->get_child("view"),
	     config.get_root()["view"]),
	appearance(m_settings->get_child("appearance"),
	           config.get_root()["appearance"]),
	security(m_settings->get_child("security"),
	         config.get_root()["security"]),
	network(m_settings->get_child("network"),
	        config.get_root()["network"])
{
}

