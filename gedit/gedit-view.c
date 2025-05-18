/*
 * This file is part of gedit
 *
 * Copyright (C) 1998, 1999 Alex Roberts, Evan Lawrence
 * Copyright (C) 2000, 2002 Chema Celorio, Paolo Maggi
 * Copyright (C) 2003-2005 Paolo Maggi
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "gedit-view.h"
#include <libpeas/peas-extension-set.h>
#include "gedit-view-activatable.h"
#include "gedit-plugins-engine.h"
#include "gedit-utils.h"
#include "gedit-settings.h"

/**
 * SECTION:gedit-view
 * @Title: GeditView
 * @Short_description: A document view
 *
 * #GeditView is a subclass of #TeplView and is the #GtkWidget showing a
 * document.
 *
 * It contains a #GeditDocument which can be retrieved with
 * gtk_text_view_get_buffer().
 */

struct _GeditViewPrivate
{
	PeasExtensionSet *extensions;

	gchar *direct_save_uri;

	TeplSignalGroup *file_signal_group;

	TeplSettingsBindingGroup *settings_binding_group;
};

enum
{
	TARGET_URI_LIST = 100,
	TARGET_XDNDDIRECTSAVE
};

enum
{
	SIGNAL_DROP_URIS,
	N_SIGNALS
};

static guint signals[N_SIGNALS];

G_DEFINE_TYPE_WITH_PRIVATE (GeditView, gedit_view, TEPL_TYPE_VIEW)

static void
update_font (GeditView *view)
{
	TeplSettings *settings;
	gchar *selected_font;

	settings = tepl_settings_get_singleton ();

	selected_font = tepl_settings_get_selected_font (settings);
	tepl_utils_override_font_string (GTK_WIDGET (view), selected_font);
	g_free (selected_font);
}

static void
font_changed_cb (TeplSettings *settings,
		 GeditView    *view)
{
	update_font (view);
}

static void
update_editable (GeditView *view)
{
	GeditDocument *doc;
	GtkSourceFile *file;

	doc = GEDIT_DOCUMENT (gtk_text_view_get_buffer (GTK_TEXT_VIEW (view)));
	file = gedit_document_get_file (doc);

	gtk_text_view_set_editable (GTK_TEXT_VIEW (view),
				    !gtk_source_file_is_readonly (file));
}

static void
file_read_only_notify_cb (GtkSourceFile *file,
			  GParamSpec    *pspec,
			  GeditView     *view)
{
	update_editable (view);
}

static void
buffer_changed (GeditView *view)
{
	GeditDocument *doc;
	GtkSourceFile *file;

	doc = GEDIT_DOCUMENT (gtk_text_view_get_buffer (GTK_TEXT_VIEW (view)));
	file = gedit_document_get_file (doc);

	tepl_signal_group_clear (&view->priv->file_signal_group);
	view->priv->file_signal_group = tepl_signal_group_new (G_OBJECT (file));

	tepl_signal_group_add (view->priv->file_signal_group,
			       g_signal_connect (file,
						 "notify::read-only",
						 G_CALLBACK (file_read_only_notify_cb),
						 view));

	update_editable (view);
}

static void
buffer_notify_cb (GeditView  *view,
		  GParamSpec *pspec,
		  gpointer    user_data)
{
	buffer_changed (view);
}

static GdkAtom
drag_get_uri_target (GtkWidget      *widget,
		     GdkDragContext *context)
{
	GdkAtom target;
	GtkTargetList *target_list;

	target_list = gtk_target_list_new (NULL, 0);
	gtk_target_list_add_uri_targets (target_list, 0);

	target = gtk_drag_dest_find_target (widget, context, target_list);
	gtk_target_list_unref (target_list);

	return target;
}

static gboolean
gedit_view_drag_motion (GtkWidget      *widget,
			GdkDragContext *context,
			gint            x,
			gint            y,
			guint           timestamp)
{
	gboolean drop_zone;

	/* Chain up to allow textview to scroll and position dnd mark, note
	 * that this needs to be checked if gtksourceview or gtktextview
	 * changes drag_motion behaviour.
	 */
	drop_zone = GTK_WIDGET_CLASS (gedit_view_parent_class)->drag_motion (widget,
									     context,
									     x, y,
									     timestamp);

	/* If this is a URL, deal with it here */
	if (drag_get_uri_target (widget, context) != GDK_NONE)
	{
		gdk_drag_status (context,
				 gdk_drag_context_get_suggested_action (context),
				 timestamp);
		drop_zone = TRUE;
	}

	return drop_zone;
}

static void
gedit_view_drag_data_received (GtkWidget        *widget,
		       	       GdkDragContext   *context,
			       gint              x,
			       gint              y,
			       GtkSelectionData *selection_data,
			       guint             info,
			       guint             timestamp)
{
	/* If this is an URL emit SIGNAL_DROP_URIS, otherwise chain up the
	 * signal.
	 */
	switch (info)
	{
		case TARGET_URI_LIST:
		{
			gchar **uri_list;

			uri_list = gedit_utils_drop_get_uris (selection_data);

			if (uri_list != NULL)
			{
				g_signal_emit (widget, signals[SIGNAL_DROP_URIS], 0, uri_list);
				g_strfreev (uri_list);

				gtk_drag_finish (context, TRUE, FALSE, timestamp);
			}

			break;

		}
		case TARGET_XDNDDIRECTSAVE:
		{
			GeditView *view;

			view = GEDIT_VIEW (widget);

			/* Indicate that we don't provide "F" fallback */
			if (gtk_selection_data_get_format (selection_data) == 8 &&
			    gtk_selection_data_get_length (selection_data) == 1 &&
			    gtk_selection_data_get_data (selection_data)[0] == 'F')
			{
				gdk_property_change (gdk_drag_context_get_source_window (context),
						     gdk_atom_intern ("XdndDirectSave0", FALSE),
						     gdk_atom_intern ("text/plain", FALSE), 8,
						     GDK_PROP_MODE_REPLACE, (const guchar *) "", 0);
			}
			else if (gtk_selection_data_get_format (selection_data) == 8 &&
				 gtk_selection_data_get_length (selection_data) == 1 &&
				 gtk_selection_data_get_data (selection_data)[0] == 'S' &&
				 view->priv->direct_save_uri != NULL)
			{
				gchar **uris;

				uris = g_new (gchar *, 2);
				uris[0] = view->priv->direct_save_uri;
				uris[1] = NULL;
				g_signal_emit (widget, signals[SIGNAL_DROP_URIS], 0, uris);
				g_free (uris);
			}

			g_free (view->priv->direct_save_uri);
			view->priv->direct_save_uri = NULL;

			gtk_drag_finish (context, TRUE, FALSE, timestamp);

			break;
		}
		default:
		{
			GTK_WIDGET_CLASS (gedit_view_parent_class)->drag_data_received (widget,
											context,
											x, y,
											selection_data,
											info,
											timestamp);
			break;
		}
	}
}

static gboolean
gedit_view_drag_drop (GtkWidget      *widget,
		      GdkDragContext *context,
		      gint            x,
		      gint            y,
		      guint           timestamp)
{
	gboolean drop_zone;
	GdkAtom target;
	guint info;
	gboolean found;
	GtkTargetList *target_list;

	target_list = gtk_drag_dest_get_target_list (widget);
	target = gtk_drag_dest_find_target (widget, context, target_list);
	found = gtk_target_list_find (target_list, target, &info);

	if (found && (info == TARGET_URI_LIST || info == TARGET_XDNDDIRECTSAVE))
	{
		if (info == TARGET_XDNDDIRECTSAVE)
		{
			gchar *uri;
			uri = gedit_utils_set_direct_save_filename (context);

			if (uri != NULL)
			{
				GeditView *view = GEDIT_VIEW (widget);
				g_free (view->priv->direct_save_uri);
				view->priv->direct_save_uri = uri;
			}
		}

		gtk_drag_get_data (widget, context, target, timestamp);
		drop_zone = TRUE;
	}
	else
	{
		/* Chain up */
		drop_zone = GTK_WIDGET_CLASS (gedit_view_parent_class)->drag_drop (widget,
										   context,
										   x, y,
										   timestamp);
	}

	return drop_zone;
}

static void
extension_added (PeasExtensionSet *extensions,
		 PeasPluginInfo   *info,
		 PeasExtension    *exten,
		 GeditView        *view)
{
	gedit_view_activatable_activate (GEDIT_VIEW_ACTIVATABLE (exten));
}

static void
extension_removed (PeasExtensionSet *extensions,
		   PeasPluginInfo   *info,
		   PeasExtension    *exten,
		   GeditView        *view)
{
	gedit_view_activatable_deactivate (GEDIT_VIEW_ACTIVATABLE (exten));
}

static void
gedit_view_realize (GtkWidget *widget)
{
	GeditView *view = GEDIT_VIEW (widget);

	GTK_WIDGET_CLASS (gedit_view_parent_class)->realize (widget);

	g_signal_connect (view->priv->extensions,
	                  "extension-added",
	                  G_CALLBACK (extension_added),
	                  view);

	g_signal_connect (view->priv->extensions,
	                  "extension-removed",
	                  G_CALLBACK (extension_removed),
	                  view);

	/* We only activate the extensions when the view is realized,
	 * because most plugins will expect this behaviour, and we won't
	 * change the buffer later anyway.
	 */
	peas_extension_set_foreach (view->priv->extensions,
	                            (PeasExtensionSetForeachFunc) extension_added,
	                            view);
}

static void
gedit_view_unrealize (GtkWidget *widget)
{
	GeditView *view = GEDIT_VIEW (widget);

	g_signal_handlers_disconnect_by_func (view->priv->extensions, extension_added, view);
	g_signal_handlers_disconnect_by_func (view->priv->extensions, extension_removed, view);

	/* We need to deactivate the extension on unrealize because it is not
	 * mandatory that a view has been realized when we dispose it, leading
	 * to deactivating the plugin without being activated.
	 */
	peas_extension_set_foreach (view->priv->extensions,
	                            (PeasExtensionSetForeachFunc) extension_removed,
	                            view);

	GTK_WIDGET_CLASS (gedit_view_parent_class)->unrealize (widget);
}

static GtkTextBuffer *
gedit_view_create_buffer (GtkTextView *text_view)
{
	return GTK_TEXT_BUFFER (gedit_document_new ());
}

static void
gedit_view_constructed (GObject *object)
{
	GeditView *view = GEDIT_VIEW (object);
	GeditSettings *gedit_settings;
	TeplSettings *tepl_settings;
	GSettings *editor_settings;

	G_OBJECT_CLASS (gedit_view_parent_class)->constructed (object);

	gedit_settings = _gedit_settings_get_singleton ();
	tepl_settings = tepl_settings_get_singleton ();

	editor_settings = _gedit_settings_peek_editor_settings (gedit_settings);

	update_font (view);
	g_signal_connect_object (tepl_settings,
				 "font-changed",
				 G_CALLBACK (font_changed_cb),
				 view,
				 0);

	g_settings_bind (editor_settings, GEDIT_SETTINGS_DISPLAY_LINE_NUMBERS,
			 view, "show-line-numbers",
			 G_SETTINGS_BIND_GET | G_SETTINGS_BIND_NO_SENSITIVITY);
	tepl_settings_binding_group_add (view->priv->settings_binding_group, "show-line-numbers");

	g_settings_bind (editor_settings, GEDIT_SETTINGS_AUTO_INDENT,
			 view, "auto-indent",
			 G_SETTINGS_BIND_GET | G_SETTINGS_BIND_NO_SENSITIVITY);
	tepl_settings_binding_group_add (view->priv->settings_binding_group, "auto-indent");

	g_settings_bind (editor_settings, GEDIT_SETTINGS_TABS_SIZE,
			 view, "tab-width",
			 G_SETTINGS_BIND_GET | G_SETTINGS_BIND_NO_SENSITIVITY);
	tepl_settings_binding_group_add (view->priv->settings_binding_group, "tab-width");

	g_settings_bind (editor_settings, GEDIT_SETTINGS_INSERT_SPACES,
			 view, "insert-spaces-instead-of-tabs",
			 G_SETTINGS_BIND_GET | G_SETTINGS_BIND_NO_SENSITIVITY);
	tepl_settings_binding_group_add (view->priv->settings_binding_group, "insert-spaces-instead-of-tabs");

	g_settings_bind (editor_settings, GEDIT_SETTINGS_DISPLAY_RIGHT_MARGIN,
			 view, "show-right-margin",
			 G_SETTINGS_BIND_GET | G_SETTINGS_BIND_NO_SENSITIVITY);
	tepl_settings_binding_group_add (view->priv->settings_binding_group, "show-right-margin");

	g_settings_bind (editor_settings, GEDIT_SETTINGS_RIGHT_MARGIN_POSITION,
			 view, "right-margin-position",
			 G_SETTINGS_BIND_GET | G_SETTINGS_BIND_NO_SENSITIVITY);
	tepl_settings_binding_group_add (view->priv->settings_binding_group, "right-margin-position");

	g_settings_bind (editor_settings, GEDIT_SETTINGS_HIGHLIGHT_CURRENT_LINE,
			 view, "highlight-current-line",
			 G_SETTINGS_BIND_GET | G_SETTINGS_BIND_NO_SENSITIVITY);
	tepl_settings_binding_group_add (view->priv->settings_binding_group, "highlight-current-line");

	g_settings_bind (editor_settings, GEDIT_SETTINGS_WRAP_MODE,
			 view, "wrap-mode",
			 G_SETTINGS_BIND_GET | G_SETTINGS_BIND_NO_SENSITIVITY);
	tepl_settings_binding_group_add (view->priv->settings_binding_group, "wrap-mode");

	g_settings_bind (editor_settings, GEDIT_SETTINGS_SMART_HOME_END,
			 view, "smart-home-end",
			 G_SETTINGS_BIND_GET | G_SETTINGS_BIND_NO_SENSITIVITY);
	tepl_settings_binding_group_add (view->priv->settings_binding_group, "smart-home-end");
}

static void
gedit_view_dispose (GObject *object)
{
	GeditView *view = GEDIT_VIEW (object);

	if (view->priv->settings_binding_group != NULL)
	{
		tepl_settings_binding_group_unbind (view->priv->settings_binding_group, object);
		tepl_settings_binding_group_free (view->priv->settings_binding_group);
		view->priv->settings_binding_group = NULL;
	}

	g_clear_object (&view->priv->extensions);
	tepl_signal_group_clear (&view->priv->file_signal_group);

	/* Disconnect notify buffer because the destroy of the textview will set
	 * the buffer to NULL, and we call get_buffer in the notify which would
	 * reinstate a buffer which we don't want.
	 * There is no problem calling g_signal_handlers_disconnect_by_func()
	 * several times (if dispose() is called several times).
	 */
	g_signal_handlers_disconnect_by_func (view, buffer_notify_cb, NULL);

	G_OBJECT_CLASS (gedit_view_parent_class)->dispose (object);
}

static void
gedit_view_class_init (GeditViewClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
	GtkTextViewClass *text_view_class = GTK_TEXT_VIEW_CLASS (klass);
	GtkBindingSet *binding_set;

	object_class->constructed = gedit_view_constructed;
	object_class->dispose = gedit_view_dispose;

	/* Override the gtk_text_view_drag_motion and drag_drop
	 * functions to get URIs
	 *
	 * If the mime type is text/uri-list, then we will accept
	 * the potential drop, or request the data (depending on the
	 * function).
	 *
	 * If the drag context has any other mime type, then pass the
	 * information onto the GtkTextView's standard handlers.
	 *
	 * See bug #89881 for details
	 */
	widget_class->drag_motion = gedit_view_drag_motion;
	widget_class->drag_data_received = gedit_view_drag_data_received;
	widget_class->drag_drop = gedit_view_drag_drop;

	widget_class->realize = gedit_view_realize;
	widget_class->unrealize = gedit_view_unrealize;

	text_view_class->create_buffer = gedit_view_create_buffer;

	/**
	 * GeditView::drop-uris:
	 * @view: a #GeditView.
	 * @uri_list: a %NULL-terminated list of URIs.
	 *
	 * The #GeditView::drop-uris signal allows plugins to intercept the
	 * default drag-and-drop behaviour of 'text/uri-list'. #GeditView
	 * handles drag-and-drop in the default handlers of
	 * #GtkWidget::drag-drop, #GtkWidget::drag-motion and
	 * #GtkWidget::drag-data-received. The view emits the
	 * #GeditView::drop-uris signal from #GtkWidget::drag-data-received if
	 * valid URIs have been dropped.  Plugins should connect to
	 * #GtkWidget::drag-motion, #GtkWidget::drag-drop and
	 * #GtkWidget::drag-data-received to change this default behaviour. They
	 * should NOT use this signal because this will not prevent gedit from
	 * loading the URI.
	 */
	signals[SIGNAL_DROP_URIS] =
		g_signal_new ("drop-uris",
		              G_TYPE_FROM_CLASS (object_class),
		              G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
		              G_STRUCT_OFFSET (GeditViewClass, drop_uris),
		              NULL, NULL, NULL,
		              G_TYPE_NONE, 1, G_TYPE_STRV);

	/* FIXME: some of these bindings - especially for the "change-case" -
	 * could be moved to a plugin that enables more advanced keyboard
	 * shortcuts. Enabling too many keyboard shortcuts by default makes the
	 * user experience worse, because when mistyping a key it could trigger
	 * an unknown/unexpected action, so it's a bit a WTF moment for the
	 * user. gedit must not become like Emacs or Vim. More advanced stuff
	 * are put in plugins. Also, the "change-case" is available in the
	 * right-click menu, so it's already easily accessible.
	 */
	binding_set = gtk_binding_set_by_class (klass);

	gtk_binding_entry_add_signal (binding_set,
	                              GDK_KEY_d,
	                              GDK_CONTROL_MASK,
	                              "delete-from-cursor", 2,
	                              G_TYPE_ENUM, GTK_DELETE_PARAGRAPHS,
	                              G_TYPE_INT, 1);

	gtk_binding_entry_add_signal (binding_set,
	                              GDK_KEY_u,
	                              GDK_CONTROL_MASK,
	                              "change-case", 1,
	                              G_TYPE_ENUM, GTK_SOURCE_CHANGE_CASE_UPPER);

	gtk_binding_entry_add_signal (binding_set,
	                              GDK_KEY_l,
	                              GDK_CONTROL_MASK,
	                              "change-case", 1,
	                              G_TYPE_ENUM, GTK_SOURCE_CHANGE_CASE_LOWER);

	gtk_binding_entry_add_signal (binding_set,
	                              GDK_KEY_asciitilde,
	                              GDK_CONTROL_MASK,
	                              "change-case", 1,
	                              G_TYPE_ENUM, GTK_SOURCE_CHANGE_CASE_TOGGLE);
}

static void
gedit_view_init (GeditView *view)
{
	GtkTargetList *target_list;

	view->priv = gedit_view_get_instance_private (view);

	view->priv->settings_binding_group = tepl_settings_binding_group_new ();

	/* Drag and drop support */
	view->priv->direct_save_uri = NULL;
	target_list = gtk_drag_dest_get_target_list (GTK_WIDGET (view));

	if (target_list != NULL)
	{
		gtk_target_list_add (target_list,
		                     gdk_atom_intern ("XdndDirectSave0", FALSE),
		                     0,
		                     TARGET_XDNDDIRECTSAVE);
		gtk_target_list_add_uri_targets (target_list, TARGET_URI_LIST);
	}

	/* GeditViewActivatable */
	view->priv->extensions =
		peas_extension_set_new (PEAS_ENGINE (gedit_plugins_engine_get_default ()),
		                        GEDIT_TYPE_VIEW_ACTIVATABLE,
		                        "view", view,
		                        NULL);

	/* Act on buffer changes */
	buffer_changed (view);
	g_signal_connect (view,
			  "notify::buffer",
			  G_CALLBACK (buffer_notify_cb),
			  NULL);

	/* Useful for on screen keyboards */
	gtk_text_view_set_input_hints (GTK_TEXT_VIEW (view), GTK_INPUT_HINT_WORD_COMPLETION);
}

/**
 * gedit_view_new:
 * @doc: a #GeditDocument.
 *
 * Creates a new #GeditView object displaying @doc. @doc cannot be %NULL.
 *
 * Returns: (transfer floating): a new #GeditView.
 */
GtkWidget *
gedit_view_new (GeditDocument *doc)
{
	g_return_val_if_fail (GEDIT_IS_DOCUMENT (doc), NULL);

	return g_object_new (GEDIT_TYPE_VIEW,
			     "buffer", doc,
			     NULL);
}

/* ex:set ts=8 noet: */
