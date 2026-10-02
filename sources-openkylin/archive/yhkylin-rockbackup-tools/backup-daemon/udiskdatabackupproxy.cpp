/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/

#include "udiskdatabackupproxy.h"
#include <QStorageInfo>
#include <QDateTime>
#include <QDebug>
#include <QTimer>
#include <QRegularExpression>

#include "../common/utils.h"
#include "../common/mydusizetool.h"
#include "mymountproxy.h"
#include "myprocess/calcbackupsize.h"

IMPLEMENT_DYNCREATE(UDiskDataBackupProxy)

UDiskDataBackupProxy::UDiskDataBackupProxy()
{
    m_isForce = false;
}

UDiskDataBackupProxy::~UDiskDataBackupProxy()
{
    m_isForce = false;
}

/**
 * @brief 环境检测
 * @return false,检测失败;true,检测成功
 */
bool UDiskDataBackupProxy::checkEnvEx()
{
    qDebug() << "UDiskDataBackupProxy::checkEnv invoke begin";

    // 1、检查/backup分区是否挂载上(不管是本地磁盘还是u盘设备，都得保证/backup挂载上); 若没挂载，挂载
    // 后来支持无备份分区
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

    // 防命令注入
    // 1、形如：mkdir '`id&>id_bak_test.txt`'中的文件夹名称
    // 2、形如：$()的文件夹名称
    // 3、形如：${}的文件夹名称
    // 4、包含[;、&、|]等可以包含并执行系统命令或用于连续执行系统命令的符号
    if (    m_backupWrapper.m_prefixDestPath.contains(QRegularExpression(".*`.*`.*"))
         || m_backupWrapper.m_prefixDestPath.contains(QRegularExpression(".*\\$\\(.*\\).*"))
         || m_backupWrapper.m_prefixDestPath.contains(QRegularExpression(".*\\$\\{.*\\}.*"))
         || m_backupWrapper.m_prefixDestPath.contains(QRegularExpression("[;&|]+"))) {
        qCritical () << "The path to backup contains ``,$(),${},;&|,etc";

        emit checkResult(int(BackupResult::DU_ERR));
        return false;
    }

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

    QTimer::singleShot(1*1000, this, &UDiskDataBackupProxy::checkDestDirExists);

    // 2、判断备份是否增量备份
    isIncBackup();

    // 3、检测空间是否满足备份
    calcSizeForBackupToUdisk();

    qDebug() << "UDiskDataBackupProxy::checkEnv invoke end";
    return true;
}

/**
 * @brief 执行备份逻辑
 */
void UDiskDataBackupProxy::doWorkEx()
{
    qDebug() << "UDiskDataBackupProxy::doWorkEx invoke begin";

    m_isOnlyCheck = false;
    // 环境检测
    checkEnvEx();

    qDebug() << "UDiskDataBackupProxy::doWorkEx invoke end";
}

/**
 * @brief 取消操作
 */
void UDiskDataBackupProxy::cancelEx()
{
    qDebug() << "UDiskDataBackupProxy::cancelEx invoke begin";

    m_bCancel = true;
    if (!m_isFinished) {
        emit this->checkResult(int(BackupResult::START_CANCEL));

        if (m_calc)
            m_calc->stop();
        if (m_p)
            m_p->stop();

        QProcess::execute("sync");
        Utils::wait(5);
        deleteFailedData();
        emit this->checkResult(int(BackupResult::CANCEL_SUCCESS));
    }

    qDebug() << "UDiskDataBackupProxy::cancelEx invoke end";
}

/**
 * @brief 失败则删除相应数据
 */
void UDiskDataBackupProxy::deleteFailedData()
{
    if (m_curUuid.isEmpty())
        return;

    // 1、删除备份目录
    QString destPath = m_backupWrapper.m_prefixDestPath + BACKUP_SNAPSHOTS_PATH + "/" + m_curUuid;
    destPath.replace("//", "/");
    QStringList args;
    args << "-rf";
    args << destPath;
    QProcess::execute("rm", args);

    // 2、删除xml文件中的备份项
    QString xmlPath = m_backupWrapper.m_prefixDestPath + BACKUP_XML_PATH;
    xmlPath.replace("//", "/");
    ParseBackupList parse(xmlPath);
    parse.deleteItem(m_curUuid);
}

/**
 * @brief 判断是否增量备份
 * @return true,增量备份； false，全量备份
 */
bool UDiskDataBackupProxy::isIncBackup()
{
    QString backupPath;
    ParseBackupList::BackupPoint point;
    if (m_backupWrapper.m_uuid.isEmpty()) {
        return false;
    } else {
        backupPath = m_backupWrapper.m_prefixDestPath + BACKUP_SNAPSHOTS_PATH + "/" + m_backupWrapper.m_uuid + "/data";
    }

    backupPath.replace("//", "/");
    if (Utils::isDirExist(backupPath)) {
        m_backupWrapper.m_bIncrement = true;
        m_backupWrapper.m_type = BackupType::INC_BACKUP_DATA;
        return true;
    }
    return false;
}


/**
 * @brief 校验剩余空间是否满足备份
 */
bool UDiskDataBackupProxy::checkFreeCapacityToUdisk(qint64 itotalSize)
{
    qDebug() << "UDiskDataBackupProxy::checkFreeCapacityToUdisk invoke begin";

    // 如果是取消了操作，则不再发送其它信息
    if (m_bCancel)
        return false;

    // 拔掉U盘的场景
    if (m_isForce) {
        emit this->checkResult(int(BackupResult::WRITE_BACKUP_PATHS_TO_USER_FAILED));
        return false;
    }

    // 1、计算待备份数据的大小
    m_size = itotalSize;
    // 备份过程中会有一些临时文件产生，会占用一部分空间，故我们预留500M的空间
    itotalSize += 5 * MB;

    // 2、计算备份分区剩余空间大小
    QString backupPath(m_backupWrapper.m_prefixDestPath);
    backupPath.replace("//", "/");
    QStorageInfo backupDisk(backupPath);
    qint64 freeSize = backupDisk.bytesAvailable();

    // 3、校验空间是否足够
    bool result = true;
    if (itotalSize > freeSize) {
        emit checkResult(int(BackupResult::BACKUP_CAPACITY_IS_NOT_ENOUGH));
        result = false;
        return result;
    } else {
        emit checkResult(int(BackupResult::CHECK_ENV_SUCCESS));
    }

    if (m_isOnlyCheck)
        return result;

    // 开始备份
    doBackupToUdisk();

    qDebug() << "UDiskDataBackupProxy::checkFreeCapacityToUdisk invoke end";
    return result;
}

/**
 * @brief 计算备份所需空间大小
 * @return 计算备份所需空间大小,单位字节
 */
void UDiskDataBackupProxy::calcSizeForBackupToUdisk()
{
    QString destPath;
    QStringList args;
    if (m_backupWrapper.m_bIncrement) {
        // 在原来的备份点上增量备份场景
        args = getRsyncArgs(DataBackupScene::TRY_INC_DATA_BACKUP);
        destPath = m_backupWrapper.m_prefixDestPath;
        destPath += BACKUP_SNAPSHOTS_PATH;
        destPath += "/";
        destPath += m_backupWrapper.m_uuid;
        destPath += "/data/";
    } else {
        // 全量备份场景
        args = getRsyncArgs(DataBackupScene::TRY_DATA_BACKUP);
        destPath = Utils::getSysRootPath();
        destPath += CHECK_PATH;
    }

    // 拼接备份源路径和目标路径
    QString srcPath = Utils::getSysRootPath();
    srcPath += "/";
    srcPath.replace("//", "/");
    args << srcPath;
    destPath.replace("//", "/");
    args << destPath;
    Utils::mkpath(destPath);

    connect(m_calc, &CalcBackupSize::finished, this, &UDiskDataBackupProxy::checkFreeCapacityToUdisk);
    m_calc->start(args, false);
}

/**
 * @brief 备份
 */
void UDiskDataBackupProxy::doBackupToUdisk()
{
    qDebug() << "UDiskDataBackupProxy::doBackupToUdisk invoke begin";

    // 准备
    if (!doPrepareToUdisk())
        return ;

    // 启动数据备份
    backupDataToUdisk();

    qDebug() << "UDiskDataBackupProxy::doBackupToUdisk invoke end";
}

/**
 * @brief 备份准备
 * @return true,准备成功；false，准备失败
 */
bool UDiskDataBackupProxy::doPrepareToUdisk()
{
    qDebug() << "UDiskDataBackupProxy::doPrepareToUdisk invoke begin";

    m_bSuccess = false;

    // 1、设置当前备份的Uuid
    if (m_backupWrapper.m_uuid.isEmpty()) {
        // 新增备份点，不指定uuid的场景
        m_curUuid += Utils::createUuid();
    } else {
        // 指定uuid备份的场景
        m_curUuid = m_backupWrapper.m_uuid;
    }

    // 2、准备备份目录及文件
    m_destPath = m_backupWrapper.m_prefixDestPath + BACKUP_SNAPSHOTS_PATH + "/" + m_curUuid + "/data";
    m_destPath.replace("//", "/");
    if (!Utils::mkpath(m_destPath)) {
        emit checkResult(int(BackupResult::WRITE_BACKUP_PATHS_TO_USER_FAILED));
        qCritical() << QString("mkdir %1 failed !").arg(m_destPath) ;
        return false;
    }

    m_userFile = m_backupWrapper.m_prefixDestPath + BACKUP_SNAPSHOTS_PATH + "/" + m_curUuid + "/" + PATHS_USER_FILE;
    m_userFile.replace("//", "/");
    if (!Utils::writeFileByLines(m_userFile, m_backupWrapper.m_backupPaths)) {
        emit checkResult(int(BackupResult::WRITE_BACKUP_PATHS_TO_USER_FAILED));
        qCritical() << QString("create file %1 failed !").arg(m_userFile) ;
        return false;
    }

    m_excludeUserFile = m_backupWrapper.m_prefixDestPath + BACKUP_SNAPSHOTS_PATH + "/" + m_curUuid + "/" + EXCLUDE_PATHS_USER_FILE;
    m_excludeUserFile.replace("//", "/");
    if (!Utils::writeFileByLines(m_excludeUserFile, m_backupWrapper.m_backupExcludePaths)) {
        emit checkResult(int(BackupResult::WRITE_BACKUP_PATHS_TO_USER_FAILED));
        qCritical() << QString("create file %1 failed !").arg(m_excludeUserFile) ;
        return false;
    }

    // 3、记录/backup/snapshots/backuplist.xml文件
    if (!recordBackupPointToUdisk()) {
        qCritical() << "add or update item to backuplist.xml failed !";
        return false;
    }

    qDebug() << "UDiskDataBackupProxy::doPrepareToUdisk invoke end";
    return true;
}

/**
 * @brief 记录/backup/snapshots/backuplist.xml文件
 * @return true-成功；false-失败
 */
bool UDiskDataBackupProxy::recordBackupPointToUdisk()
{
    m_backupPoint.m_backupName = m_backupWrapper.m_backupName;
    m_backupPoint.m_uuid = m_curUuid;
    m_backupPoint.m_iPosition = m_backupWrapper.m_iPosition;
    m_backupPoint.m_type = m_backupWrapper.m_type;
    m_backupPoint.m_size = Utils::StringBySize(m_size);
    m_backupPoint.m_time = QDateTime::currentDateTime().toString("yy-MM-dd hh:mm:ss");
    m_backupPoint.m_state = BACKUP_PARSE_STATE_FAIL_STRTING;
    if (0 < m_backupWrapper.m_frontUid)
        m_backupPoint.m_userId = QString::number(m_backupWrapper.m_frontUid);
    m_backupPoint.m_os = SystemInfo::m_os;
    m_backupPoint.m_arch = SystemInfo::m_arch;
    m_backupPoint.m_archdetect = SystemInfo::m_archDetect;
    QString xmlPath = m_backupWrapper.m_prefixDestPath + BACKUP_XML_PATH;
    xmlPath.replace("//", "/");
    ParseBackupList parse(xmlPath);
    if (!m_backupWrapper.m_bIncrement) {
        if (parse.addItem(m_backupPoint) != ParseBackupList::SUCCESS) {
            emit checkResult(int(BackupResult::WRITE_STORAGEINFO_ADD_ITEM_FAIL));
            return false;
        }
    } else {
        if (parse.updateItem(m_backupPoint) != ParseBackupList::SUCCESS) {
            emit checkResult(int(BackupResult::WRITE_STORAGEINFO_UPDATE_ITEM_FAIL));
            return false;
        }
    }

    return true;
}

/**
 * @brief 备份系统
 * @return true，备份成功；false，备份失败
 */
bool UDiskDataBackupProxy::backupDataToUdisk()
{
    qDebug() << "UDiskDataBackupProxy::backupDataToUdisk invoke begin";

    QStringList args;
    if (m_backupWrapper.m_bIncrement) {
        // 在原来的备份点上增量备份场景
        args = getRsyncArgs(DataBackupScene::INC_DATA_BACKUP);
    } else {
        // 全量备份场景
        args = getRsyncArgs(DataBackupScene::DATA_BACKUP);
    }

    // 拼接备份源路径和目标路径
    QString srcPath = Utils::getSysRootPath();
    srcPath += "/";
    srcPath.replace("//", "/");
    args << srcPath;
    QString destPath = m_destPath + "/";
    destPath.replace("//", "/");
    args << destPath;

    m_p = new RsyncPathToDirProcess(this);
    connect(m_p, &RsyncPathToDirProcess::progress, this, &UDiskDataBackupProxy::progress);
    connect(m_p, &RsyncPathToDirProcess::finished, this,  [&](bool result) {
        // 如果是取消了操作，则不再发送其它信息
        if (m_bCancel)
            return ;

        m_isForce = false;
        m_isFinished = true;
        if (result) {
            m_backupPoint.m_state = BACKUP_PARSE_STATE_SUCCESS_STRTING;
            m_backupPoint.m_size = Utils::StringBySize(Utils::getDirOrFileSize(m_destPath));
            QString xmlPath = m_backupWrapper.m_prefixDestPath + BACKUP_XML_PATH;
            xmlPath.replace("//", "/");
            ParseBackupList parse(xmlPath);
            if (ParseBackupList::ParseResult::SUCCESS != parse.updateItem(m_backupPoint)) {
                qCritical() << "update backuplist.xml error in sendBackupResult";
                result = false;
            } else {
                // Utils::writeBackupLog(time + "," + m_curUuid + "," + QString::number(m_backupWrapper.m_type) + ","+ m_backupWrapper.m_note + "," + m_backupPoint.m_size+ "," + QString::number(m_backupWrapper.m_frontUid));
                Utils::writeBackupLog(m_backupPoint.m_time + ","
                                      + m_curUuid + "," + QString::number(m_backupWrapper.m_type) + ","
                                      + m_backupWrapper.m_note + "," + m_backupPoint.m_size
                                      + "," + QString::number(m_backupWrapper.m_frontUid)
                                      + "," + m_backupWrapper.m_backupName);
                Utils::update_backup_unique_settings(m_curUuid, m_backupPoint.m_backupName);
                m_bSuccess = true;
            }
        }
        emit this->workResult(result);
    });
    m_p->start(args, false);

    do_kylin_security(m_destPath);

    qDebug() << "UDiskDataBackupProxy::backupDataToUdisk invoke end";
    return true;
}

/**
 * @brief 校验移动盘是否还在
 * @return: bool,存在返回true；不存在返回false
 * @author: zhaominyong
 * @since: 2021/05/24
 * @note:
 *      add by zhaominyong at 2021/05/24 for bug:54377 【备份还原】备份数据到U盘的过程中拔出U盘，备份还原工具仍然一直显示正在备份数据
 */
bool UDiskDataBackupProxy::checkDestDirExists()
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
            m_isForce = true;
            if (m_calc != nullptr)
                m_calc->stop();
            if (m_p != nullptr)
                m_p->stop();
            // 10s钟后如果还没有退出，则强制退出
            QTimer::singleShot(10*1000, this, &UDiskDataBackupProxy::checkDestDirExists);
        } else {
            QTimer::singleShot(1*1000, this, &UDiskDataBackupProxy::checkDestDirExists);
        }
    }

    return true;
}


