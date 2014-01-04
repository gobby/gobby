/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2014 Armin Burgmeier <armin@arbur.net>
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

#ifndef _GOBBY_ASYNC_OPERATION_HPP_
#define _GOBBY_ASYNC_OPERATION_HPP_

#include <glibmm/thread.h>

#include <memory>

namespace Gobby
{

class AsyncOperation
{
public:
	class Handle
	{
		friend class AsyncOperation;
	public:
		Handle(AsyncOperation& operation);
		~Handle();

		void cancel();
	private:
		AsyncOperation* m_operation;
	};

	AsyncOperation();
	virtual ~AsyncOperation();

	static std::auto_ptr<Handle>
	start(std::auto_ptr<AsyncOperation> operation);

protected:
	virtual void run() = 0;
	virtual void finish() = 0;

	const Handle* get_handle() const { return m_handle; }

private:
	void thread_run();
	bool done();

	Glib::Thread* m_thread;
	Handle* m_handle;
	bool m_finished;
};

}
	
#endif // _GOBBY_ASYNC_OPERATION_HPP_
