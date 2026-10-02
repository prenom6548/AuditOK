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

#include "partitionrole.h"
#include <QMap>
namespace KInstaller {
static const QMap <PartitionRole, QString> roleMap {
    {PartitionRole::Extended, QString("extended")},
    {PartitionRole::Logical, QString("logical")},
    {PartitionRole::Lusk, QString("LUSK")},
    {PartitionRole::Lvm_Lv, QString("LVM logical volume")},
    {PartitionRole::Primary, QString("primary")},
    {PartitionRole::Unallocated, QString("unallocated")},
    {PartitionRole::None, QString("none")}
};


}
