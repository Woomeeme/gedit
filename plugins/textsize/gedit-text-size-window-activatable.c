/* SPDX-FileCopyrightText: 2024 - Sébastien Wilmet
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "gedit-text-size-window-activatable.h"
#include <gedit/gedit-window.h>
#include <gedit/gedit-window-activatable.h>
#include "gedit-text-size-view-activatable.h"

struct _GeditTextSizeWindowActivatablePrivate
{
	GeditWindow *window;
};

enum
{
	PROP_0,
	PROP_WINDOW
};

static void gedit_window_activatable_iface_init (GeditWindowActivatableInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (GeditTextSizeWindowActivatable,
				gedit_text_size_window_activatable,
				G_TYPE_OBJECT,
				0,
				G_ADD_PRIVATE_DYNAMIC (GeditTextSizeWindowActivatable)
				G_IMPLEMENT_INTERFACE_DYNAMIC (GEDIT_TYPE_WINDOW_ACTIVATABLE,
							       gedit_window_activatable_iface_init))

static void
gedit_text_size_window_activatable_get_property (GObject    *object,
						 guint       prop_id,
						 GValue     *value,
						 GParamSpec *pspec)
{
	GeditTextSizeWindowActivatable *activatable = GEDIT_TEXT_SIZE_WINDOW_ACTIVATABLE (object);

	switch (prop_id)
	{
		case PROP_WINDOW:
			g_value_set_object (value, activatable->priv->window);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gedit_text_size_window_activatable_set_property (GObject      *object,
						 guint         prop_id,
						 const GValue *value,
						 GParamSpec   *pspec)
{
	GeditTextSizeWindowActivatable *activatable = GEDIT_TEXT_SIZE_WINDOW_ACTIVATABLE (object);

	switch (prop_id)
	{
		case PROP_WINDOW:
			g_assert (activatable->priv->window == NULL);
			activatable->priv->window = GEDIT_WINDOW (g_value_dup_object (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gedit_text_size_window_activatable_dispose (GObject *object)
{
	GeditTextSizeWindowActivatable *activatable = GEDIT_TEXT_SIZE_WINDOW_ACTIVATABLE (object);

	g_clear_object (&activatable->priv->window);

	G_OBJECT_CLASS (gedit_text_size_window_activatable_parent_class)->dispose (object);
}

static void
gedit_text_size_window_activatable_finalize (GObject *object)
{

	G_OBJECT_CLASS (gedit_text_size_window_activatable_parent_class)->finalize (object);
}

static void
gedit_text_size_window_activatable_class_init (GeditTextSizeWindowActivatableClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = gedit_text_size_window_activatable_get_property;
	object_class->set_property = gedit_text_size_window_activatable_set_property;
	object_class->dispose = gedit_text_size_window_activatable_dispose;
	object_class->finalize = gedit_text_size_window_activatable_finalize;

	g_object_class_override_property (object_class, PROP_WINDOW, "window");
}

static void
gedit_text_size_window_activatable_class_finalize (GeditTextSizeWindowActivatableClass *klass)
{
}

static void
gedit_text_size_window_activatable_init (GeditTextSizeWindowActivatable *activatable)
{
	activatable->priv = gedit_text_size_window_activatable_get_instance_private (activatable);
}

static GeditTextSizeViewActivatable *
get_active_view_activatable (GeditTextSizeWindowActivatable *self)
{
	GeditView *active_view;

	active_view = gedit_window_get_active_view (self->priv->window);
	if (active_view == NULL)
	{
		return NULL;
	}

	return gedit_text_size_view_activatable_get_from_view (active_view);
}

static void
text_larger_activate_cb (GSimpleAction *action,
			 GVariant      *parameter,
			 gpointer       user_data)
{
	GeditTextSizeWindowActivatable *self = GEDIT_TEXT_SIZE_WINDOW_ACTIVATABLE (user_data);
	GeditTextSizeViewActivatable *active_view_activatable;

	active_view_activatable = get_active_view_activatable (self);
	if (active_view_activatable != NULL)
	{
		gedit_text_size_view_activatable_make_larger (active_view_activatable);
	}
}

static void
text_smaller_activate_cb (GSimpleAction *action,
			  GVariant      *parameter,
			  gpointer       user_data)
{
	GeditTextSizeWindowActivatable *self = GEDIT_TEXT_SIZE_WINDOW_ACTIVATABLE (user_data);
	GeditTextSizeViewActivatable *active_view_activatable;

	active_view_activatable = get_active_view_activatable (self);
	if (active_view_activatable != NULL)
	{
		gedit_text_size_view_activatable_make_smaller (active_view_activatable);
	}
}

static void
text_normal_activate_cb (GSimpleAction *action,
			 GVariant      *parameter,
			 gpointer       user_data)
{
	GeditTextSizeWindowActivatable *self = GEDIT_TEXT_SIZE_WINDOW_ACTIVATABLE (user_data);
	GeditTextSizeViewActivatable *active_view_activatable;

	active_view_activatable = get_active_view_activatable (self);
	if (active_view_activatable != NULL)
	{
		gedit_text_size_view_activatable_reset_to_normal_size (active_view_activatable);
	}
}

static const GActionEntry action_entries[] = {
	{ "text-larger", text_larger_activate_cb },
	{ "text-smaller", text_smaller_activate_cb },
	{ "text-normal", text_normal_activate_cb },
};

static void
add_actions (GeditTextSizeWindowActivatable *self)
{
	g_action_map_add_action_entries (G_ACTION_MAP (self->priv->window),
					 action_entries,
					 G_N_ELEMENTS (action_entries),
					 self);
}

static void
remove_actions (GeditTextSizeWindowActivatable *self)
{
	g_action_map_remove_action_entries (G_ACTION_MAP (self->priv->window),
					    action_entries,
					    G_N_ELEMENTS (action_entries));
}

static void
update_actions_state (GeditTextSizeWindowActivatable *self)
{
	gboolean enabled;
	gint i;

	enabled = gedit_window_get_active_tab (self->priv->window) != NULL;

	for (i = 0; i < G_N_ELEMENTS (action_entries); i++)
	{
		const GActionEntry action_entry = action_entries[i];
		GAction *action;

		action = g_action_map_lookup_action (G_ACTION_MAP (self->priv->window),
						     action_entry.name);

		if (action != NULL)
		{
			g_simple_action_set_enabled (G_SIMPLE_ACTION (action), enabled);
		}
	}
}

static void
gedit_text_size_window_activatable_activate (GeditWindowActivatable *activatable)
{
	GeditTextSizeWindowActivatable *self = GEDIT_TEXT_SIZE_WINDOW_ACTIVATABLE (activatable);

	add_actions (self);
	update_actions_state (self);
}

static void
gedit_text_size_window_activatable_deactivate (GeditWindowActivatable *activatable)
{
	GeditTextSizeWindowActivatable *self = GEDIT_TEXT_SIZE_WINDOW_ACTIVATABLE (activatable);

	remove_actions (self);
}

static void
gedit_text_size_window_activatable_update_state (GeditWindowActivatable *activatable)
{
	GeditTextSizeWindowActivatable *self = GEDIT_TEXT_SIZE_WINDOW_ACTIVATABLE (activatable);

	update_actions_state (self);
}

static void
gedit_window_activatable_iface_init (GeditWindowActivatableInterface *iface)
{
	iface->activate = gedit_text_size_window_activatable_activate;
	iface->deactivate = gedit_text_size_window_activatable_deactivate;
	iface->update_state = gedit_text_size_window_activatable_update_state;
}

void
gedit_text_size_window_activatable_register (PeasObjectModule *module)
{
	gedit_text_size_window_activatable_register_type (G_TYPE_MODULE (module));

	peas_object_module_register_extension_type (module,
						    GEDIT_TYPE_WINDOW_ACTIVATABLE,
						    GEDIT_TYPE_TEXT_SIZE_WINDOW_ACTIVATABLE);
}
