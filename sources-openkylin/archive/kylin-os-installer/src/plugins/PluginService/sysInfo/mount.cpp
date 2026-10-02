/*
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
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
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#include "mount.h"
#include <QDebug>
#include "base_unit/file_unit.h"

namespace KServer {

QList<MountItemStr> parseMountItems()
{
    MountItemList result;
    const QString content(KReadFile("/proc/mounts"));
    const QStringList lines = content.split("\n");

    for(const QString& line : lines) {
        const QStringList parts = line.split(" ", QString::SkipEmptyParts);
        if(parts.length() == 6) {
            const MountItemStr item = {parts[0], parts[1], parts[2], parts[3]};
            result.append(item);
        }
    }
    return (MountItemList)result;
}
}
