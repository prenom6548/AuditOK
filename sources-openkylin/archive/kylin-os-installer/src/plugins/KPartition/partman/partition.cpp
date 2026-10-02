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

#include "partition.h"
#include <QDebug>
#include <QRegularExpression>
namespace KInstaller {
namespace Partman {
Partition::Partition():
    m_num(-1),
    fs(FSType::Empty),
    m_sectorStart(-1),
    m_sectorEnd(-1),
    m_sectorSize(0),
    m_total(-1),
    m_freespace(0),
    m_devPath(),
    m_partitionPath(),
    m_FSlabel(),
    m_name(),
    m_type(PartitionType::Unallocated),
    m_status(PartitionStatus::Real),
    blFormatPartition(false),
    flags(),
    m_mountPoint(),
    m_isMounted(0),
    osType(OSType::Empty)
{

}
Partition::Partition(const Partition &partition):
    m_num(partition.m_num),
    fs(partition.fs),
    realFs(partition.realFs),
    m_sectorStart(partition.m_sectorStart),
    m_sectorEnd(partition.m_sectorEnd),
    m_sectorSize(partition.m_sectorSize),
    m_total(partition.m_total),
    m_freespace(partition.m_freespace),
    m_devPath(partition.m_devPath),
    m_partitionPath(partition.m_partitionPath),
    m_FSlabel(partition.m_FSlabel),
    m_name(partition.m_name),
    m_OSname(partition.m_OSname),
    m_type(partition.m_type),    
    m_status(partition.m_status),
    blFormatPartition(partition.blFormatPartition),
    flags(partition.flags),
    m_mountPoint(partition.m_mountPoint),
    m_isMounted(partition.m_isMounted),
   osType(partition.osType)
{

}

Partition::~Partition()
{

}

bool Partition::operator==(const Partition &other) const {
   return (this->m_devPath == other.m_devPath &&
          this->m_sectorStart == other.m_sectorStart &&
          this->m_sectorEnd == other.m_sectorEnd &&
          this->m_sectorSize == other.m_sectorSize &&
          m_type == other.m_type);
}


qint64 Partition::getSectorLenth()
{
    if(m_sectorStart >= 0 && m_sectorEnd >= 0) {
        return (m_sectorEnd - m_sectorStart +1);
    } else {
        qWarning() << "分区起始位置设置不正确";
        return -1;
    }
}

int Partition::getNum()
{
    if(m_num > 0) {
        return m_num;
    } else {
        return -1;
    }
}

qint64 Partition::getSector()
{
    return m_sectorStart + (m_sectorEnd - m_sectorStart) / 2;
}

qint64 Partition::getByteLength()
{
    const qint64 sectors = getSectorLenth();
    if(sectors >= 0) {
        m_total = sectors * m_sectorSize;
        return m_total;
    } else {
        return -1;
    }
}

void Partition::changeNumber(int partitinNum)//更m_partitionPath新值
{
    m_num = partitinNum;
    QRegularExpression reg("^[0-9]*$");
    if(reg.match(m_devPath.at(m_devPath.length()-1)).hasMatch()) {
        m_partitionPath = QString("%1p%2").arg(m_devPath).arg(partitinNum);
        return;
    }
    if(this->m_devPath=="lvm-vg"){
        m_partitionPath = QString("%1%2").arg("lvm-lv").arg(partitinNum);
        return;
    }

    m_partitionPath = QString("%1%2").arg(this->m_devPath).arg(partitinNum);
}

QString Partition::getUsedSpace()
{
    QString result = GetBiByateValue(QString("%1").arg(getByteLength() - m_freespace));
//    QString result = GetBiByateValue(getByteLength() - m_freespace);
    return (QString)result;
}

QString Partition::getTotalSpace()
{
    QString tspace = GetBiByateValue(QString("%1").arg(getByteLength()));
//    QString tspace = GetBiByateValue(getByteLength());
    return (QString)tspace;
}


int getPartitionIndex(PartitionList &partList, Partition::Ptr pPart)//返回partList分区列表中，哪个分区包含了实参pPart分区
{
    auto find_it = std::find_if(partList.begin(), partList.end(), [=](Partition::Ptr part) {
        return (pPart->m_sectorStart >= part->m_sectorStart &&
                pPart->m_sectorEnd - 1 <= part->m_sectorEnd &&
                pPart->m_type == part->m_type);
    });
    if(find_it == partList.end()) {
        return -1;
    }
    return find_it - partList.begin();
}

int getExtendedPartitionIndex(const PartitionList &partList)//返回实参中的第一个扩展分区的下标
{
    for(int i = 0; i < partList.length(); i++) {
        if(partList.at(i)->m_type == PartitionType::Extended) {
            return i;
        }
    }
    return -1;
}

PartitionList getPrimaryPartitions(const PartitionList &partitions)//返回实参中的主分区列表
{
    PartitionList result = {};
    for(const Partition::Ptr partition : partitions) {
        if(partition->m_type == PartitionType::Normal ||
                partition->m_type == PartitionType::Extended) {
            result.append(partition);
        }
    }
    return (PartitionList)result;
}

PartitionList getLogicalPartitions(const PartitionList &partitions)//返回实参中的逻辑分区列表
{
    PartitionList result = {};
    for(const Partition::Ptr partition : partitions) {
        if(partition->m_type == PartitionType::Logical) {
            result.append(partition);
        }
    }
    return (PartitionList)result;
}

bool isPartitionsJoint(const Partition::Ptr part1, const Partition::Ptr part2)//如果两个分区相互重叠，就返回true
{
    return ((part1->m_sectorStart <= part2->m_sectorStart &&
             part2->m_sectorStart <= part1->m_sectorEnd) ||
            (part1->m_sectorStart <= part2->m_sectorEnd &&
             part2->m_sectorEnd <= part1->m_sectorEnd));
}



}
}
