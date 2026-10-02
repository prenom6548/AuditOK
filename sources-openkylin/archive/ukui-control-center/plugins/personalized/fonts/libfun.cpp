/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include <gio/gio.h>
#include <fontconfig/fontconfig.h>
#include <freetype2/ft2build.h>
#include FT_FREETYPE_H
#include FT_TYPE1_TABLES_H
#include FT_SFNT_NAMES_H
#include FT_TRUETYPE_IDS_H
#include FT_MULTIPLE_MASTERS_H

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QProcess>

#include "libfun.h"

const QDBusArgument &operator<<(QDBusArgument &argument, const FontInformation &key)
{
    argument.beginStructure();
    argument << key.path;
    argument << key.name;
    argument << key.family;
    argument << key.style;
    argument << key.type;
    argument << key.foundry;
    argument << key.version;
    argument << key.copyright;
    argument << key.manufacturer;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, FontInformation &key)
{
    argument.beginStructure();
    argument >> key.path;
    argument >> key.name;
    argument >> key.family;
    argument >> key.style;
    argument >> key.type;
    argument >> key.foundry;
    argument >> key.version;
    argument >> key.copyright;
    argument >> key.manufacturer;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator<<(QDBusArgument &argument, const ViewData &key)
{
    argument.beginStructure();
    argument << key.path;
    argument << key.family;
    argument << key.style;
    argument << key.name;
    argument << key.styleList;
    argument << key.pathList;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, ViewData &key)
{
    argument.beginStructure();
    argument >> key.path;
    argument >> key.family;
    argument >> key.style;
    argument >> key.name;
    argument >> key.styleList;
    argument >> key.pathList;
    argument.endStructure();
    return argument;
}

void LibFun::remove_whitespace(char **ori)
{
    g_auto(GStrv) split = NULL;
    g_autoptr(GString) reassembled = NULL;
    gchar *str = NULL;
    gint i = 0;
    gint stripped = 0;
    size_t len = 0;

    reassembled = g_string_new(NULL);

    split = g_strsplit(*ori , "\n" , -1);

    for (i = 0 ; split[i] != NULL ; i++) {
        str = split[i];

        len = strspn(str , WHITESPACE_CHARS);
        if (len) {
            str += len;
        }

        if (strnlen(str, sizeof(str)) == 0 && ((split[i + 1] == NULL) || (strnlen(split[i + 1], sizeof(split[i + 1])) == 0))) {
            continue;
        }

        if (stripped++ > 0) {
            g_string_append(reassembled , "\n");
        }

        g_string_append(reassembled , str);
    }

    g_free(*ori);
    *ori = g_strdup(reassembled->str);

    return;
}

ViewData::ViewData()
{
    path.clear();
    family.clear();
    style.clear();
    name.clear();
    styleList.clear();
    pathList.clear();
}

ViewData::~ViewData()
{
    path.clear();
    family.clear();
    style.clear();
    name.clear();
    styleList.clear();
    pathList.clear();
}
