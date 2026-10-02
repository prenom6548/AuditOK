/*
 *  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
 *  yhkylin-systembackup-tools is licensed under GPL-2+
 *  See LICENSE file for more details.
 */

#include "customizedatabackupproxy.h"
#include <QStorageInfo>
#include <QDateTime>
#include <QDebug>
#include <QRegularExpression>

#include "../common/utils.h"
#include "../common/mydusizetool.h"
#include "mymountproxy.h"
#include "myprocess/calcbackupsize.h"

IMPLEMENT_DYNCREATE(CustomizeDataBackupProxy)

CustomizeDataBackupProxy::CustomizeDataBackupProxy()
{
    m_isOnlyCheck = true;
    m_bSuccess = false;
    m_isFinished = false;
    m_p = nullptr;
    m_size = 0;
    m_calc = new CalcBackupSize(this);

    connect(this, &CustomizeDataBackupProxy::cancel, this, &CustomizeDataBackupProxy::cancelEx);
}

CustomizeDataBackupProxy::~CustomizeDataBackupProxy()
{
    delete m_p;
    m_p = nullptr;

    delete m_calc;
    m_calc = nullptr;
}

/**
 * @brief 环境检测
 * @return false,检测失败;true,检测成功
 */
bool CustomizeDataBackupProxy::checkEnvEx()
{
    qDebug() << "CustomizeDataBackupProxy::checkEnv invoke begin";

    // 1、检查/backup分区是否挂载上(不管是本地磁盘还是u盘设备，都得保证/backup挂载上); 若没挂载，挂载
    MyMountProxy mountProxy;
    MountResult result = mountProxy.mountBackupPartition();
    // 无备份分区
    if (MountResult::CANNOT_GET_BACKUPUUID == result) {
        qInfo() << "There is no backup partition!";

        QString snapshotsPath = Utils::getBackupRootPath() + BACKUP_SNAPSHOTS_PATH;
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

    if (Utils::isDirEmpty(m_backupWrapper.m_prefixDestPath)) {
        qCritical() << QString("dstDir %s is not exist!").arg(m_backupWrapper.m_prefixDestPath);
        Utils::mkpath(m_backupWrapper.m_prefixDestPath);
    }

    // 因为有可能是选择的移动设备，故进行校验移动设备情况：空间大小、文件格式、挂载模式等
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

    // 2、判断备份是否增量备份
    isIncBackup();

    // 3、检测空间是否满足备份
    calcSizeForBackup();

    qDebug() << "CustomizeDataBackupProxy::checkEnv invoke end";
    return true;
}

/**
 * @brief 执行备份逻辑
 */
void CustomizeDataBackupProxy::doWorkEx()
{
    qDebug() << "CustomizeDataBackupProxy::doWorkEx invoke begin";

    m_isOnlyCheck = false;
    // 环境检测
    checkEnvEx();

    qDebug() << "CustomizeDataBackupProxy::doWorkEx invoke end";
}

/**
 * @brief 取消操作
 */
void CustomizeDataBackupProxy::cancelEx()
{
    qDebug() << "CustomizeDataBackupProxy::cancelEx invoke begin";

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

    qDebug() << "CustomizeDataBackupProxy::cancelEx invoke end";
}

/**
 * @brief 失败则删除相应数据
 */
void CustomizeDataBackupProxy::deleteFailedData()
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
    QString xmlPath = Utils::getBackupRootPath() + BACKUP_XML_PATH;
    xmlPath.replace("//", "/");
    ParseBackupList parse(xmlPath);
    parse.deleteItem(m_curUuid);
}

/**
 * @brief 判断是否增量备份
 * @return true,增量备份； false，全量备份
 */
bool CustomizeDataBackupProxy::isIncBackup()
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
bool CustomizeDataBackupProxy::checkFreeCapacity(qint64 itotalSize)
{
    qDebug() << "CustomizeDataBackupProxy::checkFreeCapacity invoke begin";

    // 如果是取消了操作，则不再发送其它信息
    if (m_bCancel)
        return false;

    // 1、计算待备份数据的大小
    m_size = itotalSize;
    // 备份过程中会有一些临时文件产生，会占用一部分空间，故我们预留5M的空间
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
    doBackup();

    qDebug() << "CustomizeDataBackupProxy::checkFreeCapacity invoke end";
    return result;
}

/**
 * @brief 计算备份所需空间大小
 * @return 计算备份所需空间大小,单位字节
 */
void CustomizeDataBackupProxy::calcSizeForBackup()
{
    QString destPath = Utils::getBackupRootPath();

    QStringList args;
    if (m_backupWrapper.m_bIncrement) {
        // 在原来的备份点上增量备份场景
        args = getRsyncArgs(CustomizeDataBackupScene::TRY_INC_DATA_BACKUP);
        destPath = m_backupWrapper.m_prefixDestPath;
        destPath += BACKUP_SNAPSHOTS_PATH;
        destPath += "/";
        destPath += m_backupWrapper.m_uuid;
        destPath += "/data/";
    } else {
        // 全量备份场景
        args = getRsyncArgs(CustomizeDataBackupScene::TRY_DATA_BACKUP);
        destPath += CHECK_PATH;
    }

    // 拼接备份源路径和目标路径
    QString srcPath = Utils::getBackupRootPath();
    srcPath += "/";
    srcPath.replace("//", "/");
    args << srcPath;
    destPath.replace("//", "/");
    args << destPath;
    Utils::mkpath(destPath);

    connect(m_calc, &CalcBackupSize::finished, this, &CustomizeDataBackupProxy::checkFreeCapacity);
    m_calc->start(args, false);
}

/**
 * @brief 根据场景获取rsync命令参数
 * @param scene，场景
 * @return rsync的参数信息
 */
QStringList CustomizeDataBackupProxy::getRsyncArgs(CustomizeDataBackupScene scene)
{
    QStringList args;
    QString backupFile = "/tmp/.backup.user";
    Utils::writeFileByLines(backupFile, m_backupWrapper.m_backupPaths);

    switch (scene) {
    case CustomizeDataBackupScene::DATA_BACKUP :
        args << "-avAHXr";
        args << "--info=progress2";
        args << "--no-inc-recursive";
        args << "--ignore-missing-args";
        args << "--delete";
        args << "--files-from" << backupFile;
        break ;
    case CustomizeDataBackupScene::INC_DATA_BACKUP :
        args << "-avAHXr";
        args << "--info=progress2";
        args << "--no-inc-recursive";
        args << "--ignore-missing-args";
        args << "--delete";
        args << "--files-from" << backupFile;
        break ;
    case CustomizeDataBackupScene::TRY_DATA_BACKUP :
        args << "-aAHXrn";
        args << "--stats";
        args << "--ignore-missing-args";
        args << "--files-from" << backupFile;
        break ;
    case CustomizeDataBackupScene::TRY_INC_DATA_BACKUP :
        args << "-aAHXrn";
        args << "--stats";
        args << "--ignore-missing-args";
        args << "--delete";
        args << "--files-from" << backupFile;
        break ;
    default:
        return args;
    }

    return args;
}

/**
 * @brief 备份
 */
void CustomizeDataBackupProxy::doBackup()
{
    qDebug() << "CustomizeDataBackupProxy::doBackup invoke begin";

    // 准备
    if (!doPrepare())
        return ;

    // 启动数据备份
    backupData();

    qDebug() << "CustomizeDataBackupProxy::doBackup invoke end";
}

/**
 * @brief 备份准备
 * @return true,准备成功；false，准备失败
 */
bool CustomizeDataBackupProxy::doPrepare()
{
    qDebug() << "CustomizeDataBackupProxy::doPrepare invoke begin";

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
    if (!recordBackupPoint()) {
        qCritical() << "add or update item to backuplist.xml failed !";
        return false;
    }

    qDebug() << "CustomizeDataBackupProxy::doPrepare invoke end";
    return true;
}

/**
 * @brief 记录/backup/snapshots/backuplist.xml文件
 * @return true-成功；false-失败
 */
bool CustomizeDataBackupProxy::recordBackupPoint()
{
    m_backupPoint.m_backupName = m_backupWrapper.m_backupName;
    m_backupPoint.m_uuid = m_curUuid;
    m_backupPoint.m_iPosition = m_backupWrapper.m_iPosition;
    m_backupPoint.m_type = m_backupWrapper.m_type;
    m_backupPoint.m_size = Utils::StringBySize(m_size);
    m_backupPoint.m_note = m_backupWrapper.m_note;
    m_backupPoint.m_time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    m_backupPoint.m_state = BACKUP_PARSE_STATE_FAIL_STRTING;
    if (0 < m_backupWrapper.m_frontUid)
        m_backupPoint.m_userId = QString::number(m_backupWrapper.m_frontUid);
    m_backupPoint.m_os = SystemInfo::m_os;
    m_backupPoint.m_arch = SystemInfo::m_arch;
    m_backupPoint.m_archdetect = SystemInfo::m_archDetect;
    m_backupPoint.m_path = m_backupWrapper.m_prefixDestPath;
    QString xmlPath = Utils::getBackupRootPath() + BACKUP_XML_PATH;
    xmlPath.replace("//", "/");
    ParseBackupList parse(xmlPath);
    if (m_backupWrapper.m_bIncrement) {
        if (parse.updateItem(m_backupPoint) != ParseBackupList::SUCCESS) {
            emit checkResult(int(BackupResult::WRITE_STORAGEINFO_UPDATE_ITEM_FAIL));
            return false;
        }
    } else {
        if (parse.addItem(m_backupPoint) != ParseBackupList::SUCCESS) {
            emit checkResult(int(BackupResult::WRITE_STORAGEINFO_ADD_ITEM_FAIL));
            return false;
        }
    }

    return true;
}

/**
 * @brief 备份系统
 * @return true，备份成功；false，备份失败
 */
bool CustomizeDataBackupProxy::backupData()
{
    qDebug() << "CustomizeDataBackupProxy::backupData invoke begin";

    QStringList args;
    if (m_backupWrapper.m_bIncrement) {
        // 在原来的备份点上增量备份场景
        args = getRsyncArgs(CustomizeDataBackupScene::INC_DATA_BACKUP);
    } else {
        // 全量备份场景
        args = getRsyncArgs(CustomizeDataBackupScene::DATA_BACKUP);
    }

    // 拼接备份源路径和目标路径
    QString srcPath = Utils::getBackupRootPath();
    srcPath += "/";
    srcPath.replace("//", "/");
    args << srcPath;
    QString destPath = m_destPath + "/";
    destPath.replace("//", "/");
    args << destPath;

    m_p = new RsyncPathToDirProcess(this);
    connect(m_p, &RsyncPathToDirProcess::progress, this, &CustomizeDataBackupProxy::progress);
    connect(m_p, &RsyncPathToDirProcess::finished, this,  [&](bool result) {
        // 如果是取消了操作，则不再发送其它信息
        if (m_bCancel)
            return ;

        m_isFinished = true;
        if (result) {
            m_backupPoint.m_state = BACKUP_PARSE_STATE_SUCCESS_STRTING;
            m_backupPoint.m_size = Utils::StringBySize(Utils::getDirOrFileSize(m_destPath));
            QString xmlPath = Utils::getBackupRootPath() + BACKUP_XML_PATH;
            xmlPath.replace("//", "/");
            ParseBackupList parse(xmlPath);
            parse.updateItem(m_backupPoint);

            // Utils::writeBackupLog(time + "," + m_curUuid + "," + QString::number(m_backupWrapper.m_type) + ","+ m_backupWrapper.m_note + "," + m_backupPoint.m_size+ "," + QString::number(m_backupWrapper.m_frontUid));
            Utils::writeBackupLog(m_backupPoint.m_time + ","
                                  + m_curUuid + "," + QString::number(m_backupWrapper.m_type) + ","
                                  + m_backupPoint.m_note + "," + m_backupPoint.m_size
                                  + "," + QString::number(m_backupWrapper.m_frontUid)
                                  + "," + m_backupWrapper.m_backupName);
            Utils::update_backup_unique_settings(m_curUuid, m_backupPoint.m_backupName);
            m_bSuccess = true;
        }

        emit this->workResult(result);
    });
    m_p->start(args, false);



    qDebug() << "CustomizeDataBackupProxy::backupData invoke end";
    return true;
}




