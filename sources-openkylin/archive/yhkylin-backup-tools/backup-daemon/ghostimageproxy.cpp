/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/

#include "ghostimageproxy.h"
#include <QStorageInfo>
#include <QFileInfo>
#include <QDateTime>
#include <QDebug>
#include <unistd.h>
#include "../common/utils.h"
#include "../common/mydusizetool.h"
#include "mymountproxy.h"
#include "myprocess/calcbackupsize.h"

IMPLEMENT_DYNCREATE(GhostImageProxy)

GhostImageProxy::GhostImageProxy()
{
    m_mksquashfs = nullptr;
    m_bSuccess = false;
    m_isFinished = false;

    connect(this, &GhostImageProxy::cancel, this, &GhostImageProxy::cancelEx);
}

GhostImageProxy::~GhostImageProxy()
{
    delete m_mksquashfs;
    m_mksquashfs = nullptr;
}

/**
 * @brief 环境检测
 */
bool GhostImageProxy::checkEnvEx()
{
    qDebug() << "GhostImageProxy::checkEnvEx invoke begin";

    // 1、检查/backup分区是否挂载上(不管是本地磁盘还是u盘设备，都得保证/backup挂载上); 若没挂载，挂载
    MyMountProxy mountProxy;
    if ( MountResult::MOUNTED != mountProxy.mountBackupPartition() ) {
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

    // 4、校验空间大小是否充足
    m_destPath = Utils::getSysRootPath() + GHOST_PATH;
    m_destPath.replace("//", "/");
    Utils::mkpath(m_destPath);
    m_kyimg = Utils::getSysRootPath() + GHOST_PATH + "/" + m_backupWrapper.m_backupName;
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

    emit checkResult(int(BackupResult::CHECK_ENV_SUCCESS));

    qDebug() << "GhostImageProxy::checkEnvEx invoke end";
    return true;
}

/**
 * @brief 任务处理
 */
void GhostImageProxy::doWorkEx()
{
    qDebug() << "GhostImageProxy::doWorkEx invoke begin";

    if (!checkEnvEx())
        return ;

    doGhostImage();

    qDebug() << "GhostImageProxy::doWorkEx invoke end";
}

/**
 * @brief 任务取消
 */
void GhostImageProxy::cancelEx()
{
    qDebug() << "GhostImageProxy::cancelEx invoke begin";

    m_bCancel = true;
    if (!m_isFinished) {
        emit this->checkResult(int(BackupResult::START_CANCEL));

        if (m_mksquashfs)
            m_mksquashfs->stop();

        QProcess::execute("sync");
        Utils::wait(5);
        deleteFailedData();
        emit this->checkResult(int(BackupResult::CANCEL_SUCCESS));
    }

    qDebug() << "GhostImageProxy::cancelEx invoke end";
}

/**
 * @brief 失败则删除相应数据
 */
void GhostImageProxy::deleteFailedData()
{
    // 1、删除镜像文件
    if (!m_kyimg.isEmpty()) {
        QFile kyimg(m_kyimg);
        if (kyimg.exists())
            kyimg.remove();
    }
}

/**
 * @brief ghost镜像
 */
void GhostImageProxy::doGhostImage()
{
    qDebug() << "GhostImageProxy::doGhostImage invoke begin";

    QStringList args;
    // 拼接备份源路径和目标路径
    QString srcPath = Utils::getSysRootPath() + BACKUP_SNAPSHOTS_PATH + "/" + m_backupWrapper.m_uuid + "/data";
    srcPath.replace("//", "/");
    args << srcPath;
    args << m_kyimg;

    m_mksquashfs = new MkSquashFSProcess(this);
    connect(m_mksquashfs, &MkSquashFSProcess::progress, this, &GhostImageProxy::progress);
    connect(m_mksquashfs, &MkSquashFSProcess::finished, this,  [&](bool result) {
        qDebug() << "GhostImageProxy::finished invoke begin";

        // 如果是取消了操作，则不再发送其它信息
        if (m_bCancel)
            return ;

        m_isFinished = true;
        if (result) {
            chown(m_kyimg.toLocal8Bit().data(), m_backupWrapper.m_frontUid, m_backupWrapper.m_gid);
//            QFileInfo fileInfo(m_kyimg);
//            QString imgSize = Utils::StringBySize(fileInfo.size());
//            QString time = QDateTime::currentDateTime().toString("yy-MM-dd hh:mm:ss");
//            Utils::writeBackupLog(time + ","
//                                  + m_backupWrapper.m_uuid + "," + QString::number(m_backupWrapper.m_type) + ","
//                                  + m_backupWrapper.m_note + "," + imgSize
//                                  + ",," + m_backupWrapper.m_backupName);
            m_bSuccess = true;
        }
        emit this->workResult(result);

        qDebug() << "GhostImageProxy::finished invoke end";
    });
    m_bSuccess = false;
    m_mksquashfs->start(args);

    qDebug() << "GhostImageProxy::doGhostImage invoke end";
}

