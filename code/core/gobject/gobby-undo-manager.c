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

#include "core/gobject/gobby-undo-manager.h"

#include <gtksourceview/gtksource.h>

typedef struct _GobbyUndoManagerPrivate GobbyUndoManagerPrivate;
struct _GobbyUndoManagerPrivate {
  InfTextSession* session;
  InfTextUndoGrouping* undo_grouping;
};

enum {
  PROP_0,

  PROP_SESSION,
  PROP_UNDO_GROUPING
};

#define GOBBY_UNDO_MANAGER_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), GOBBY_TYPE_UNDO_MANAGER, GobbyUndoManagerPrivate))

static void gobby_undo_manager_undo_manager_iface_init(GtkSourceUndoManagerIface* iface);
G_DEFINE_TYPE_WITH_CODE(GobbyUndoManager, gobby_undo_manager, G_TYPE_OBJECT,
  G_ADD_PRIVATE(GobbyUndoManager)
  G_IMPLEMENT_INTERFACE(GTK_SOURCE_TYPE_UNDO_MANAGER, gobby_undo_manager_undo_manager_iface_init))

static void
gobby_undo_manager_can_undo_changed_cb(InfAdoptedAlgorithm* algorithm,
                                       InfAdoptedUser* user,
                                       gboolean can_undo,
                                       gpointer user_data)
{
  GobbyUndoManager* undo_manager;
  GobbyUndoManagerPrivate* priv;

  undo_manager = GOBBY_UNDO_MANAGER(user_data);
  priv = GOBBY_UNDO_MANAGER_PRIVATE(undo_manager);

  InfAdoptedUser* our_user;
  g_object_get(G_OBJECT(priv->undo_grouping), "user", &our_user, NULL);

  if(our_user == user)
  {
    gtk_source_undo_manager_can_undo_changed(
      GTK_SOURCE_UNDO_MANAGER(undo_manager)
    );
  }

  g_object_unref(our_user);
}

static void
gobby_undo_manager_can_redo_changed_cb(InfAdoptedAlgorithm* algorithm,
                                       InfAdoptedUser* user,
                                       gboolean can_redo,
                                       gpointer user_data)
{
  GobbyUndoManager* undo_manager;
  GobbyUndoManagerPrivate* priv;

  undo_manager = GOBBY_UNDO_MANAGER(user_data);
  priv = GOBBY_UNDO_MANAGER_PRIVATE(undo_manager);

  InfAdoptedUser* our_user;
  g_object_get(G_OBJECT(priv->undo_grouping), "user", &our_user, NULL);

  if(our_user == user)
  {
    gtk_source_undo_manager_can_redo_changed(
      GTK_SOURCE_UNDO_MANAGER(undo_manager)
    );
  }

  g_object_unref(our_user);

}

static void
gobby_undo_manager_set_session(GobbyUndoManager* undo_manager,
                               InfTextSession* session)
{
  GobbyUndoManagerPrivate* priv;
  InfAdoptedAlgorithm* algorithm;

  priv = GOBBY_UNDO_MANAGER_PRIVATE(undo_manager);

  if(priv->session != NULL)
  {
    algorithm = inf_adopted_session_get_algorithm(
      INF_ADOPTED_SESSION(priv->session)
    );

    inf_signal_handlers_disconnect_by_func(
      G_OBJECT(algorithm),
      G_CALLBACK(gobby_undo_manager_can_undo_changed_cb),
      undo_manager
    );

    inf_signal_handlers_disconnect_by_func(
      G_OBJECT(algorithm),
      G_CALLBACK(gobby_undo_manager_can_redo_changed_cb),
      undo_manager
    );

    g_object_unref(priv->session);
  }

  priv->session = session;

  if(session != NULL)
  {
    g_object_ref(session);

    algorithm = inf_adopted_session_get_algorithm(
      INF_ADOPTED_SESSION(session)
    );

    g_signal_connect(
      G_OBJECT(algorithm),
      "can-undo-changed",
      G_CALLBACK(gobby_undo_manager_can_undo_changed_cb),
      undo_manager
    );

    g_signal_connect(
      G_OBJECT(algorithm),
      "can-redo-changed",
      G_CALLBACK(gobby_undo_manager_can_redo_changed_cb),
      undo_manager
    );
  }
}

static void
gobby_undo_manager_init(GobbyUndoManager* undo_manager)
{
  GobbyUndoManagerPrivate* priv;
  priv = GOBBY_UNDO_MANAGER_PRIVATE(undo_manager);

  priv->session = NULL;
  priv->undo_grouping = NULL;
}

static void
gobby_undo_manager_dispose(GObject* object)
{
  GobbyUndoManager* undo_manager;
  GobbyUndoManagerPrivate* priv;

  undo_manager = GOBBY_UNDO_MANAGER(object);
  priv = GOBBY_UNDO_MANAGER_PRIVATE(undo_manager);

  gobby_undo_manager_set_session(undo_manager, NULL);

  if(priv->undo_grouping != NULL)
  {
    g_object_unref(priv->undo_grouping);
    priv->undo_grouping = NULL;
  }

  G_OBJECT_CLASS(gobby_undo_manager_parent_class)->dispose(object);
}

static void
gobby_undo_manager_finalize(GObject* object)
{
  GobbyUndoManager* undo_manager;
  GobbyUndoManagerPrivate* priv;

  undo_manager = GOBBY_UNDO_MANAGER(object);
  priv = GOBBY_UNDO_MANAGER_PRIVATE(undo_manager);

  G_OBJECT_CLASS(gobby_undo_manager_parent_class)->finalize(object);
}

static void
gobby_undo_manager_set_property(GObject* object,
                                guint prop_id,
                                const GValue* value,
                                GParamSpec* pspec)
{
  GobbyUndoManager* undo_manager;
  GobbyUndoManagerPrivate* priv;

  undo_manager = GOBBY_UNDO_MANAGER(object);
  priv = GOBBY_UNDO_MANAGER_PRIVATE(undo_manager);

  switch(prop_id)
  {
  case PROP_SESSION:
    gobby_undo_manager_set_session(
      undo_manager,
      INF_TEXT_SESSION(g_value_get_object(value))
    );

    break;
  case PROP_UNDO_GROUPING:
    if(priv->undo_grouping != NULL)
      g_object_unref(priv->undo_grouping);
    priv->undo_grouping = INF_TEXT_UNDO_GROUPING(g_value_dup_object(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    break;
  }
}

static void
gobby_undo_manager_get_property(GObject* object,
                                guint prop_id,
                                GValue* value,
                                GParamSpec* pspec)
{
  GobbyUndoManager* undo_manager;
  GobbyUndoManagerPrivate* priv;

  undo_manager = GOBBY_UNDO_MANAGER(object);
  priv = GOBBY_UNDO_MANAGER_PRIVATE(undo_manager);

  switch(prop_id)
  {
  case PROP_SESSION:
    g_value_set_object(value, priv->session);
    break;
  case PROP_UNDO_GROUPING:
    g_value_set_object(value, priv->undo_grouping);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    break;
  }
}

static void
gobby_undo_manager_class_init(GobbyUndoManagerClass* undo_manager_class)
{
  GObjectClass* object_class;
  object_class = G_OBJECT_CLASS(undo_manager_class);

  object_class->dispose = gobby_undo_manager_dispose;
  object_class->finalize = gobby_undo_manager_finalize;
  object_class->set_property = gobby_undo_manager_set_property;
  object_class->get_property = gobby_undo_manager_get_property;

  g_object_class_install_property(
    object_class,
    PROP_SESSION,
    g_param_spec_object(
      "session",
      "Session",
      "The session on which to operate",
      INF_TEXT_TYPE_SESSION,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY
    )
  );

  g_object_class_install_property(
    object_class,
    PROP_UNDO_GROUPING,
    g_param_spec_object(
      "undo-grouping",
      "Undo Grouping",
      "The undo grouping to use",
      INF_TEXT_TYPE_UNDO_GROUPING,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY
    )
  );
}

static gboolean
gobby_undo_manager_can_undo(GtkSourceUndoManager* manager)
{
  GobbyUndoManager* undo_manager;
  GobbyUndoManagerPrivate* priv;
  InfAdoptedAlgorithm* algorithm;
  InfAdoptedUser* user;
  gboolean can_undo;

  undo_manager = GOBBY_UNDO_MANAGER(manager);
  priv = GOBBY_UNDO_MANAGER_PRIVATE(undo_manager);
  algorithm =
    inf_adopted_session_get_algorithm(INF_ADOPTED_SESSION(priv->session));
  g_object_get(G_OBJECT(priv->undo_grouping), "user", &user, NULL);

  can_undo = inf_adopted_algorithm_can_undo(algorithm, user);
  g_object_unref(user);

  return can_undo;
}

static gboolean
gobby_undo_manager_can_redo(GtkSourceUndoManager* manager)
{
  GobbyUndoManager* undo_manager;
  GobbyUndoManagerPrivate* priv;
  InfAdoptedAlgorithm* algorithm;
  InfAdoptedUser* user;
  gboolean can_redo;

  undo_manager = GOBBY_UNDO_MANAGER(manager);
  priv = GOBBY_UNDO_MANAGER_PRIVATE(undo_manager);
  algorithm =
    inf_adopted_session_get_algorithm(INF_ADOPTED_SESSION(priv->session));
  g_object_get(G_OBJECT(priv->undo_grouping), "user", &user, NULL);

  can_redo = inf_adopted_algorithm_can_redo(algorithm, user);
  g_object_unref(user);

  return can_redo;
}

static void
gobby_undo_manager_undo(GtkSourceUndoManager* manager)
{
  GobbyUndoManager* undo_manager;
  GobbyUndoManagerPrivate* priv;
  InfAdoptedUser* user;
  guint n_undo;

  undo_manager = GOBBY_UNDO_MANAGER(manager);
  priv = GOBBY_UNDO_MANAGER_PRIVATE(undo_manager);

  g_object_get(G_OBJECT(priv->undo_grouping), "user", &user, NULL);

  n_undo = inf_adopted_undo_grouping_get_undo_size(
    INF_ADOPTED_UNDO_GROUPING(priv->undo_grouping)
  );

  g_assert(n_undo > 0);
  inf_adopted_session_undo(INF_ADOPTED_SESSION(priv->session), user, n_undo);
  g_object_unref(user);
}

static void
gobby_undo_manager_redo(GtkSourceUndoManager* manager)
{
  GobbyUndoManager* undo_manager;
  GobbyUndoManagerPrivate* priv;
  InfAdoptedUser* user;
  guint n_redo;

  undo_manager = GOBBY_UNDO_MANAGER(manager);
  priv = GOBBY_UNDO_MANAGER_PRIVATE(undo_manager);

  g_object_get(G_OBJECT(priv->undo_grouping), "user", &user, NULL);

  n_redo = inf_adopted_undo_grouping_get_redo_size(
    INF_ADOPTED_UNDO_GROUPING(priv->undo_grouping)
  );

  g_assert(n_redo > 0);
  inf_adopted_session_redo(INF_ADOPTED_SESSION(priv->session), user, n_redo);
  g_object_unref(user);

}

static void
gobby_undo_manager_begin_not_undoable_action(GtkSourceUndoManager* manager)
{
  /* Unused */
}

static void
gobby_undo_manager_end_not_undoable_action(GtkSourceUndoManager* manager)
{
  /* Unused */
}

static void
gobby_undo_manager_undo_manager_iface_init(GtkSourceUndoManagerIface* iface)
{
  iface->can_undo = gobby_undo_manager_can_undo;
  iface->can_redo = gobby_undo_manager_can_redo;
  iface->undo = gobby_undo_manager_undo;
  iface->redo = gobby_undo_manager_redo;
  iface->begin_not_undoable_action =
    gobby_undo_manager_begin_not_undoable_action;
  iface->end_not_undoable_action =
    gobby_undo_manager_end_not_undoable_action;
}

GobbyUndoManager*
gobby_undo_manager_new(InfTextSession* session,
                       InfTextUndoGrouping* undo_grouping)
{
  GObject* object;

  object = g_object_new(
    GOBBY_TYPE_UNDO_MANAGER,
    "session", session,
    "undo-grouping", undo_grouping,
    NULL
  );

  return GOBBY_UNDO_MANAGER(object);
}
