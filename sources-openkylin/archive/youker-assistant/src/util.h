/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
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

#ifndef UTIL_H
#define UTIL_H

#include <kysdk/kysdk-base/kyutils.h>
#include <QString>
#include <QDebug>
#include <kysdk/kysdk-base/kyutils.h>
#include <QRegularExpression>

class Util
{
public:
    static KDKVolumeBaseType getVolumeUnit(const QString &unit);
    static char* getVolume(const QString &volume, KDKVolumeBaseType unit);
    static bool isVolumeUnit(const QString &str);
    static bool judgmentHW();
    static bool isValidDateFormat(const QString &dateString);
};

#endif // UTIL_H
