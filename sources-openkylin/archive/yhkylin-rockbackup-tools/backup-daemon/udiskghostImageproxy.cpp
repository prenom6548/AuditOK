/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/

#include "udiskghostImageproxy.h"
#include <QStorageInfo>
#include <QFileInfo>
#include <QDateTime>
#include <QTimer>
#include <QDebug>

#include <unistd.h>
#include "../common/utils.h"
#include "../common/mydusizetool.h"
#include "mymountproxy.h"
#include "myprocess/calcbackupsize.h"

IMPLEMENT_DYNCREATE(UDiskGhostImageProxy)

UDiskGhostImageProxy::UDiskGhostImageProxy()
{
    m_mksquashfs = nullptr;
    m_p = nullptr;
    m_bSuccess = false;
    m_isFinished = false;
    m_isForce = false;

    connect(this, &UDiskGhostImageProxy::cancel, this, &UDiskGhostImageProxy::cancelEx);
}

UDiskGhostImageProxy::~UDiskGhostImageProxy()
{
    if (!m_kyimg.isEmpty()) {
        QFile kyimg(m_kyimg);
        if (kyimg.exists())
            kyimg.remove();
    }
}

/**
 * @brief 环境检测
 */
bool UDiskGhostImageProxy::checkEnvEx()
{
    qDebug() << "UDiskGhostImageProxy::checkEnvEx invoke begin";

    // 1、检查/backup分区是否挂载上(不管是本地磁盘还是u盘设备，都得保证/backup挂载上); 若没挂载，挂载
    MyMountProxy mountProxy;
    MountResult result = mountProxy.mountBackupPartition();
    // 无备份分区
    if (MountResult::CANNOT_GET_BACKUPUUID == result) {
        qInfo() << "There is no backup partition!";

        QString snapshotsPath = Utils::getSysRootPath() + BACKUP_SNAPSHOTS_PATH;
        snapshotsPath.replace("//", "/");
        Utils::mkpath(snapshotsPath);
        Utils::generateExcludePathsFile();
    } else if (MountResult::MOUNTED != result) {
        emit checkResult(int(BackupResult::BACKUP_PARTITION_MOUNT_FAIL));
        return false;
    }

    // 2、校验backuppoint.xml中相应的备份节点是否存在
    QString xmlPath = Utils::getSysRootPath() + BACKUP_XML_PATH;
    xmlPath.replace("//", "/");
    ParseBackupList xmlParse(xmlPath);
    ParseBackupList::BackupPoint backupPoint = xmlParse.findBackupPointByUuid(m_backupWrapper.m_uuid);
    if (backupPoint.m_backupName.isEmpty()) {
        emit checkResult(int(BackupResult::GHOST_CANNOT_FIND_BACKUPPOINT));
        return false;
    }

    // 3、校验备份数据是否存在
    QString dataPath = Utils::getSysRootPath() + BACKUP_SNAPSHOTS_PATH + "/" + m_backupWrapper.m_uuid + "/data";
    dataPath.replace("//", "/");
    if (Utils::isDirEmpty(dataPath)) {
        emit checkResult(int(BackupResult::GHOST_SRC_DIRECTORY_IS_NOT_EXIST));
        return false;
    }
    qint64 itotalSize = Utils::getDirOrFileSize(dataPath);

    // 4、校验移动设备情况：空间大小、文件格式、挂载模式等
    QString backupPath(m_backupWrapper.m_prefixDestPath);
    backupPath.replace("//", "/");
    QStorageInfo udisk(backupPath);
    QString udisk_type = udisk.fileSystemType();
    qDebug() << "udisk's filesystemtype is " << udisk_type;
    if (udisk_type == "vfat") {
        qCritical() << m_backupWrapper.m_prefixDestPath + " udisk's filesystemtype is vfat";
        emit checkResult(int(BackupResult::UDISK_FILESYSTEM_TYPE_IS_VFAT));
        return false;
    }
    if (udisk.isReadOnly()) {
        // 只读挂载的U盘
        qCritical() << QString("udisk(%s) is readonly filesystem").arg(m_backupWrapper.m_prefixDestPath);
        emit checkResult(int(BackupResult::UDISK_FILESYSTEM_IS_READONLY));
        return false;
    }

    m_destPath = m_backupWrapper.m_prefixDestPath + GHOST_PATH;
    m_destPath.replace("//", "/");
    Utils::mkpath(m_destPath);
    m_kyimg = m_destPath + "/" + m_backupWrapper.m_backupName;
    m_kyimg.replace("//", "/");
    QFile kyimg(m_kyimg);
    if (kyimg.exists())
        kyimg.remove();
    QStorageInfo storageInfo(m_destPath);
    qint64 sizeAvailable = storageInfo.bytesAvailable();
    if (sizeAvailable < itotalSize / 2) {
        emit checkResult(int(BackupResult::BACKUP_CAPACITY_IS_NOT_ENOUGH));
        return false;
    }

    // 5、依托本地存储先行压缩再拷贝到移动设备，在此查找空闲分区临时借用
    bool found = false;
    QHash<QString, qint64> hash = Utils::getAvailableSizeOfPartitions();
    for (QHash<QString, qint64>::const_iterator it = hash.constBegin(); it != hash.constEnd(); ++it) {
        QString path = it.key();
        qint64 leftSize = it.value();
        if (leftSize > itotalSize / 2) {
            Utils::mkpath(path + GHOST_PATH);
            m_kyimg = path + GHOST_PATH + "/" + m_backupWrapper.m_backupName;
            m_kyimg.replace("//", "/");
            QFile kyimg(m_kyimg);
            if (kyimg.exists())
                kyimg.remove();

            found = true;
            break ;
        }
    }
    if (!found) {
        emit checkResult(int(BackupResult::BACKUP_CAPACITY_FOR_UDISKIMG_IS_NOT_ENOUGH));
        return false;
    }

    emit checkResult(int(BackupResult::CHECK_ENV_SUCCESS));

    qDebug() << "UDiskGhostImageProxy::checkEnvEx invoke end";
    return true;
}

/**
 * @brief 任务处理
 */
void UDiskGhostImageProxy::doWorkEx()
{
    qDebug() << "UDiskGhostImageProxy::doWorkEx invoke begin";

    if (!checkEnvEx())
        return ;

    doGhostImage();

    qDebug() << "UDiskGhostImageProxy::doWorkEx invoke end";
}

/**
 * @brief 任务取消
 */
void UDiskGhostImageProxy::cancelEx()
{
    m_bCancel = true;
    if (!m_isFinished) {
        emit this->checkResult(int(BackupResult::START_CANCEL));

        if (m_mksquashfs)
            m_mksquashfs->stop();
        if (m_p)
            m_p->stop();

        QProcess::execute("sync");
        Utils::wait(5);
        deleteFailedData();
        emit this->checkResult(int(BackupResult::CANCEL_SUCCESS));
    }
}

/**
 * @brief 失败则删除相应数据
 */
void UDiskGhostImageProxy::deleteFailedData()
{
    // 1、删除临时镜像文件
    if (!m_kyimg.isEmpty()) {
        QFile kyimg(m_kyimg);
        if (kyimg.exists())
            kyimg.remove();
    }

    // 2、删除目标镜像文件
    QString kyimgFile = m_destPath + "/" + m_backupWrapper.m_backupName;
    kyimgFile.replace("//", "/");
    QFile kyimgDest(kyimgFile);
    if (kyimgDest.exists())
        kyimgDest.remove();
}

/**
 * @brief ghost镜像
 */
void UDiskGhostImageProxy::doGhostImage()
{
    qDebug() << "UDiskGhostImageProxy::doGhostImage invoke begin";

    QStringList args;
    // 拼接备份源路径和目标路径
    QString srcPath = Utils::getSysRootPath() + BACKUP_SNAPSHOTS_PATH + "/" + m_backupWrapper.m_uuid + "/data";
    srcPath.replace("//", "/");
    args << srcPath;
    args << m_kyimg;

    m_mksquashfs = new MkSquashFSProcess(this);
    connect(m_mksquashfs, &MkSquashFSProcess::progress, this, &UDiskGhostImageProxy::progress);
    connect(m_mksquashfs, &MkSquashFSProcess::finished, this,  [&](bool result) {
        qDebug() << "UDiskGhostImageProxy::finished invoke begin";

        // 如果是取消了操作，则不再发送其它信息
        if (m_bCancel)
            return ;

        if (result && !m_isForce) {
            chown(m_kyimg.toLocal8Bit().data(), m_backupWrapper.m_frontUid, m_backupWrapper.m_gid);

            // 同步到U盘
            m_p = new RsyncPathToDirProcess(this);
            connect(m_p, &RsyncPathToDirProcess::progress, this, &UDiskGhostImageProxy::progress);
            connect(m_p, &RsyncPathToDirProcess::finished, this,  [&](bool resultRsync) {
                // 如果是取消了操作，则不再发送其它信息
                if (m_bCancel)
                    return ;

                m_isForce = false;
                m_isFinished = true;
                if (resultRsync) {
//                    QFileInfo fileInfo(m_kyimg);
//                    QString imgSize = Utils::StringBySize(fileInfo.size());
//                    QString time = QDateTime::currentDateTime().toString("yy-MM-dd hh:mm:ss");
//                    Utils::writeBackupLog(time + ","
//                                          + m_backupWrapper.m_uuid + "," + QString::number(m_backupWrapper.m_type) + ","
//                                          + m_backupWrapper.m_note + "," + imgSize
//                                          + ",," + m_backupWrapper.m_backupName);
                    m_bSuccess = true;
                }
                QFile kyimg(m_kyimg);
                if (kyimg.exists())
                    kyimg.remove();

                emit this->workResult(resultRsync);
            });

            QStringList arguments;
            arguments << "-av";
            arguments << "--info=progress2";
            arguments << m_kyimg;
            arguments << m_destPath + "/";
            m_p->start(arguments, false);
            emit this->checkResult(int(BackupResult::MKSQUASHFS_DO_SUCCESS));
            emit this->progress(0);
        } else {
            m_isFinished = true;
            emit this->workResult(false);
        }

        qDebug() << "UDiskGhostImageProxy::finished invoke end";
    });
    m_bSuccess = false;
    m_isFinished = false;
    m_mksquashfs->start(args);
    emit checkResult(int(BackupResult::GHOST_START_SUCCESS));
    QTimer::singleShot(1*1000, this, &UDiskGhostImageProxy::checkDestDirExists);

    qDebug() << "UDiskGhostImageProxy::doGhostImage invoke end";
}

/**
 * @brief 校验移动盘是否还在
 * @return: bool,存在返回true；不存在返回false
 * @author: zhaominyong
 * @since: 2021/05/24
 * @note:
 *      add by zhaominyong at 2021/05/24 for bug:54377 【备份还原】备份数据到U盘的过程中拔出U盘，备份还原工具仍然一直显示正在备份数据
 */
bool UDiskGhostImageProxy::checkDestDirExists()
{
    if (!m_isFinished)
    {
        // 拔掉U盘后，没有响应的场景（怀疑可能是某应用程序关闭引起，希望不是dbus服务关掉了）
        if (m_isForce) {
            emit this->workResult(false);
            return false;
        }

        if (Utils::isDirEmpty(m_backupWrapper.m_prefixDestPath)) {
            qCritical() << QString("dstDir %s is not exist!").arg(m_backupWrapper.m_prefixDestPath);
            if (m_mksquashfs)
                m_mksquashfs->stop();
            if (m_p)
                m_p->stop();
            // 10s钟后如果还没有退出，则强制退出
            QTimer::singleShot(10*1000, this, &UDiskGhostImageProxy::checkDestDirExists);
            m_isForce = true;
        } else {
            QTimer::singleShot(1*1000, this, &UDiskGhostImageProxy::checkDestDirExists);
        }
    }

    return true;
}
