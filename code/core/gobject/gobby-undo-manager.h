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

#ifndef __GOBBY_UNDO_MANAGER_H__
#define __GOBBY_UNDO_MANAGER_H__

#include <libinftext/inf-text-session.h>
#include <libinftext/inf-text-undo-grouping.h>

#include <glib-object.h>

G_BEGIN_DECLS

#define GOBBY_TYPE_UNDO_MANAGER                 (gobby_undo_manager_get_type())
#define GOBBY_UNDO_MANAGER(obj)                 (G_TYPE_CHECK_INSTANCE_CAST((obj), GOBBY_TYPE_UNDO_MANAGER, GobbyUndoManager))
#define GOBBY_UNDO_MANAGER_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST((klass), GOBBY_TYPE_UNDO_MANAGER, GobbyUndoManagerClass))
#define GOBBY_IS_UNDO_MANAGER(obj)              (G_TYPE_CHECK_INSTANCE_TYPE((obj), GOBBY_TYPE_UNDO_MANAGER))
#define GOBBY_IS_UNDO_MANAGER_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE((klass), GOBBY_TYPE_UNDO_MANAGER))
#define GOBBY_UNDO_MANAGER_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS((obj), GOBBY_TYPE_UNDO_MANAGER, GobbyUndoManagerClass))

typedef struct _GobbyUndoManager GobbyUndoManager;
typedef struct _GobbyUndoManagerClass GobbyUndoManagerClass;

/**
 * GobbyUndoManagerClass:
 *
 * This structure does not contain any public fields.
 */
struct _GobbyUndoManagerClass {
  /*< private >*/
  GObjectClass parent_class;
};

/**
 * GobbyUndoManager:
 *
 * #GobbyUndoManager is an opaque data type. You should only access it via the
 * public API functions.
 */
struct _GobbyUndoManager {
  /*< private >*/
  GObject parent;
};

GType
gobby_undo_manager_get_type(void) G_GNUC_CONST;

GobbyUndoManager*
gobby_undo_manager_new(InfTextSession* session,
                       InfTextUndoGrouping* grouping);

G_END_DECLS

#endif /* __GOBBY_UNDO_MANAGER_H__ */

/* vim:set et sw=2 ts=2: */
