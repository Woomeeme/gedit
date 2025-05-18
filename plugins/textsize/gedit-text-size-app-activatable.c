/* SPDX-FileCopyrightText: 2024 - Sébastien Wilmet
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "gedit-text-size-app-activatable.h"
#include <glib/gi18n.h>
#include <gedit/gedit-app.h>
#include <gedit/gedit-app-activatable.h>

struct _GeditTextSizeAppActivatablePrivate
{
	GeditApp *app;
	GeditMenuExtension *menu_extension;
};

enum
{
	PROP_0,
	PROP_APP
};

static void gedit_app_activatable_iface_init (GeditAppActivatableInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (GeditTextSizeAppActivatable,
				gedit_text_size_app_activatable,
				G_TYPE_OBJECT,
				0,
				G_ADD_PRIVATE_DYNAMIC (GeditTextSizeAppActivatable)
				G_IMPLEMENT_INTERFACE_DYNAMIC (GEDIT_TYPE_APP_ACTIVATABLE,
							       gedit_app_activatable_iface_init))

static void
gedit_text_size_app_activatable_get_property (GObject    *object,
					      guint       prop_id,
					      GValue     *value,
					      GParamSpec *pspec)
{
	GeditTextSizeAppActivatable *activatable = GEDIT_TEXT_SIZE_APP_ACTIVATABLE (object);

	switch (prop_id)
	{
		case PROP_APP:
			g_value_set_object (value, activatable->priv->app);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gedit_text_size_app_activatable_set_property (GObject      *object,
					      guint         prop_id,
					      const GValue *value,
					      GParamSpec   *pspec)
{
	GeditTextSizeAppActivatable *activatable = GEDIT_TEXT_SIZE_APP_ACTIVATABLE (object);

	switch (prop_id)
	{
		case PROP_APP:
			g_assert (activatable->priv->app == NULL);
			activatable->priv->app = GEDIT_APP (g_value_dup_object (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gedit_text_size_app_activatable_dispose (GObject *object)
{
	GeditTextSizeAppActivatable *activatable = GEDIT_TEXT_SIZE_APP_ACTIVATABLE (object);

	g_clear_object (&activatable->priv->app);
	g_clear_object (&activatable->priv->menu_extension);

	G_OBJECT_CLASS (gedit_text_size_app_activatable_parent_class)->dispose (object);
}

static void
gedit_text_size_app_activatable_finalize (GObject *object)
{

	G_OBJECT_CLASS (gedit_text_size_app_activatable_parent_class)->finalize (object);
}

static void
gedit_text_size_app_activatable_class_init (GeditTextSizeAppActivatableClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = gedit_text_size_app_activatable_get_property;
	object_class->set_property = gedit_text_size_app_activatable_set_property;
	object_class->dispose = gedit_text_size_app_activatable_dispose;
	object_class->finalize = gedit_text_size_app_activatable_finalize;

	g_object_class_override_property (object_class, PROP_APP, "app");
}

static void
gedit_text_size_app_activatable_class_finalize (GeditTextSizeAppActivatableClass *klass)
{
}

static void
gedit_text_size_app_activatable_init (GeditTextSizeAppActivatable *activatable)
{
	activatable->priv = gedit_text_size_app_activatable_get_instance_private (activatable);
}

static void
add_accels (GtkApplication *app)
{
	const gchar *accels[3];

	accels[0] = "<Control>plus";
	accels[1] = "<Control>KP_Add";
	accels[2] = NULL;
	gtk_application_set_accels_for_action (app, "win.text-larger", accels);

	accels[0] = "<Control>minus";
	accels[1] = "<Control>KP_Subtract";
	gtk_application_set_accels_for_action (app, "win.text-smaller", accels);

	accels[0] = "<Control>0";
	accels[1] = "<Control>KP_0";
	gtk_application_set_accels_for_action (app, "win.text-normal", accels);
}

static void
remove_accels (GtkApplication *app)
{
	const gchar *null_accels[] = { NULL };

	gtk_application_set_accels_for_action (app, "win.text-larger", null_accels);
	gtk_application_set_accels_for_action (app, "win.text-smaller", null_accels);
	gtk_application_set_accels_for_action (app, "win.text-normal", null_accels);
}

static void
extend_menu (GeditTextSizeAppActivatable *self)
{
	GMenuItem *menu_item;

	g_clear_object (&self->priv->menu_extension);
	self->priv->menu_extension = gedit_app_activatable_extend_menu (GEDIT_APP_ACTIVATABLE (self),
									"view-section-2");

	menu_item = g_menu_item_new (_("_Normal size"), "win.text-normal");
	gedit_menu_extension_prepend_menu_item (self->priv->menu_extension, menu_item);
	g_object_unref (menu_item);

	menu_item = g_menu_item_new (_("S_maller Text"), "win.text-smaller");
	gedit_menu_extension_prepend_menu_item (self->priv->menu_extension, menu_item);
	g_object_unref (menu_item);

	menu_item = g_menu_item_new (_("_Larger Text"), "win.text-larger");
	gedit_menu_extension_prepend_menu_item (self->priv->menu_extension, menu_item);
	g_object_unref (menu_item);
}

static void
gedit_text_size_app_activatable_activate (GeditAppActivatable *activatable)
{
	GeditTextSizeAppActivatable *self = GEDIT_TEXT_SIZE_APP_ACTIVATABLE (activatable);

	add_accels (GTK_APPLICATION (self->priv->app));
	extend_menu (self);
}

static void
gedit_text_size_app_activatable_deactivate (GeditAppActivatable *activatable)
{
	GeditTextSizeAppActivatable *self = GEDIT_TEXT_SIZE_APP_ACTIVATABLE (activatable);

	remove_accels (GTK_APPLICATION (self->priv->app));
	g_clear_object (&self->priv->menu_extension);
}

static void
gedit_app_activatable_iface_init (GeditAppActivatableInterface *iface)
{
	iface->activate = gedit_text_size_app_activatable_activate;
	iface->deactivate = gedit_text_size_app_activatable_deactivate;
}

void
gedit_text_size_app_activatable_register (PeasObjectModule *module)
{
	gedit_text_size_app_activatable_register_type (G_TYPE_MODULE (module));

	peas_object_module_register_extension_type (module,
						    GEDIT_TYPE_APP_ACTIVATABLE,
						    GEDIT_TYPE_TEXT_SIZE_APP_ACTIVATABLE);
}
