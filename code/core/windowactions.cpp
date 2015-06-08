/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2015 Armin Burgmeier <armin@arbur.net>
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

#include "windowactions.hpp"

Gobby::WindowActions::WindowActions(Gio::ActionMap& map,
                                    const Preferences& preferences):
	new_document(map.add_action("new")),
	open(map.add_action("open")),
	open_location(map.add_action("open-location")),
	save(map.add_action("save")),
	save_as(map.add_action("save-as")),
	save_all(map.add_action("save-all")),
	export_html(map.add_action("export-html")),
	connect(map.add_action("connect")),
	close(map.add_action("close")),

	undo(map.add_action("undo")),
	redo(map.add_action("redo")),
	cut(map.add_action("cut")),
	copy(map.add_action("copy")),
	paste(map.add_action("paste")),
	find(map.add_action("find")),
	find_next(map.add_action("find-next")),
	find_prev(map.add_action("find-prev")),
	find_replace(map.add_action("find-replace")),
	goto_line(map.add_action("goto-line")),

	hide_user_colors(map.add_action("hide-user-colors")),
	fullscreen(map.add_action_bool("fullscreen", false)),
	zoom_in(map.add_action("zoom-in")),
	zoom_out(map.add_action("zoom-out")),
	view_toolbar(map.add_action_bool(
		"view-toolbar",
		static_cast<bool>(preferences.appearance.show_toolbar))),
	view_statusbar(map.add_action_bool(
		"view-statusbar",
		static_cast<bool>(preferences.appearance.show_statusbar))),
	view_browser(map.add_action_bool(
		"view-browser",
		static_cast<bool>(preferences.appearance.show_browser))),
	view_chat(map.add_action_bool(
		"view-chat",
		static_cast<bool>(preferences.appearance.show_chat))),
	view_document_userlist(map.add_action_bool(
		"view-document-userlist",
		static_cast<bool>(
			preferences.appearance.show_document_userlist))),
	view_chat_userlist(map.add_action_bool(
		"view-chat-userlist",
		static_cast<bool>(
			preferences.appearance.show_chat_userlist))),
	highlight_mode(map.add_action_radio_string("highlight-mode", ""))
{
}
