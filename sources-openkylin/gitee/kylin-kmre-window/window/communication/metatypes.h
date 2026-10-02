/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DBUS_METATYPES_H_
#define DBUS_METATYPES_H_

#include <QMetaType>
#include <QMetaObject>
#include <QDBusMetaType>
#include <QDBusArgument>
#include <QString>
#include <QDebug>

//struct AndroidMeta {
//    QString path;
//    QString mimeType;
//};

class AndroidMeta
{
public:
    QString path;
    QString mimeType;
    QString pkgName;

    friend QDebug operator<<(QDebug argument, const AndroidMeta &data) {
        argument << data.path;
        return argument;
    }

    friend QDBusArgument& operator<<(QDBusArgument& arg, const AndroidMeta& meta) {
        arg.beginStructure();
        arg << meta.path << meta.mimeType << meta.pkgName;
        arg.endStructure();
        return arg;
    }

    friend const QDBusArgument& operator>>(const QDBusArgument& arg, AndroidMeta& meta) {
        arg.beginStructure();
        arg >> meta.path >> meta.mimeType >> meta.pkgName;
        arg.endStructure();
        return arg;
    }

    bool operator==(const AndroidMeta data) const {
        return data.path == path;
    }

    bool operator!=(const AndroidMeta data) const {
        return data.path != path;
    }
};

typedef QList<AndroidMeta> AndroidMetaList;
Q_DECLARE_METATYPE(AndroidMeta);
Q_DECLARE_METATYPE(AndroidMetaList);
Q_DECLARE_METATYPE(QList<QByteArray>)

#endif // DBUS_METATYPES_H_
