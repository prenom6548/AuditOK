/*
 *  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
 *  yhkylin-systembackup-tools is licensed under GPL-2+
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
#include <QRegularExpression>
#include <QJsonObject>
#include <QJsonDocument>

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
/*
    // 1、检查/backup分区是否挂载上(不管是本地磁盘还是u盘设备，都得保证/backup挂载上); 若没挂载，挂载
    MyMountProxy mountProxy;
    if ( MountResult::MOUNTED != mountProxy.mountBackupPartition() ) {
        emit checkResult(int(BackupResult::BACKUP_PARTITION_MOUNT_FAIL));
        return false;
    }

    // 2、校验backuppoint.xml中相应的备份节点是否存在
    QString xmlPath = Utils::getBackupRootPath() + BACKUP_XML_PATH;
    xmlPath.replace("//", "/");
    ParseBackupList xmlParse(xmlPath);
    ParseBackupList::BackupPoint backupPoint = xmlParse.findBackupPointByUuid(m_backupWrapper.m_uuid);
    if (backupPoint.m_backupName.isEmpty()) {
        emit checkResult(int(BackupResult::GHOST_CANNOT_FIND_BACKUPPOINT));
        return false;
    }

    // 3、校验备份数据是否存在
    QString dataPath = Utils::getBackupRootPath() + BACKUP_SNAPSHOTS_PATH + "/" + m_backupWrapper.m_uuid + "/data";
    dataPath.replace("//", "/");
    if (Utils::isDirEmpty(dataPath)) {
        emit checkResult(int(BackupResult::GHOST_SRC_DIRECTORY_IS_NOT_EXIST));
        return false;
    }
    qint64 itotalSize = Utils::getDirOrFileSize(dataPath);
*/

    qDebug()<<"dest path "<< m_backupWrapper.m_prefixDestPath;

    //check if directory exist
    bool bExist = Utils::isDirExists(m_backupWrapper.m_prefixDestPath);
    if(!bExist)
    {
        if (!Utils::mkpath(m_backupWrapper.m_prefixDestPath))
        {
            qCritical() << QString("mkdir %1 failed !").arg(m_backupWrapper.m_prefixDestPath) ;
            emit checkResult(int(BackupResult::BACKUP_CURRENT_DIR_IS_NOT_EXIST));
            return false;
        }
    }
    //
    m_kyimg = m_backupWrapper.m_prefixDestPath + "/" + m_backupWrapper.m_backupName;
    m_kyimg.replace("//", "/");
    QFile kyimg(m_kyimg);
    if (kyimg.exists())
        kyimg.remove();

    // 校验空间大小是否充足
    qint64 totalSize = getTotalDiskSpace();
    // mksquashfs means size = size *0.6
    totalSize = totalSize*0.6;
    qDebug() << "Total disk space (excluding /data/.backup):" << totalSize << "bytes";

    //left size
    qint64 leftsize = Utils::getLeftSize(m_backupWrapper.m_prefixDestPath);

    qDebug()<<"ghost left size "<< leftsize;
    if (leftsize < totalSize) {
        emit checkResult(int(BackupResult::BACKUP_CAPACITY_IS_NOT_ENOUGH));
        return false;
    }

    /*
     * 如果是U盘，判断是否是vfat或udf类型，如果是，返回错误
    */
    if(m_backupWrapper.m_iPosition == 1)
    {
        if(Utils::isVFatFilesystem(m_backupWrapper.m_prefixDestPath))
        {
            emit checkResult(int(BackupResult::UDISK_FILESYSTEM_TYPE_IS_VFAT));
            return false;
        }
        if(Utils::isUDFFilesystem(m_backupWrapper.m_prefixDestPath))
        {
            emit checkResult(int(BackupResult::UDISK_FILESYSTEM_TYPE_IS_UDF));
            return false;
        }
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

    /*
    if (!checkEnvEx())
        return ;
    */
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

        if(!unmountDevices()){
            qCritical() << "Failed to unmount devices";
            emit this->workResult(false);
            return;
        }
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
    QString srcPath = "/tmp/ghost";
    m_kyimg = m_backupWrapper.m_prefixDestPath + "/" + m_backupWrapper.m_backupName;
    m_kyimg.replace("//", "/");
    qDebug()<<"ghost dest path name" << m_kyimg;
    args << srcPath;
    args << m_kyimg;
    args << "-e" << "/tmp/ghost/data/.backup";

    m_mksquashfs = new MkSquashFSProcess(this);
    connect(m_mksquashfs, &MkSquashFSProcess::progress, this, &GhostImageProxy::progress);
    connect(m_mksquashfs, &MkSquashFSProcess::finished, this,  [&](bool result) {
        qDebug() << "GhostImageProxy::finished invoke begin";

        // 如果是取消了操作，则不再发送其它信息
        if (m_bCancel)
            return ;

        m_isFinished = true;
        if (result) {

            QString ghostPath = m_backupWrapper.m_prefixDestPath + "/";
            qDebug() << "更改镜像属性：" << ghostPath;
            chown(ghostPath.toLocal8Bit().data(), m_backupWrapper.m_frontUid, m_backupWrapper.m_gid);
//            QFileInfo fileInfo(m_kyimg);
//            QString imgSize = Utils::StringBySize(fileInfo.size());
//            QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
//            Utils::writeBackupLog(time + ","
//                                  + m_backupWrapper.m_uuid + "," + QString::number(m_backupWrapper.m_type) + ","
//                                  + m_backupWrapper.m_note + "," + imgSize
//                                  + ",," + m_backupWrapper.m_backupName);
            m_bSuccess = true;
        }

        if(!unmountDevices()){
            qCritical() << "Failed to unmount devices";
            emit this->workResult(false);
            return;
        }
        emit this->workResult(result);

        qDebug() << "GhostImageProxy::finished invoke end";
    });
    m_bSuccess = false;

    createTempDirs();
    if (!mountDevices()) {
        qCritical() << "Failed to mount devices";
        emit this->workResult(false);
        return;
    }
    QString xmlPath = "/tmp/ghost/system-info.json";
    createSystemInfoJson(xmlPath);
    m_mksquashfs->start(args);
/*
    if(!unmountDevices()){
        qCritical() << "Failed to unmount devices";
        emit this->workResult(false);
        return;
    }
*/
    qDebug() << " doGhostImage successfully end";
}

// 计算总大小（/sysroot + /boot + /data，排除 /data/.backup）
qint64 GhostImageProxy::getTotalDiskSpace()
{
    qint64 total = 0;

    // 获取 /sysroot

    qint64 dataSize = Utils::getDirectorySize("/sysroot");
    if (dataSize > 0) {
        total += dataSize;
    } else {
        qWarning() << "Failed to calculate /sysroot size";
    }
    qDebug()<<"sysroot size "<< total;

    dataSize = Utils::getDirectorySize("/boot");
    if (dataSize > 0) {
        total += dataSize;
    } else {
        qWarning() << "Failed to calculate /boot size";
    }

    qDebug()<<"boot size "<< total;
    // 计算 /data 目录大小（排除 /data/.backup）
    dataSize = Utils::getDirectorySize("/data", ".backup");
    if (dataSize > 0) {
        total += dataSize;
    } else {
        qWarning() << "Failed to calculate /data size";
    }
    qDebug()<<"total size "<< total;

    return total;
}

void GhostImageProxy::createTempDirs()
{
    QDir dir;
    dir.mkpath("/tmp/ghost/sysroot");
    dir.mkpath("/tmp/ghost/boot/efi");
    dir.mkpath("/tmp/ghost/data");
}

bool GhostImageProxy::mountDevices()
{
    if (!parseFstab()) return false;

    for (const auto &mountPoint : mountPoints.keys()) {
        const MountInfo &info = mountPoints[mountPoint];
        QString targetPath = "/tmp/ghost" + mountPoint;
        if (!mountDevice(info.device, targetPath, info.fsType)) {
            return false;
        }
    }
    return true;
}

bool GhostImageProxy::parseFstab()
{
    QFile fstab("/etc/fstab");
    if (!fstab.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCritical() << "Failed to open /etc/fstab";
        return false;
    }

    QTextStream in(&fstab);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#')) continue;

        QStringList parts = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);
        if (parts.size() < 3) continue;

        QString device, mountPoint = parts[1], fsType = parts[2];

        // 跳过swap backup和无效条目
        if (fsType == "swap" || mountPoint == "none" || mountPoint.contains("backup")) continue;

        // 统一通过UUID或LABEL获取设备路径
        if (parts[0].startsWith("UUID=")) {
            qDebug()<<"uuid is "<< parts[0].mid(5);
            device = getDeviceByUuid(parts[0].mid(5));
        } else{
            continue;
        }

        if (device.isEmpty()) {
            qWarning() << "Cannot find device for:" << parts[0];
            continue;
        }

        if(mountPoint=="/"){
            mountPoint="/sysroot";
        }
        qDebug()<<"device ##" << device << " type "<< fsType << " mountpoint "<< mountPoint;
        mountPoints[mountPoint] = {device, mountPoint, fsType};
    }
    return !mountPoints.isEmpty();
}

QString GhostImageProxy::getDeviceByUuid(const QString &uuid)
{
    qDebug()<<"getDevice function ,uuid ####"<< uuid;
    QProcess blkid;
    blkid.setProgram("/sbin/blkid");
    QStringList args={
        "-U",
       uuid
    };
    blkid.setArguments(args);
    blkid.start();

    if (!blkid.waitForFinished(-1)) {
        // 进程超时或出错
        qDebug() << "blkid failed or timed out " << blkid.errorString();
        return "";

    }
    QString output = QString(blkid.readAllStandardOutput()).trimmed();
    qDebug() << "blkid output:" << output;
    return output;

   // return blkid.waitForFinished() ? QString(blkid.readAllStandardOutput()).trimmed() : "";
}

bool GhostImageProxy::mountDevice(const QString &device, const QString &mountPoint, const QString &fsType) {
    QProcess mount;
    mount.start("mount", {device, mountPoint});
    if (!mount.waitForFinished(-1) || mount.exitCode() != 0) {
        qCritical() << "Mount failed:" << device << "->" << mountPoint << "Error:" << mount.readAllStandardError();
        return false;
    }
    qDebug() << "Mounted" << device << "to" << mountPoint;
    return true;
}


bool GhostImageProxy::unmountDevices()
{
    QProcess umount;
    umount.start("sh", {"-c", "umount -l /tmp/ghost/*"});
    if (!umount.waitForFinished() || umount.exitCode() != 0) {
        qWarning() << "Unmount failed, Error:" << umount.readAllStandardError();
        return true;
    } else {
        qDebug() << "Unmounted successfully";
     //   QDir("/tmp/ghost").removeRecursively();
        return true;
    }
}


void GhostImageProxy::createSystemInfoJson(const QString &outputPath)
{
    QJsonObject json;

    QString result;
    Utils::executeCMD("uname -m", result);
    QString arch = result.trimmed();
    QString version = getKylinVersion();

    qDebug()<<"arch "<< arch << " version "<< version;
    json["arch"] = arch;
    json["version"] = version;
    json["time"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    QFile file(outputPath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        file.write(QJsonDocument(json).toJson());
        file.close();
    }
}

QString GhostImageProxy::getKylinVersion()
{
    QFile file("/etc/kylin-build");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open /etc/kylin-build";
        return "";
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        QRegularExpression regex("Kylin-Desktop\\s+(V\\d+)"); // 匹配 "Kylin-Desktop V11"
        QRegularExpressionMatch match = regex.match(line);
        if (match.hasMatch()) {
            return match.captured(1); // 返回 "V11"
        }
    }

    return ""; // 未找到版本号
}
