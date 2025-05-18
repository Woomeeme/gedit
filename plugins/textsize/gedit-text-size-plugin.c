/* SPDX-FileCopyrightText: 2024 - Sébastien Wilmet
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "gedit-text-size-plugin.h"
#include "gedit-text-size-app-activatable.h"
#include "gedit-text-size-window-activatable.h"
#include "gedit-text-size-view-activatable.h"

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
	gedit_text_size_app_activatable_register (module);
	gedit_text_size_window_activatable_register (module);
	gedit_text_size_view_activatable_register (module);
}
