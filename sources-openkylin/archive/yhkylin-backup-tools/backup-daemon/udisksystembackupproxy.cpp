/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/

#include "udisksystembackupproxy.h"

#include <QStorageInfo>
#include <QDateTime>
#include <QDebug>
#include <QTimer>
#include <QRegularExpression>
#include "../common/utils.h"
#include "../common/mydusizetool.h"
#include "mymountproxy.h"
#include <QDBusInterface>
#include <QDBusPendingReply>

IMPLEMENT_DYNCREATE(UDiskSystemBackupProxy)

UDiskSystemBackupProxy::UDiskSystemBackupProxy()
{
    m_bSuccess = false;
    m_isFinished = false;
    m_p = nullptr;
    m_size = 0;
    m_calc = new CalcBackupSize(this);
    m_isOnlyCheck = true;
    m_mksquashfs = nullptr;
    m_isForce = false;

    connect(this, &UDiskSystemBackupProxy::cancel, this, &UDiskSystemBackupProxy::cancelEx);
}

UDiskSystemBackupProxy::~UDiskSystemBackupProxy()
{
    delete m_p;
    m_p = nullptr;

    delete m_calc;
    m_calc = nullptr;

    delete m_mksquashfs;
    m_mksquashfs = nullptr;

    QString rm("rm -rf ");
    rm += m_imgPath;
    QProcess::execute(rm);
}

/**
 * @brief 环境检测
 * @return
 */
bool UDiskSystemBackupProxy::checkEnvEx()
{
    qDebug() << "UDiskSystemBackupProxy::checkEnv invoke begin";

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

    QTimer::singleShot(1*1000, this, &UDiskSystemBackupProxy::checkDestDirExists);
    // 2、计算备份大小
    calcSizeForBackup();

    qDebug() << "UDiskSystemBackupProxy::checkEnv invoke end";
    return true;
}

/**
 * @brief 执行备份逻辑
 */
void UDiskSystemBackupProxy::doWorkEx()
{
    qDebug() << "UDiskSystemBackupProxy::doWorkEx invoke begin";

    m_isOnlyCheck = false;
    // 环境检测
    checkEnvEx();

    qDebug() << "UDiskSystemBackupProxy::doWorkEx invoke end";
}

void UDiskSystemBackupProxy::cancelEx()
{
    qDebug() << "UDiskSystemBackupProxy::cancelEx invoke begin";

    m_bCancel = true;
    if (!m_isFinished) {
        emit this->checkResult(int(BackupResult::START_CANCEL));

        if (m_calc)
            m_calc->stop();
        if (m_mksquashfs)
            m_mksquashfs->stop();
        if (m_p)
            m_p->stop();

        QProcess::execute("sync");
        Utils::wait(5);
        deleteFailedData();
        emit this->checkResult(int(BackupResult::CANCEL_SUCCESS));
    }

    qDebug() << "UDiskSystemBackupProxy::cancelEx invoke end";
}

/**
 * @brief 失败则删除相应数据
 */
void UDiskSystemBackupProxy::deleteFailedData()
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
bool UDiskSystemBackupProxy::isIncBackup()
{
    QString backupPath;
    ParseBackupList::BackupPoint point;
    if (m_backupWrapper.m_uuid.isEmpty()) {
        QString xmlPath(m_backupWrapper.m_prefixDestPath + BACKUP_XML_PATH);
        xmlPath.replace("//", "/");
        ParseBackupList parser(xmlPath);
        point = parser.getLastSysBackupPoint();
        if (point.m_uuid.isEmpty())
            return false;
        backupPath = m_backupWrapper.m_prefixDestPath + BACKUP_SNAPSHOTS_PATH + "/" + point.m_uuid + "/data";
    } else {
        backupPath = m_backupWrapper.m_prefixDestPath + BACKUP_SNAPSHOTS_PATH + "/" + m_backupWrapper.m_uuid + "/data";
    }

    backupPath.replace("//", "/");
    if (Utils::isDirExist(backupPath)) {
        m_backupWrapper.m_baseUuid = point.m_uuid;
        m_backupWrapper.m_bIncrement = true;
        m_backupWrapper.m_type = BackupType::INC_BACKUP_SYSTEM;
        return true;
    }
    return false;
}

/**
 * @brief 校验剩余空间是否满足备份
 */
void UDiskSystemBackupProxy::checkFreeCapacity(qint64 itotalSize)
{
    qDebug() << "UDiskSystemBackupProxy::checkFreeCapacity invoke begin";

    // 如果是取消了操作，则不再发送其它信息
    if (m_bCancel)
        return ;

    // 拔掉U盘的场景
    if (m_isForce) {
        emit this->checkResult(int(BackupResult::WRITE_BACKUP_PATHS_TO_USER_FAILED));
        return ;
    }

    // 1、计算待备份数据的大小
    m_size = itotalSize;
    // 备份过程中会有一些临时文件产生，会占用一部分空间，故我们预留500M的空间
    itotalSize += 500 * MB;

    // 2、计算备份分区剩余空间大小
    QString backupPath(m_backupWrapper.m_prefixDestPath);
    backupPath.replace("//", "/");
    QStorageInfo backupDisk(backupPath);
    qint64 freeSize = backupDisk.bytesAvailable();

    // 3、校验空间是否足够
    if (itotalSize > freeSize) {
        emit checkResult(int(BackupResult::BACKUP_CAPACITY_IS_NOT_ENOUGH));
        return ;
    } else {
        emit checkResult(int(BackupResult::CHECK_ENV_SUCCESS));
    }

    if (m_isOnlyCheck)
        return ;

    // 4、判断是否需要先压缩成img文件，压缩后一般小于原大小的70%
    itotalSize = itotalSize * 7 / 10;
    QMap<QString, QString> hash = Utils::getLeftSizeOfPartitions();
    QStringList partitions;
    partitions << "/backup" << "/data" << "/";
    for (const QString& partition : partitions) {
        QString size = hash.value(partition);
        if (size.endsWith("G")) {
            size.replace("G", "");
            qint64 leftSize = size.toFloat() * GB;
            if (itotalSize < leftSize) {
                m_imgPath = partition + IMGBACKUP_PATH;
                m_imgPath.replace("//", "/");
                break ;
            }
        }
    }

    // 5、开始制作img或开始备份
    if (m_imgPath.isEmpty()) {
        doBackup();
    } else {
        doMksqushfs();
    }

    qDebug() << "UDiskSystemBackupProxy::checkFreeCapacity invoke end";
}

/**
 * @brief 计算备份所需空间大小
 * @return 计算备份所需空间大小,单位字节
 */
void UDiskSystemBackupProxy::calcSizeForBackup()
{
    // 拼接备份源路径和目标路径，测试所需备份空间大小；目标路径为一虚拟路径
    QString srcPath = Utils::getSysRootPath();
    srcPath += "/";
    srcPath.replace("//", "/");
    QString destPath = Utils::getSysRootPath();
    destPath += CHECK_PATH;
    destPath.replace("//", "/");
    Utils::mkpath(destPath);

    QStringList args = getRsyncArgs(UDiskSystemBackupScene::TRY_SYSTEM_BACKUP);
    args << srcPath;
    args << destPath;

    connect(m_calc, &CalcBackupSize::finished, this, &UDiskSystemBackupProxy::checkFreeCapacity);
    m_calc->start(args, false);
}

/**
 * @brief 根据场景获取rsync命令参数
 * @param scene，场景
 * @return rsync的参数信息
 */
QStringList UDiskSystemBackupProxy::getRsyncArgs(UDiskSystemBackupScene scene)
{
    QStringList args;
    QStringList excludes;

    switch (scene) {
    case UDiskSystemBackupScene::SYSTEM_BACKUP :
        args << "-avAHXr";
        args << "--info=progress2";
        args << "--no-inc-recursive";
        args << "--ignore-missing-args";
        break ;
    case UDiskSystemBackupScene::TRY_SYSTEM_BACKUP :
        args << "-aAHXrn";
        args << "--stats";
        args << "--ignore-missing-args";
        break ;
    case UDiskSystemBackupScene::MKSQUASHFS :
        // （在mksquashfs时排除bind挂载的任意一方时，都备份不上）
        Utils::excludeFstabBindPath(excludes);
        // -e 排除路径设置
        for (QString item : m_backupWrapper.m_backupExcludePaths) {
            if (excludes.contains(item))
                continue;
            if (item.endsWith("/*")) {
                item = item.mid(0, item.length() - 2);
            }

            args << "-e" << item;
        }
        args << "-e" << m_imgPath;
        return args;
    case UDiskSystemBackupScene::IMG_BACKUP :
        args << "-avAHXr";
        args << "--info=progress2";
        args << "--no-inc-recursive";
        args << "--ignore-missing-args";
        return args;
    default:
        return args;
    }

    // 文件保护箱
    QDBusInterface interface("com.kylin.boxmanage", "/backup", "com.kylin.boxmanage.backup", QDBusConnection::systemBus());

    // 调用D-Bus方法
    QDBusMessage reply = interface.call("get_allBoxRootEntries");

    // 检查回复是否有效
    if (reply.type() == QDBusMessage::ReplyMessage) {
        // 提取返回的数据
        QStringList stringList = reply.arguments().at(0).toStringList();

        // 检查列表是否为空，并打印结果
        if (!stringList.isEmpty()) {
            qDebug() << "获取到的 Box Root Entries 是:" << stringList;
            for (int i = 0; i < stringList.size(); ++i) { // 添加了对size()的检查
                QString PathBox = stringList.at(i); // 使用.at()而不是()，虽然()在这里也可以工作，但.at()在索引无效时会抛出更明确的异常
                args << QString("--exclude=") + PathBox;
            }
        } else {
            qDebug() << "没有获取到 Box Root Entries。";
        }
    } else {
        qDebug() << "D-Bus 方法调用失败:" << reply.errorMessage();
    }

    // --exclude=排除路径设置
    for (const QString & item : m_backupWrapper.m_backupExcludePaths) {
        args << QString("--exclude=%1").arg(item);
    }

    return args;
}

/**
 * @brief mksqushfs
 */
void UDiskSystemBackupProxy::doMksqushfs()
{
    qDebug() << "UDiskSystemBackupProxy::doMksqushfs invoke begin";

    m_mksquashfs = new MkSquashFSProcess(this);
    connect(m_mksquashfs, &MkSquashFSProcess::progress, this, &UDiskSystemBackupProxy::progress);
    connect(m_mksquashfs, &MkSquashFSProcess::finished, this, [=](bool result) {
        // 如果是取消了操作，则不再发送其它信息
        if (m_bCancel)
            return ;

        if (result && !m_isForce) {
            // 开始备份
            doBackup();
        } else {
            m_isFinished = true;
            emit checkResult(int(BackupResult::MKSQUASHFS_DO_FAIL));
        }
    });
    Utils::mkpath(m_imgPath);

    QString srcPath = Utils::getSysRootPath();
    srcPath += "/";
    srcPath.replace("//", "/");
    QString dstImg = m_imgPath + "/" + UDISK_MKSQUASHFS_IMG_NAME;
    QStringList args;
    args << srcPath << dstImg;
    args.append(getRsyncArgs(UDiskSystemBackupScene::MKSQUASHFS));

    if (m_mksquashfs->start(args)) {
        emit checkResult(int(BackupResult::MKSQUASHFS_START_SUCCESS));
    } else {
        emit checkResult(int(BackupResult::MKSQUASHFS_DO_FAIL));
    }

    qDebug() << "UDiskSystemBackupProxy::doMksqushfs invoke end";
}

/**
 * @brief 备份
 */
void UDiskSystemBackupProxy::doBackup()
{
    qDebug() << "UDiskSystemBackupProxy::doBackup invoke begin";

    // 准备
    if (!doPrepare())
        return ;

    // 启动备份efi, 修改为和其它目录统一备份，不再单独进行备份
//    if (!backupEfi()) {
//        emit checkResult(int(BackupResult::EFI_RSYNC_FAIL));
//        return ;
//    }

    if (m_imgPath.isEmpty()) {
        // 启动系统备份
        backupSystem();
    } else {
        // 备份img文件
        backupImg();
    }


    qDebug() << "UDiskSystemBackupProxy::doBackup invoke end";
}

/**
 * @brief 备份准备
 * @return true,准备成功；false，准备失败
 */
bool UDiskSystemBackupProxy::doPrepare()
{
    qDebug() << "UDiskSystemBackupProxy::doPrepare invoke begin";

    m_bSuccess = false;

    // 1、设置当前备份的Uuid
    m_curUuid += Utils::createUuid();

    // 2、准备备份目录及文件
    m_destPath = m_backupWrapper.m_prefixDestPath + BACKUP_SNAPSHOTS_PATH + "/" + m_curUuid + "/data";
    m_destPath.replace("//", "/");
    if (!Utils::mkpath(m_destPath)) {
        qCritical() << QString("mkdir %1 failed !").arg(m_destPath) ;
        emit checkResult(int(BackupResult::WRITE_BACKUP_PATHS_TO_USER_FAILED));
        return false;
    }

    QString userFile = m_backupWrapper.m_prefixDestPath + BACKUP_SNAPSHOTS_PATH + "/" + m_curUuid + "/" + PATHS_USER_FILE;
    userFile.replace("//", "/");
    if (!Utils::writeFileByLines(userFile, m_backupWrapper.m_backupPaths)) {
        qCritical() << QString("create file %1 failed !").arg(userFile);
        emit checkResult(int(BackupResult::WRITE_BACKUP_PATHS_TO_USER_FAILED));
        return false;
    }

    QString excludeUserFile = m_backupWrapper.m_prefixDestPath + BACKUP_SNAPSHOTS_PATH + "/" + m_curUuid + "/" + EXCLUDE_PATHS_USER_FILE;
    excludeUserFile.replace("//", "/");
    if (!Utils::writeFileByLines(excludeUserFile, m_backupWrapper.m_backupExcludePaths)) {
        qCritical() << QString("create file %1 failed !").arg(excludeUserFile);
        emit checkResult(int(BackupResult::WRITE_BACKUP_PATHS_TO_USER_FAILED));
        return false;
    }

    // 3、记录/backup/snapshots/backuplist.xml文件
    if (!recordBackupPoint()) {
        qCritical() << "add or update item to backuplist.xml failed !";
        return false;
    }

    qDebug() << "UDiskSystemBackupProxy::doPrepare invoke end";
    return true;
}

/**
 * @brief 记录/backup/snapshots/backuplist.xml文件
 * @return true-成功；false-失败
 */
bool UDiskSystemBackupProxy::recordBackupPoint()
{
    m_backupPoint.m_backupName = m_backupWrapper.m_backupName;
    m_backupPoint.m_uuid = m_curUuid;
    m_backupPoint.m_iPosition = m_backupWrapper.m_iPosition;
    m_backupPoint.m_type = m_backupWrapper.m_type;
    m_backupPoint.m_size = Utils::StringBySize(m_size);
    m_backupPoint.m_time = QDateTime::currentDateTime().toString("yy-MM-dd hh:mm:ss");
    m_backupPoint.m_state = BACKUP_PARSE_STATE_FAIL_STRTING;
    m_backupPoint.m_os = SystemInfo::m_os;
    m_backupPoint.m_arch = SystemInfo::m_arch;
    m_backupPoint.m_archdetect = SystemInfo::m_archDetect;
    QString xmlPath = m_backupWrapper.m_prefixDestPath + BACKUP_XML_PATH;
    xmlPath.replace("//", "/");
    ParseBackupList parse(xmlPath);
    if (m_backupWrapper.m_uuid.isEmpty() || !m_backupWrapper.m_bIncrement) {
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
 * @return true，启动备份成功；false，启动备份失败
 */
bool UDiskSystemBackupProxy::backupSystem()
{
    qDebug() << "UDiskSystemBackupProxy::backupSystem invoke begin";

    // 全量备份场景
    QStringList args = getRsyncArgs(UDiskSystemBackupScene::SYSTEM_BACKUP);

    // 拼接备份源路径和目标路径
    QString srcPath = Utils::getSysRootPath();
    srcPath += "/";
    srcPath.replace("//", "/");
    args << srcPath;
    QString destPath = m_destPath + "/";
    destPath.replace("//", "/");
    args << destPath;

    return backup(args);
}

/**
 * @brief 备份系统img文件
 * @return true，启动备份成功；false，启动备份失败
 */
bool UDiskSystemBackupProxy::backupImg()
{
    qDebug() << "UDiskSystemBackupProxy::backupImg invoke";

    QStringList args;
    QString srcPath = m_imgPath + "/" + UDISK_MKSQUASHFS_IMG_NAME;
    QString destPath = m_destPath + "/";
    destPath.replace("//", "/");
    args << srcPath << destPath;

    return backup(args);
}

/**
 * @brief 备份公共逻辑
 * @param args
 * @return true，启动备份成功；false，启动备份失败
 */
bool UDiskSystemBackupProxy::backup(const QStringList &args)
{
    qDebug() << "UDiskSystemBackupProxy::backup invoke begin";

    m_p = new RsyncPathToDirProcess(this);
    connect(m_p, &RsyncPathToDirProcess::progress, this, &UDiskSystemBackupProxy::progress);
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
                                      + ",," + m_backupWrapper.m_backupName);

                Utils::update_backup_unique_settings(m_curUuid, m_backupPoint.m_backupName);
                m_bSuccess = true;
            }
        }
        emit this->workResult(result);
    });
    m_p->start(args, false);
    emit checkResult(int(BackupResult::BACKUP_START_SUCCESS));

    //do_kylin_security(m_destPath);

    qDebug() << "UDiskSystemBackupProxy::backup invoke end";
    return true;
}

void UDiskSystemBackupProxy::do_kylin_security(const QString& dstDir)
{
    int ret = 0;

}

/**
 * @brief 校验移动盘是否还在
 * @return: bool,存在返回true；不存在返回false
 * @author: zhaominyong
 * @since: 2021/05/24
 * @note:
 *      add by zhaominyong at 2021/05/24 for bug:54377 【备份还原】备份数据到U盘的过程中拔出U盘，备份还原工具仍然一直显示正在备份数据
 */
bool UDiskSystemBackupProxy::checkDestDirExists()
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
            if (m_mksquashfs != nullptr)
                m_mksquashfs->stop();
            if (m_p != nullptr)
                m_p->stop();
            // 10s钟后如果还没有退出，则强制退出
            QTimer::singleShot(10*1000, this, &UDiskSystemBackupProxy::checkDestDirExists);
        } else {
            QTimer::singleShot(1*1000, this, &UDiskSystemBackupProxy::checkDestDirExists);
        }
    }

    return true;
}



