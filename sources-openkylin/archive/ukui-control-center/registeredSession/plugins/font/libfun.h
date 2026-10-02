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

#include <QDBusArgument>
#include <QDBusMetaType>

#define INSTALL_PATH "/.local/share/fonts/ukui-control-center/"
#define WHITESPACE_CHARS "\f \t"
#define MATCH_VERSION_STR "Version"

typedef struct _FontInformation
{
    QString path;             /* 路径 */
    QString name;             /* 名称 */
    QString family;           /* 系列 */
    QString style;            /* 样式 */
    QString type;             /* 种类 */
    QString foundry;          /* 铸造商 */
    QString version;          /* 版本 */
    QString copyright;        /* 版权 */
    QString manufacturer;     /* 商标 */
} FontInformation;

const QDBusArgument &operator<<(QDBusArgument &argument, const FontInformation &key);
const QDBusArgument &operator>>(const QDBusArgument &argument, FontInformation &key);
Q_DECLARE_METATYPE(FontInformation)

typedef struct _ViewData
{
    QString path;
    QString family;
    QString style;
    QString name;
    QStringList styleList;
    QStringList pathList;
} ViewData;

const QDBusArgument &operator<<(QDBusArgument &argument, const ViewData &key);
const QDBusArgument &operator>>(const QDBusArgument &argument, ViewData &key);
Q_DECLARE_METATYPE(ViewData)

typedef QList<ViewData> ViewDataList;
typedef QList<FontInformation> FontInformationList;

class LibFun
{
public:
    static QList<FontInformation> getAllFontInformation(void);
    static bool installFont(QString path);
    static bool uninstallFont(QString path);
    static bool copyFont(QString src , QString dst);
    static void remove_whitespace(char **ori);
    static void remove_version(char **ori);
    static bool chooseFontFile(QString path);
    static bool caseInsensitiveLessThan(const FontInformation &s1, const FontInformation &s2);
    static QString getFontInfo(QString path);
    static bool isFontFile(QString path);
    static bool updateFontList();
};

#endif
