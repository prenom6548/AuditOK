/*
 * Copyright (C) 2024 KylinSoftCo., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/
#include "glibinterface.h"
#include <QString>
#include <gio/gio.h>

#define STYLE_TYPE_SCHEMA        "org.ukui.style"
#define KEY_SYSTEM_FONT_SIZE     "system-font-size"
#define DEFAULT_FONT_SIZE	 (10.0)

double getDefaultFontSize()
{
    GSettingsSchemaSource *schema_source = NULL;
    GSettingsSchema  *schema = NULL;
    double defaultFontSize = DEFAULT_FONT_SIZE;

    schema_source = g_settings_schema_source_get_default();
    if (schema_source) {
        schema = g_settings_schema_source_lookup(schema_source,STYLE_TYPE_SCHEMA,TRUE);
        if (schema) {
            GSettings *gs = g_settings_new(STYLE_TYPE_SCHEMA);
            if (gs) {
                GVariant *size = g_settings_get_default_value(gs, KEY_SYSTEM_FONT_SIZE);
                QString fontsize(g_variant_get_string(size, NULL));
                g_variant_unref(size);
                g_object_unref(gs);
                defaultFontSize = fontsize.toDouble();
            }
            g_settings_schema_unref(schema);
        }
    }
    return defaultFontSize;
}
