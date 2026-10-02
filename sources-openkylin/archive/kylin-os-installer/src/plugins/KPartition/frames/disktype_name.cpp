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

#include "disktype_name.h"
#include <QObject>

QString findNameByDiskType(DiskType type)
{
    QString str = "";
    switch (type) {
    case DiskType::WD:
        return str = QObject::tr("wd");
    case DiskType::Seagate:
        return str = QObject::tr("seagate");
    case DiskType::Hitachi:
        return str = QObject::tr("hitachi");
    case DiskType::Samsung:
        return str = QObject::tr("samsung");
    case DiskType::Toshiba:
        return str = QObject::tr("toshiba");
    case DiskType::Fujitsu:
        return str = QObject::tr("fujitsu");
    case DiskType::Maxtor:
        return str = QObject::tr("maxtor");
    case DiskType::IBM:
        return str = QObject::tr("IBM");
    case DiskType::ExcelStor:
        return str = QObject::tr("excelStor");
    case DiskType::LENOVO:
        return str = QObject::tr("lenovo");
    case DiskType::Other:
        return str = QObject::tr("other");
    default:
        return str = QObject::tr("Unknown");
        break;
    }
}
