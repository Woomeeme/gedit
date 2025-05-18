/* SPDX-FileCopyrightText: 2024 - Sébastien Wilmet
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "gedit-text-size-view-activatable.h"
#include <gedit/gedit-view-activatable.h>

struct _GeditTextSizeViewActivatablePrivate
{
	GeditView *view;
	PangoFontDescription *default_font;

	/* For Ctrl+scroll event with smooth direction. */
	gdouble cur_delta_y;
};

enum
{
	PROP_0,
	PROP_VIEW
};

#define GEDIT_VIEW_KEY "GeditTextSizeViewActivatable-key"

static void gedit_view_activatable_iface_init (GeditViewActivatableInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (GeditTextSizeViewActivatable,
				gedit_text_size_view_activatable,
				G_TYPE_OBJECT,
				0,
				G_ADD_PRIVATE_DYNAMIC (GeditTextSizeViewActivatable)
				G_IMPLEMENT_INTERFACE_DYNAMIC (GEDIT_TYPE_VIEW_ACTIVATABLE,
							       gedit_view_activatable_iface_init))

/* Returns: (transfer full) (nullable) */
static PangoFontDescription *
get_current_font (GeditTextSizeViewActivatable *self)
{
	GtkStyleContext *style_context;
	PangoFontDescription *font = NULL;

	if (self->priv->view == NULL)
	{
		return NULL;
	}

	style_context = gtk_widget_get_style_context (GTK_WIDGET (self->priv->view));

	gtk_style_context_get (style_context,
			       gtk_style_context_get_state (style_context),
			       GTK_STYLE_PROPERTY_FONT, &font,
			       NULL);

	return font;
}

static void
set_font (GeditTextSizeViewActivatable *self,
	  const PangoFontDescription   *font)
{
	if (self->priv->view != NULL)
	{
		tepl_utils_override_font_description (GTK_WIDGET (self->priv->view), font);
	}
}

static void
change_font_size (GeditTextSizeViewActivatable *self,
		  gint                          amount)
{
	PangoFontDescription *font;
	gint size;
	gint new_size;

	font = get_current_font (self);
	if (font == NULL)
	{
		g_warning ("textsize plugin: failed to get the current font.");
		return;
	}

	size = pango_font_description_get_size (font);
	new_size = size + amount * PANGO_SCALE;
	if (new_size < PANGO_SCALE)
	{
		new_size = PANGO_SCALE;
	}

	if (pango_font_description_get_size_is_absolute (font))
	{
		pango_font_description_set_absolute_size (font, new_size);
	}
	else
	{
		pango_font_description_set_size (font, new_size);
	}

	set_font (self, font);

	pango_font_description_free (font);
}

static void
default_font_changed_cb (TeplSettings                 *settings,
			 GeditTextSizeViewActivatable *self)
{
	g_clear_pointer (&self->priv->default_font, pango_font_description_free);
	self->priv->default_font = get_current_font (self);
}

static gboolean
scroll_event_cb (GeditView                    *view,
		 GdkEventScroll               *event,
		 GeditTextSizeViewActivatable *self)
{
	guint state;

	state = event->state & gtk_accelerator_get_default_mod_mask ();

	if (state != GDK_CONTROL_MASK)
	{
		return GDK_EVENT_PROPAGATE;
	}

	if (event->direction == GDK_SCROLL_UP)
	{
		gedit_text_size_view_activatable_make_larger (self);
		return GDK_EVENT_STOP;
	}
	else if (event->direction == GDK_SCROLL_DOWN)
	{
		gedit_text_size_view_activatable_make_smaller (self);
		return GDK_EVENT_STOP;
	}
	else if (event->direction == GDK_SCROLL_SMOOTH)
	{
		gint discrete_steps;

		self->priv->cur_delta_y += event->delta_y;

		/* A cast from double to int does the same as the trunc() function. */
		discrete_steps = (gint) self->priv->cur_delta_y;

		self->priv->cur_delta_y -= discrete_steps;

		if (discrete_steps != 0)
		{
			/* Note the minus sign. */
			change_font_size (self, -discrete_steps);
		}

		return GDK_EVENT_STOP;
	}

	return GDK_EVENT_PROPAGATE;
}

static gboolean
button_press_event_cb (GeditView                    *view,
		       GdkEventButton               *event,
		       GeditTextSizeViewActivatable *self)
{
	guint state;

	state = event->state & gtk_accelerator_get_default_mod_mask ();

	if (state == GDK_CONTROL_MASK &&
	    event->button == 2)
	{
		gedit_text_size_view_activatable_reset_to_normal_size (self);
		return GDK_EVENT_STOP;
	}

	return GDK_EVENT_PROPAGATE;
}

static void
gedit_text_size_view_activatable_get_property (GObject    *object,
					       guint       prop_id,
					       GValue     *value,
					       GParamSpec *pspec)
{
	GeditTextSizeViewActivatable *activatable = GEDIT_TEXT_SIZE_VIEW_ACTIVATABLE (object);

	switch (prop_id)
	{
		case PROP_VIEW:
			g_value_set_object (value, activatable->priv->view);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gedit_text_size_view_activatable_set_property (GObject      *object,
					       guint         prop_id,
					       const GValue *value,
					       GParamSpec   *pspec)
{
	GeditTextSizeViewActivatable *activatable = GEDIT_TEXT_SIZE_VIEW_ACTIVATABLE (object);

	switch (prop_id)
	{
		case PROP_VIEW:
			g_assert (activatable->priv->view == NULL);
			activatable->priv->view = GEDIT_VIEW (g_value_dup_object (value));

			g_object_set_data (G_OBJECT (activatable->priv->view),
					   GEDIT_VIEW_KEY,
					   activatable);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gedit_text_size_view_activatable_dispose (GObject *object)
{
	GeditTextSizeViewActivatable *activatable = GEDIT_TEXT_SIZE_VIEW_ACTIVATABLE (object);

	if (activatable->priv->view != NULL)
	{
		g_object_set_data (G_OBJECT (activatable->priv->view),
				   GEDIT_VIEW_KEY,
				   NULL);
	}

	g_clear_object (&activatable->priv->view);

	G_OBJECT_CLASS (gedit_text_size_view_activatable_parent_class)->dispose (object);
}

static void
gedit_text_size_view_activatable_finalize (GObject *object)
{
	GeditTextSizeViewActivatable *activatable = GEDIT_TEXT_SIZE_VIEW_ACTIVATABLE (object);

	g_clear_pointer (&activatable->priv->default_font, pango_font_description_free);

	G_OBJECT_CLASS (gedit_text_size_view_activatable_parent_class)->finalize (object);
}

static void
gedit_text_size_view_activatable_class_init (GeditTextSizeViewActivatableClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = gedit_text_size_view_activatable_get_property;
	object_class->set_property = gedit_text_size_view_activatable_set_property;
	object_class->dispose = gedit_text_size_view_activatable_dispose;
	object_class->finalize = gedit_text_size_view_activatable_finalize;

	g_object_class_override_property (object_class, PROP_VIEW, "view");
}

static void
gedit_text_size_view_activatable_class_finalize (GeditTextSizeViewActivatableClass *klass)
{
}

static void
gedit_text_size_view_activatable_init (GeditTextSizeViewActivatable *activatable)
{
	activatable->priv = gedit_text_size_view_activatable_get_instance_private (activatable);

	activatable->priv->cur_delta_y = 0.0;
}

static void
gedit_text_size_view_activatable_activate (GeditViewActivatable *activatable)
{
	GeditTextSizeViewActivatable *self = GEDIT_TEXT_SIZE_VIEW_ACTIVATABLE (activatable);
	TeplSettings *settings;

	g_clear_pointer (&self->priv->default_font, pango_font_description_free);
	self->priv->default_font = get_current_font (self);

	/* When the "font-changed" signal is emitted, it means that the user has
	 * explicitly changed the font setting, and as such he or she probably
	 * wants to use that font instead. So the expected behavior in that case
	 * is to reset the zoom level.
	 *
	 * Connect with the 'after' flag, to ensure to get the new font in the
	 * same manner (with get_current_font()).
	 */
	settings = tepl_settings_get_singleton ();
	g_signal_connect_after (settings,
				"font-changed",
				G_CALLBACK (default_font_changed_cb),
				self);

	g_signal_connect (self->priv->view,
			  "scroll-event",
			  G_CALLBACK (scroll_event_cb),
			  self);

	g_signal_connect (self->priv->view,
			  "button-press-event",
			  G_CALLBACK (button_press_event_cb),
			  self);
}

static void
gedit_text_size_view_activatable_deactivate (GeditViewActivatable *activatable)
{
	GeditTextSizeViewActivatable *self = GEDIT_TEXT_SIZE_VIEW_ACTIVATABLE (activatable);
	TeplSettings *settings;

	settings = tepl_settings_get_singleton ();
	g_signal_handlers_disconnect_by_func (settings, default_font_changed_cb, self);

	g_signal_handlers_disconnect_by_func (self->priv->view, scroll_event_cb, self);
	g_signal_handlers_disconnect_by_func (self->priv->view, button_press_event_cb, self);

	gedit_text_size_view_activatable_reset_to_normal_size (self);
	g_clear_pointer (&self->priv->default_font, pango_font_description_free);
}

static void
gedit_view_activatable_iface_init (GeditViewActivatableInterface *iface)
{
	iface->activate = gedit_text_size_view_activatable_activate;
	iface->deactivate = gedit_text_size_view_activatable_deactivate;
}

void
gedit_text_size_view_activatable_register (PeasObjectModule *module)
{
	gedit_text_size_view_activatable_register_type (G_TYPE_MODULE (module));

	peas_object_module_register_extension_type (module,
						    GEDIT_TYPE_VIEW_ACTIVATABLE,
						    GEDIT_TYPE_TEXT_SIZE_VIEW_ACTIVATABLE);
}

GeditTextSizeViewActivatable *
gedit_text_size_view_activatable_get_from_view (GeditView *view)
{
	g_return_val_if_fail (GEDIT_IS_VIEW (view), NULL);

	return g_object_get_data (G_OBJECT (view), GEDIT_VIEW_KEY);
}

void
gedit_text_size_view_activatable_make_larger (GeditTextSizeViewActivatable *self)
{
	g_return_if_fail (GEDIT_IS_TEXT_SIZE_VIEW_ACTIVATABLE (self));

	change_font_size (self, 1);
}

void
gedit_text_size_view_activatable_make_smaller (GeditTextSizeViewActivatable *self)
{
	g_return_if_fail (GEDIT_IS_TEXT_SIZE_VIEW_ACTIVATABLE (self));

	change_font_size (self, -1);
}

void
gedit_text_size_view_activatable_reset_to_normal_size (GeditTextSizeViewActivatable *self)
{
	g_return_if_fail (GEDIT_IS_TEXT_SIZE_VIEW_ACTIVATABLE (self));

	if (self->priv->default_font != NULL)
	{
		set_font (self, self->priv->default_font);
	}
}
