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

#include "util/asyncoperation.hpp"

#include <glibmm/main.h>

#include <cassert>
#include <cstdio>

Gobby::AsyncOperation::Handle::Handle(AsyncOperation& operation):
	m_operation(&operation)
{
}

Gobby::AsyncOperation::Handle::~Handle()
{
	if(m_operation)
	{
		if(!m_operation->m_finished)
			cancel();

		m_operation->m_handle = NULL;
	}
}

void Gobby::AsyncOperation::Handle::cancel()
{
	assert(m_operation);
	assert(m_operation->m_finished == false);

	m_operation->m_finished = true;
}

Gobby::AsyncOperation::AsyncOperation():
	m_thread(NULL), m_handle(NULL), m_finished(false)
{
}

Gobby::AsyncOperation::~AsyncOperation()
{
	if(m_handle)
		m_handle->m_operation = NULL;
}

std::unique_ptr<Gobby::AsyncOperation::Handle> 
Gobby::AsyncOperation::start(std::unique_ptr<AsyncOperation> operation)
{
	assert(operation->m_thread == NULL);
	assert(operation->m_handle == NULL);
	assert(operation->m_finished == false);

	AsyncOperation* op = operation.release();

	std::unique_ptr<Handle> handle(new Handle(*op));
	op->m_handle = handle.get();

	op->m_thread = Glib::Thread::create(
		sigc::mem_fun(*op, &AsyncOperation::thread_run), true);

	return handle;
}

void Gobby::AsyncOperation::thread_run()
{
	run();

	Glib::signal_idle().connect(
		sigc::mem_fun(*this, &AsyncOperation::done));
}

bool Gobby::AsyncOperation::done()
{
	if(!m_finished)
	{
		// m_handle DTOR cancels the operation
		g_assert(m_handle != NULL);

		m_finished = true;
		finish();
	}

	if(m_handle)
		m_handle->m_operation = NULL;
	delete this;

	return false;
}
