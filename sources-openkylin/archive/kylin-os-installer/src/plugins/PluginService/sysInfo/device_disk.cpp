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

#include "device_disk.h"
#include <QDir>


namespace KServer {

QHash<QString, QString> getDevLabelInfo(const QString& dirname)
{
    QHash<QString, QString> result;

    QDir dir(dirname);
    if(!dir.exists()) {
        return result;
    }
    const QFileInfoList items = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files);
    for(const QFileInfo& info : items) {
        result.insert(info.canonicalFilePath(), info.fileName());
    }
    return result;
}

QHash<QString, QString> getparsedLabelDir()
{
    return getDevLabelInfo("/dev/disk/by-label");
}

QHash<QString, QString> getparsedPartLabelDir()
{
    return getDevLabelInfo("/dev/disk/by-partlabel");
}

QHash<QString, QString> getparsedUUIDDir()
{
    return getDevLabelInfo("/dev/disk/by-uuid");
}

}
