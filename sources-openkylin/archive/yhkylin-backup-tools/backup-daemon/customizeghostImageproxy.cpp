/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/

#include "customizeghostImageproxy.h"
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

IMPLEMENT_DYNCREATE(CustomizeGhostImageProxy)

CustomizeGhostImageProxy::CustomizeGhostImageProxy()
{
    m_p = nullptr;
    m_bSuccess = false;
    m_isFinished = false;
    m_isForce = false;

    connect(this, &CustomizeGhostImageProxy::cancel, this, &CustomizeGhostImageProxy::cancelEx);
}

CustomizeGhostImageProxy::~CustomizeGhostImageProxy()
{
    // 不成功则删除中间文件
    if (!m_bSuccess) {
        deleteFailedData();
    }
}

/**
 * @brief 环境检测
 */
bool CustomizeGhostImageProxy::checkEnvEx()
{
    qDebug() << "CustomizeGhostImageProxy::checkEnvEx invoke begin";

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
    m_backupPoint = xmlParse.findBackupPointByUuid(m_backupWrapper.m_uuid);
    if (m_backupPoint.m_backupName.isEmpty()) {
        emit checkResult(int(BackupResult::GHOST_CANNOT_FIND_BACKUPPOINT));
        return false;
    }

    // 3、校验备份数据是否存在
    m_srcPath = m_backupPoint.m_path + BACKUP_SNAPSHOTS_PATH + "/" + m_backupWrapper.m_uuid + "/data";
    m_srcPath.replace("//", "/");
    if (Utils::isDirEmpty(m_srcPath)) {
        emit checkResult(int(BackupResult::GHOST_SRC_DIRECTORY_IS_NOT_EXIST));
        return false;
    }
    m_imgDst = m_srcPath + "/" + UDISK_MKSQUASHFS_IMG_NAME;
    if (!Utils::fileExists(m_imgDst)) {
        emit checkResult(int(BackupResult::GHOST_SRC_DIRECTORY_IS_NOT_EXIST));
        return false;
    }

    // 4、校验空间是否充足
    qint64 itotalSize = Utils::getDirOrFileSize(m_srcPath);
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
    if (sizeAvailable <= itotalSize) {
        emit checkResult(int(BackupResult::BACKUP_CAPACITY_IS_NOT_ENOUGH));
        return false;
    }

    emit checkResult(int(BackupResult::CHECK_ENV_SUCCESS));

    qDebug() << "CustomizeGhostImageProxy::checkEnvEx invoke end";
    return true;
}

/**
 * @brief 任务处理
 */
void CustomizeGhostImageProxy::doWorkEx()
{
    qDebug() << "CustomizeGhostImageProxy::doWorkEx invoke begin";

    if (!checkEnvEx())
        return ;

    doGhostImage();

    qDebug() << "CustomizeGhostImageProxy::doWorkEx invoke end";
}

/**
 * @brief 任务取消
 */
void CustomizeGhostImageProxy::cancelEx()
{
    qDebug() << "CustomizeGhostImageProxy::cancelEx invoke begin";

    m_bCancel = true;
    if (!m_isFinished) {
        emit this->checkResult(int(BackupResult::START_CANCEL));

        if (m_p)
            m_p->stop();

        QProcess::execute("sync");
        Utils::wait(5);
        deleteFailedData();
        emit this->checkResult(int(BackupResult::CANCEL_SUCCESS));
    }

    qDebug() << "CustomizeGhostImageProxy::cancelEx invoke end";
}

/**
 * @brief 失败则删除相应数据
 */
void CustomizeGhostImageProxy::deleteFailedData()
{
    // 1、删除临时镜像文件
    QFile kyimg(m_destPath + "/" + UDISK_MKSQUASHFS_IMG_NAME);
    if (kyimg.exists())
        kyimg.remove();

    // 2、删除目标镜像文件
    if (!m_kyimg.isEmpty()) {
        QFile kyimgDest(m_kyimg);
        if (kyimgDest.exists())
            kyimgDest.remove();
    }
}

/**
 * @brief ghost镜像
 */
void CustomizeGhostImageProxy::doGhostImage()
{
    qDebug() << "CustomizeGhostImageProxy::doGhostImage invoke begin";

    // 同步到U盘
    m_p = new RsyncPathToDirProcess(this);
    connect(m_p, &RsyncPathToDirProcess::progress, this, &CustomizeGhostImageProxy::progress);
    connect(m_p, &RsyncPathToDirProcess::finished, this,  [&](bool resultRsync) {
        // 如果是取消了操作，则不再发送其它信息
        if (m_bCancel)
            return ;

        m_isForce = false;
        m_isFinished = true;
        if (resultRsync) {
            // 文件更名
            QString imgFileName = m_destPath + "/" + UDISK_MKSQUASHFS_IMG_NAME;
            if (Utils::fileExists(imgFileName)) {
                QStringList args;
                args << imgFileName;
                args << m_kyimg;
                QProcess::execute("mv", args);

                chown(m_kyimg.toLocal8Bit().data(), m_backupWrapper.m_frontUid, m_backupWrapper.m_gid);
                m_bSuccess = true;
                resultRsync = true;
            } else {
                resultRsync = false;
            }
        }

        emit this->workResult(resultRsync);
    });

    QStringList arguments;
    arguments << "-av";
    arguments << "--info=progress2";
    arguments << m_imgDst;
    arguments << m_destPath + "/";

    QTimer::singleShot(1*1000, this, &CustomizeGhostImageProxy::checkDestDirExists);
    m_p->start(arguments, false);

    qDebug() << "CustomizeGhostImageProxy::doGhostImage invoke end";
}

/**
 * @brief 校验移动盘是否还在
 * @return: bool,存在返回true；不存在返回false
 * @author: zhaominyong
 * @since: 2021/05/24
 * @note:
 *      add by zhaominyong at 2021/05/24 for bug:54377 【备份还原】备份数据到U盘的过程中拔出U盘，备份还原工具仍然一直显示正在备份数据
 */
bool CustomizeGhostImageProxy::checkDestDirExists()
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
            if (m_p)
                m_p->stop();
            // 10s钟后如果还没有退出，则强制退出
            QTimer::singleShot(10*1000, this, &CustomizeGhostImageProxy::checkDestDirExists);
            m_isForce = true;
        } else {
            QTimer::singleShot(1*1000, this, &CustomizeGhostImageProxy::checkDestDirExists);
        }
    }

    return true;
}

