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

#include "meminfo.h"
#include <QHash>
#include <QDebug>
#include "../base_unit/file_unit.h"

namespace KServer {

SysMemsInfo getComputerMemInfo()
{
    SysMemsInfo memResult;
    QString memInfoALL = KReadFile("/proc/meminfo");
    if (memInfoALL.isEmpty()) {
        qWarning() << "Failed to read meminfo!";
        return memResult;
    }

    QHash<QString, qint64> haMemInfo;
    for(const QString& line : memInfoALL.split('\n')) {
        const int index = line.indexOf(":");
        if(index > -1) {
            QString valueStr = QString("");
            valueStr = line.mid(index + 1);
            valueStr.remove("kB");
            valueStr = valueStr.trimmed();

            const qint64 value = valueStr.toLongLong() * 1024;
            haMemInfo.insert(line.left(index), value);
        }
    }

    memResult.memFree = haMemInfo.value("MemFree");
    memResult.cached = haMemInfo.value("Cached");
    memResult.swapTotal = haMemInfo.value("SwapTotal");
    memResult.buffers = haMemInfo.value("Buffers");
    memResult.memTotal = haMemInfo.value("MemTotal");
    memResult.memAvailable = haMemInfo.value("MemAvailabel");
    memResult.swapFree = haMemInfo.value("SwapFree");

    return memResult;
}

}
