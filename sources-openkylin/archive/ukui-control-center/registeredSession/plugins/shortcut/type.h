/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
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
#ifndef TYPE_H
#define TYPE_H

#include <QString>
#include <QDBusArgument>
#include <QDBusMetaType>

typedef struct _KeyEntry {
    QString gsSchema;
    QString keyStr;
    QString valueStr;
    QString descStr;
    QString gsPath;
    QString nameStr;
    QString bindingStr;
    QString actionStr;
}KeyEntry;

const QDBusArgument &operator<<(QDBusArgument &argument, const KeyEntry &key);
const QDBusArgument &operator>>(const QDBusArgument &argument, KeyEntry &key);
Q_DECLARE_METATYPE(KeyEntry)

typedef struct {
    QString name;
    QString key;
    QString mediakey;
} QStringPair;

const QDBusArgument &operator<<(QDBusArgument &argument, const QStringPair &string);
const QDBusArgument &operator>>(const QDBusArgument &argument, QStringPair &string);
Q_DECLARE_METATYPE(QStringPair)

typedef QList<KeyEntry> KeyEntryList;
typedef QList<QStringPair> QStringPairList;

#endif // TYPE_H
