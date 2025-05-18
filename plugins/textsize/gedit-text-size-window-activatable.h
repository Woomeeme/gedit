/* SPDX-FileCopyrightText: 2024 - Sébastien Wilmet
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <libpeas/peas.h>

G_BEGIN_DECLS

#define GEDIT_TYPE_TEXT_SIZE_WINDOW_ACTIVATABLE             (gedit_text_size_window_activatable_get_type ())
#define GEDIT_TEXT_SIZE_WINDOW_ACTIVATABLE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GEDIT_TYPE_TEXT_SIZE_WINDOW_ACTIVATABLE, GeditTextSizeWindowActivatable))
#define GEDIT_TEXT_SIZE_WINDOW_ACTIVATABLE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GEDIT_TYPE_TEXT_SIZE_WINDOW_ACTIVATABLE, GeditTextSizeWindowActivatableClass))
#define GEDIT_IS_TEXT_SIZE_WINDOW_ACTIVATABLE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GEDIT_TYPE_TEXT_SIZE_WINDOW_ACTIVATABLE))
#define GEDIT_IS_TEXT_SIZE_WINDOW_ACTIVATABLE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GEDIT_TYPE_TEXT_SIZE_WINDOW_ACTIVATABLE))
#define GEDIT_TEXT_SIZE_WINDOW_ACTIVATABLE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GEDIT_TYPE_TEXT_SIZE_WINDOW_ACTIVATABLE, GeditTextSizeWindowActivatableClass))

typedef struct _GeditTextSizeWindowActivatable         GeditTextSizeWindowActivatable;
typedef struct _GeditTextSizeWindowActivatableClass    GeditTextSizeWindowActivatableClass;
typedef struct _GeditTextSizeWindowActivatablePrivate  GeditTextSizeWindowActivatablePrivate;

struct _GeditTextSizeWindowActivatable
{
	GObject parent;

	GeditTextSizeWindowActivatablePrivate *priv;
};

struct _GeditTextSizeWindowActivatableClass
{
	GObjectClass parent_class;
};

GType	gedit_text_size_window_activatable_get_type	(void);

void	gedit_text_size_window_activatable_register	(PeasObjectModule *module);

G_END_DECLS
