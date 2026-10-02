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
#ifndef DATADEFINED_H
#define DATADEFINED_H

#include <QPixmap>
#include <QMap>
#include <QDBusArgument>

struct AutoApp
{
    QString bname;
    QString icon;
    QString name;
    bool hidden;
    int position;
};
const QDBusArgument &operator<<(QDBusArgument &argument, const AutoApp &app);
const QDBusArgument &operator>>(const QDBusArgument &argument, AutoApp &app);
Q_DECLARE_METATYPE(AutoApp)

#endif // DATADEFINED_H
