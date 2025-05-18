/* SPDX-FileCopyrightText: 2024 - Sébastien Wilmet
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <libpeas/peas.h>

G_BEGIN_DECLS

#define GEDIT_TYPE_TEXT_SIZE_APP_ACTIVATABLE             (gedit_text_size_app_activatable_get_type ())
#define GEDIT_TEXT_SIZE_APP_ACTIVATABLE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GEDIT_TYPE_TEXT_SIZE_APP_ACTIVATABLE, GeditTextSizeAppActivatable))
#define GEDIT_TEXT_SIZE_APP_ACTIVATABLE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GEDIT_TYPE_TEXT_SIZE_APP_ACTIVATABLE, GeditTextSizeAppActivatableClass))
#define GEDIT_IS_TEXT_SIZE_APP_ACTIVATABLE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GEDIT_TYPE_TEXT_SIZE_APP_ACTIVATABLE))
#define GEDIT_IS_TEXT_SIZE_APP_ACTIVATABLE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GEDIT_TYPE_TEXT_SIZE_APP_ACTIVATABLE))
#define GEDIT_TEXT_SIZE_APP_ACTIVATABLE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GEDIT_TYPE_TEXT_SIZE_APP_ACTIVATABLE, GeditTextSizeAppActivatableClass))

typedef struct _GeditTextSizeAppActivatable         GeditTextSizeAppActivatable;
typedef struct _GeditTextSizeAppActivatableClass    GeditTextSizeAppActivatableClass;
typedef struct _GeditTextSizeAppActivatablePrivate  GeditTextSizeAppActivatablePrivate;

struct _GeditTextSizeAppActivatable
{
	GObject parent;

	GeditTextSizeAppActivatablePrivate *priv;
};

struct _GeditTextSizeAppActivatableClass
{
	GObjectClass parent_class;
};

GType	gedit_text_size_app_activatable_get_type	(void);

void	gedit_text_size_app_activatable_register	(PeasObjectModule *module);

G_END_DECLS
