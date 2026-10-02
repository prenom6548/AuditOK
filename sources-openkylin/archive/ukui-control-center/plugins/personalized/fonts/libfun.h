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
#ifndef __LIB_FUN_H__
#define __LIB_FUN_H__

#include <QString>
#include <QList>
#include <QObject>
#include <QDBusArgument>
#include <QDBusMetaType>

#define INSTALL_PATH "/.local/share/fonts/ukui-control-center/"
#define WHITESPACE_CHARS "\f \t"
#define MATCH_VERSION_STR "Version"

typedef struct _FontInformation
{
    QString path;
    QString name;
    QString family;
    QString style;
    QString type;
    QString foundry;
    QString version;
    QString copyright;
    QString manufacturer;
} FontInformation;

struct ViewData
{
    ViewData();
    ~ViewData();
    QString path;
    QString family;
    QString style;
    QString name;
    QStringList styleList;
    QStringList pathList;
};

class LibFun
{
public:
    static QList<FontInformation> getAllFontInformation(void);
    static bool installFont(QString path);
    static bool copyFont(QString src , QString dst);
    static void remove_whitespace(char **ori);
    static void remove_version(char **ori);
    static bool chooseFontFile(QString path);
    static bool caseInsensitiveLessThan(const FontInformation &s1, const FontInformation &s2);
    static bool isFontFile(QString path);
};

const QDBusArgument &operator<<(QDBusArgument &argument, const FontInformation &key);
const QDBusArgument &operator>>(const QDBusArgument &argument, FontInformation &key);
Q_DECLARE_METATYPE(FontInformation)

const QDBusArgument &operator<<(QDBusArgument &argument, const ViewData &key);
const QDBusArgument &operator>>(const QDBusArgument &argument, ViewData &key);
Q_DECLARE_METATYPE(ViewData)

typedef QList<ViewData> ViewDataList;

#endif
