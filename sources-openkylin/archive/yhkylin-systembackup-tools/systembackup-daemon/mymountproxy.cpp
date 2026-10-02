/*
 *  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
 *  yhkylin-systembackup-tools is licensed under GPL-2+
 *  See LICENSE file for more details.
 */

#include <QtDebug>
#include <QDir>
#include "mymountproxy.h"
#include "../common/utils.h"

MyMountProxy::MyMountProxy(QObject* parent) : QObject(parent)
{
    m_processMount = new MountBackupProcess(this);
}

MountResult MyMountProxy::mountBackupPartition()
{
    QString backupPartitionUuid = Utils::getBackupPartitionUuid();
    if (backupPartitionUuid.isEmpty()) {
        qCritical("can't open /etc/.bootinfo or can't get RECOVERY_DEV_UUID");
        return MountResult::CANNOT_GET_BACKUPUUID;
    }

    QString rootPath = Utils::getBackupRootPath();
    QString backPath = rootPath + BACKUP_PATH;
    backPath.replace("//", "/");
    Utils::mkpath(backPath);

    bool res = m_processMount->Do(backupPartitionUuid);
    if (res) {
        QString snapshotsPath = rootPath + BACKUP_SNAPSHOTS_PATH;
        snapshotsPath.replace("//", "/");
        Utils::mkpath(snapshotsPath);

        if (!Utils::generateExcludePathsFile()) {
            qCritical("mount success, but generate some important file failed!");
            return MountResult::GENERATE_IMPORT_FILE_FAIL;
        }

        return MountResult::MOUNTED;
    }

    return MountResult::NO_MOUNTED;
}

bool MyMountProxy::umountBackupPartition()
{
    return m_processMount->umountBackupPartition();
}

bool MyMountProxy::remountPartitionsForCasperMode()
{
    qDebug("MyMountProxy::remountPartitionsForCasperMode invoke begin");

    // 1. 判断是否试用模式
    QString rootPath = Utils::getBackupRootPath();
    if (rootPath != CASPER_ROOT_PATH)
        return false;
    Utils::mkpath(rootPath);

    // 2. 查询各个分区
    QHash<QString, PartitionInfo> partitions = Utils::getPartitions();

    // 2.1 重新挂载根分区到/target上
    const PartitionInfo& root = partitions.value(LABEL_SYSROOT);
    m_processMount->umount(root.m_device);
//    m_processMount->umount(rootPath);
    if (!m_processMount->mount(root.m_device, rootPath)) {
        qCritical("remount root to /target failed!");
        return false;
    }

    // 2.2 获取/etc/fstab配置的分区挂载点信息
    QString fstabPath = Utils::getBackupRootPath() + FSTAB_PATH;
    fstabPath.replace("//", "/");
    QHash<QString, QString> mountPointToUuid = Utils::getPartUuidMap(fstabPath);
    QHash<QString, QString> uuidToMountPoint = Utils::swapKeyValue(mountPointToUuid);

    // 2.3 重新将其它分区挂载到/target下的相应目录
    QStringList parts;
    parts << LABEL_SYSBOOT << LABEL_EFI << LABEL_DATA << LABEL_KYLINBACKUP;
    for (const QString& label : parts) {
        const PartitionInfo& part = partitions.value(label);
        m_processMount->umount(part.m_device);

        // 备份分区不用挂载，后面会挂
        if (part.m_label == LABEL_KYLINBACKUP)
            continue;
        QString mountPoint = uuidToMountPoint.value(part.m_uuid);
        if (mountPoint.isEmpty()) {
            if (part.m_label == LABEL_SYSBOOT)
                mountPoint = "/boot";
            else if (part.m_label == LABEL_EFI)
                mountPoint = "/boot/efi";
            else if (part.m_label == LABEL_DATA)
                mountPoint = "/data";
        }

        if (mountPoint.isEmpty())
            continue;
        mountPoint = rootPath + mountPoint;
        mountPoint.replace("//", "/");
        Utils::mkpath(mountPoint);
        m_processMount->umount(mountPoint);
        m_processMount->mount(part.m_device, mountPoint);
    }

    // 3. 将/etc/fstab里的bind配置挂载上
    QHash<QString, QString> mapBind = Utils::getFstabBindInfo();
    for (QHash<QString, QString>::const_iterator it = mapBind.constBegin();  it != mapBind.constEnd(); ++it) {
        QString key = it.key();
        QString value = it.value();
        key = rootPath + key;
        key.replace("//", "/");
        value = rootPath + value;
        value.replace("//", "/");
        m_processMount->umount(value);
        m_processMount->mount(key, value);
    }

    qDebug("MyMountProxy::remountPartitionsForCasperMode invoke end");
    return true;
}

