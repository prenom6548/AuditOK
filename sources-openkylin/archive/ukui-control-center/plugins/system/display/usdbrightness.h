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

#ifndef USDBRIGHTNESS_H
#define USDBRIGHTNESS_H

#include <QDBusArgument>

typedef struct usdBrightness {
    QString screenName;
    int     gammaA;
    int     gammaB;
    int     gammaC;
} UsdBrightness;

QDBusArgument &operator<<(QDBusArgument &argument, const UsdBrightness &mystruct) {
    argument.beginStructure();
    argument << mystruct.screenName << mystruct.gammaA << mystruct.gammaB << mystruct.gammaC;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, UsdBrightness &mystruct) {
    argument.beginStructure();
    argument >> mystruct.screenName >> mystruct.gammaA >> mystruct.gammaB >> mystruct.gammaC;
    argument.endStructure();
    return argument;
}
Q_DECLARE_METATYPE(UsdBrightness)

typedef QList<UsdBrightness> UsdBrightnessList;

#endif // USDBRIGHTNESS_H
