/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/

#include "udiskdetector.h"
#include <QFileSystemWatcher>
#include <QTimer>

#include "../../common/utils.h"

UdiskDetector::UdiskDetector(QObject* parent)
    : QObject(parent)
{
    m_udiskList.clear();
    diskRefreshDelay = new QTimer;
    diskRefreshDelay->setObjectName("yhkylin-backup-tools_kybackup_QTimer_diskRefreshDelay");
    connect(diskRefreshDelay, &QTimer::timeout, this, &UdiskDetector::getStorageInfo);
    udiskplugwatcher = new QFileSystemWatcher(this);
    udiskplugwatcher->setObjectName("yhkylin-backup-tools_kybackup_QFileSystemWatcher_udiskplugwatcher");
    udiskplugwatcher->addPath("/dev");
    connect(udiskplugwatcher, &QFileSystemWatcher::directoryChanged, this, &UdiskDetector::refreshDiskList);
}

void UdiskDetector::getStorageInfo()
{
    m_udiskList.clear();
    diskRefreshDelay->stop(); //U盘动态刷新相关
    QHash<QString, QString> removableStorages = Utils::getRemovableStorages();
    QList<QStorageInfo> diskList = QStorageInfo::mountedVolumes(); //获取磁盘列表
    for (QStorageInfo& disk : diskList) {
        if ("" == disk.displayName()) //名称为空的磁盘不显示
            continue;
        if ("/" == disk.displayName()) //系统分区不显示
            continue;
        if ("tmpfs" == disk.fileSystemType()) //tmpfs类型的磁盘不显示
            continue;
        if ("/boot" == disk.displayName()) //boot分区不显示
            continue;
        if (disk.displayName().contains("BACKUP")) //还原分区不显示
            continue;
        if (disk.device().contains("/dev/sr0")) //光盘不显示
            continue;
        if (disk.device().contains("/dev/sda0")) //内置硬盘不显示
            continue;
        if (disk.device().contains("/dev/nvm")) //nvme类型的设备不显示
            continue;
        if (!disk.rootPath().contains("/media"))
            continue;
        if (disk.rootPath().contains("loop"))
            continue;
        if (removableStorages.contains(disk.device()))
                    m_udiskList.append(disk);
    }

    emit udiskListChanged(m_udiskList);
}

void UdiskDetector::setDiskList(QList<QStorageInfo> udiskList)
{
    m_udiskList.clear();
    m_udiskList.append(udiskList);
    emit udiskListChanged(m_udiskList);
}

void UdiskDetector::refreshDiskList()
{
    if (diskRefreshDelay->isActive()) {
        return;
    }

    m_udiskList.clear();
    diskRefreshDelay->start(1000);
}
