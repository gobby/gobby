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
