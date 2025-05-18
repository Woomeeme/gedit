/* SPDX-FileCopyrightText: 2024 - Sébastien Wilmet
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <libpeas/peas.h>
#include <gedit/gedit-view.h>

G_BEGIN_DECLS

#define GEDIT_TYPE_TEXT_SIZE_VIEW_ACTIVATABLE             (gedit_text_size_view_activatable_get_type ())
#define GEDIT_TEXT_SIZE_VIEW_ACTIVATABLE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GEDIT_TYPE_TEXT_SIZE_VIEW_ACTIVATABLE, GeditTextSizeViewActivatable))
#define GEDIT_TEXT_SIZE_VIEW_ACTIVATABLE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GEDIT_TYPE_TEXT_SIZE_VIEW_ACTIVATABLE, GeditTextSizeViewActivatableClass))
#define GEDIT_IS_TEXT_SIZE_VIEW_ACTIVATABLE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GEDIT_TYPE_TEXT_SIZE_VIEW_ACTIVATABLE))
#define GEDIT_IS_TEXT_SIZE_VIEW_ACTIVATABLE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GEDIT_TYPE_TEXT_SIZE_VIEW_ACTIVATABLE))
#define GEDIT_TEXT_SIZE_VIEW_ACTIVATABLE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GEDIT_TYPE_TEXT_SIZE_VIEW_ACTIVATABLE, GeditTextSizeViewActivatableClass))

typedef struct _GeditTextSizeViewActivatable         GeditTextSizeViewActivatable;
typedef struct _GeditTextSizeViewActivatableClass    GeditTextSizeViewActivatableClass;
typedef struct _GeditTextSizeViewActivatablePrivate  GeditTextSizeViewActivatablePrivate;

struct _GeditTextSizeViewActivatable
{
	GObject parent;

	GeditTextSizeViewActivatablePrivate *priv;
};

struct _GeditTextSizeViewActivatableClass
{
	GObjectClass parent_class;
};

GType	gedit_text_size_view_activatable_get_type		(void);

void	gedit_text_size_view_activatable_register		(PeasObjectModule *module);

GeditTextSizeViewActivatable *
	gedit_text_size_view_activatable_get_from_view		(GeditView *view);

void	gedit_text_size_view_activatable_make_larger		(GeditTextSizeViewActivatable *self);

void	gedit_text_size_view_activatable_make_smaller		(GeditTextSizeViewActivatable *self);

void	gedit_text_size_view_activatable_reset_to_normal_size	(GeditTextSizeViewActivatable *self);

G_END_DECLS
