/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2013 Armin Burgmeier <armin@arbur.net>
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
 * Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02110-1301, USA.
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
	ChatTabLabel(Folder& folder, ChatSessionView& view);
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
	gulong m_add_message_handle;
};

}

#endif // _GOBBY_CHATTABLABEL_HPP_
