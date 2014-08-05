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

#ifndef _GOBBY_CHATTABLABEL_HPP_
#define _GOBBY_CHATTABLABEL_HPP_

#include "core/chatsessionview.hpp"
#include "core/tablabel.hpp"

namespace Gobby
{

class ChatTabLabel: public TabLabel
{
public:
	ChatTabLabel(Folder& folder, ChatSessionView& view,
	             bool always_show_close_button);
	~ChatTabLabel();

protected:
	static void on_add_message_static(InfChatBuffer* buffer,
	                                  InfChatBufferMessage* message,
	                                  gpointer user_data)
	{
		static_cast<ChatTabLabel*>(user_data)->
			on_changed(message->user);
	}

	virtual void on_notify_subscription_group(); // override

	void on_changed(InfUser* author);

private:
	const bool m_always_show_close_button;
	gulong m_add_message_handle;
};

}

#endif // _GOBBY_CHATTABLABEL_HPP_
