/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/

#include "autosystembackupproxy.h"
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

IMPLEMENT_DYNCREATE(AutoSystemBackupProxy)

AutoSystemBackupProxy::AutoSystemBackupProxy()
{
    m_bSuccess = false;
    m_isFinished = false;
    m_size = 0;
    m_calc = new CalcBackupSize(this);
    m_isOnlyCheck = true;
    m_mksquashfs = nullptr;
    m_p = nullptr;
    m_calcExtra = nullptr;
    m_rsync = nullptr;

    connect(this, &AutoSystemBackupProxy::cancel, this, &AutoSystemBackupProxy::cancelEx);
}

AutoSystemBackupProxy::~AutoSystemBackupProxy()
{
    delete m_calc;
    m_calc = nullptr;

    delete m_mksquashfs;
    m_mksquashfs = nullptr;

    delete m_p;
    m_p = nullptr;

    delete m_rsync;
    m_rsync = nullptr;

    // 如果备份失败，则删除备份数据
    if (!m_bSuccess) {
        deleteFailedData();
    }
}

void AutoSystemBackupProxy::loadExistsBackupPoint()
{
    QString uniXmlPath = Utils::getSysRootPath() + BACKUP_XML_PATH;
    uniXmlPath.replace("//", "/");
    ParseBackupList uniParse(uniXmlPath);
    m_backupPoint = uniParse.findBackupPointByUuid(m_curUuid);
    QString xmlPath = Utils::getSysRootPath() + SYS_UPDATE_BACKUP_XML_PATH;
    xmlPath.replace("//", "/");
    ParseBackupList parse(xmlPath);
    // 查询节点, 系统更新备份需要兼容旧备份点
    if (m_backupPoint.isNull() && m_curUuid == AUTO_BACKUP_UUID)
    {
         m_backupPoint = parse.findBackupPointByUuid(m_curUuid);
    }
    else if (m_curUuid == AUTO_BACKUP_UUID)
    {
        m_backupPoint.m_iPosition = BackupPosition::CUSTOMIZE;
        parse.updateItem(m_backupPoint);
        uniParse.deleteItem(m_backupPoint.m_uuid);
    }
}

/**
 * @brief 环境检测
 * @return
 */
bool AutoSystemBackupProxy::checkEnvEx()
{
    qDebug() << "AutoSystemBackupProxy::checkEnv invoke begin";

    // 自动系统备份AUTO_BACKUP_SYSTEM也归为系统备份BACKUP_SYSTEM
    m_backupWrapper.m_type = BackupType::AUTO_BACKUP_SYSTEM;
    m_curUuid += m_backupWrapper.m_uuid;
    if (m_curUuid == AUTO_BACKUP_UUID)
        m_xmlFile = SYS_UPDATE_BACKUP_XML_PATH;
    else
        m_xmlFile = BACKUP_XML_PATH;

    // 1、检查/backup分区是否挂载上(不管是本地磁盘还是u盘设备，都得保证/backup挂载上); 若没挂载，挂载。
    MyMountProxy mountProxy;
    MountResult result = mountProxy.mountBackupPartition();
    // 无备份分区
    if (MountResult::CANNOT_GET_BACKUPUUID == result) {
        qInfo() << "There is no backup partition!";

        QString snapshotsPath = Utils::getSysRootPath() + BACKUP_SNAPSHOTS_PATH;
        snapshotsPath.replace("//", "/");
        Utils::mkpath(snapshotsPath);
        Utils::generateExcludePathsFile();

        m_hasBackupPartition = false;
    } else if (MountResult::MOUNTED != result) {
        emit checkResult(int(BackupResult::BACKUP_PARTITION_MOUNT_FAIL));
        return false;
    }

    /* @note:下面策略是系统更新备份时使用的，其它备份暂时还是只使用压缩备份
     * 2、如果已存在自动备份点，则先判断是否压缩备份:
     *    1) 如果是压缩备份，则删除；
     *    2) 如果不是压缩备份，则判断增量备份空间是否足够
     *       2.1) 增量备份空间足够，则进行增量备份
     *       2.2）增量备份空间不足，则删除已存在的备份。判断哪个分区有足够的空间进行非压缩的全量系统备份（所谓的空间足够就是能够满足非压缩的全量备份，并且备份空间占用分区空间不超过剩余空间的70%）
     *            2.2.1）如果某分区空间充足，则在该分区进行非压缩的全量备份
     *            2.2.2）如果没有分区有充足的空间，则判断哪个分区有足够的空间进行压缩的全量系统备份
     *                   2.2.2.1）如果某分区空间充足，则在该分区进行压缩的全量备份
     *                   2.2.2.2）如果无充足空间，则抛出错误：空间不足
     */
    m_calcMethod = MKSQUASHFS_COMPRESS;
    loadExistsBackupPoint();
    if (m_backupPoint.isNull())
    {
        // 备份点不存在，表示是新备份，系统更新备份时新备份优先非压缩全量备份方式计算
        if (m_curUuid == AUTO_BACKUP_UUID)
            m_calcMethod = NONCOMPRESS_FULL;
    }
    else
    {
        // 判断是否压缩备份
        QString backupDir = Utils::getSysRootPath() + m_backupPoint.m_path + BACKUP_SNAPSHOTS_PATH + "/" + m_backupPoint.m_uuid + "/data";
        QString imgFile = backupDir + "/"  + UDISK_MKSQUASHFS_IMG_NAME;
        imgFile.replace("//", "/");
        QString bootPath = backupDir + "/boot";
        bootPath.replace("//", "/");
        if (Utils::fileExists(imgFile))
        {
            // dst.img文件存在，说明是压缩备份，移除旧备份点
            removeAutoBackupPoint(m_backupPoint.m_uuid);

            if (m_curUuid == AUTO_BACKUP_UUID)
                m_calcMethod = NONCOMPRESS_FULL;
        }
        else if (Utils::isDirEmpty(bootPath) && m_curUuid == AUTO_BACKUP_UUID)
        {
            m_calcMethod = NONCOMPRESS_FULL;
        }
        else if (m_curUuid == AUTO_BACKUP_UUID)
        {
            m_calcMethod = NONCOMPRESS_INC;
        }
    }

    // 3、计算备份大小 NONCOMPRESS_INC>NONCOMPRESS_FULL>MKSQUASHFS_COMPRESS
    m_hasConnectCalcProcess = false;
    calcSizeForBackup();

    qDebug() << "AutoSystemBackupProxy::checkEnv invoke end";
    return true;
}

/**
 * @brief 执行备份逻辑
 */
void AutoSystemBackupProxy::doWorkEx()
{
    qDebug() << "AutoSystemBackupProxy::doWorkEx invoke begin";

    m_isOnlyCheck = false;
    // 环境检测
    checkEnvEx();

    qDebug() << "AutoSystemBackupProxy::doWorkEx invoke end";
}

void AutoSystemBackupProxy::cancelEx()
{
    m_bCancel = true;
    if (!m_isFinished) {
        emit this->checkResult(int(BackupResult::START_CANCEL));

        if (m_calc)
            m_calc->stop();
        if (m_mksquashfs)
            m_mksquashfs->stop();

        QProcess::execute("sync");
        Utils::wait(5);
        deleteFailedData();
        emit this->checkResult(int(BackupResult::CANCEL_SUCCESS));
    }
}

/**
 * @brief 失败则删除相应数据
 */
void AutoSystemBackupProxy::deleteFailedData()
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
    QString xmlPath = Utils::getSysRootPath() + m_xmlFile;
    xmlPath.replace("//", "/");
    ParseBackupList parse(xmlPath);
    parse.deleteItem(m_curUuid);
}

/**
 * @brief 校验剩余空间是否满足备份
 */
void AutoSystemBackupProxy::checkFreeCapacity(qint64 itotalSize)
{
    qDebug() << "AutoSystemBackupProxy::checkFreeCapacity invoke begin";

    // 如果是取消了操作，则不再发送其它信息
    if (m_bCancel)
        return ;

    // 1、计算待备份数据的大小
    m_size = itotalSize;
    // 备份过程中会有一些临时文件产生，会占用一部分空间，故我们预留500M的空间
    itotalSize += 500 * MB;
    qDebug() << "备份数据大小+500M预留空间约为：" << itotalSize << "字节";

    qint64 extraSize = 0;
    switch (m_calcMethod) {
    case MKSQUASHFS_COMPRESS:
        // 判断是否需要先压缩成img文件，压缩后一般小于原大小的60%
        itotalSize = itotalSize * 6 / 10;
        extraSize = calcSizeForExtraBackup();
        qDebug() << "额外备份数据大小约为：" << extraSize << "字节";
        itotalSize += extraSize;
        qDebug() << "数据压缩备份，由于数据不同压缩比也不同，为保险起见，暂定为数据大小的60%，大概需要空间：" << itotalSize << "字节";

        break;
    default:
        extraSize = calcSizeForExtraBackup();
        qDebug() << "额外备份数据大小约为：" << extraSize << "字节";
        itotalSize += extraSize;
        qDebug() << "系统备份大概需要空间：" << itotalSize << "字节";

        break;
    }

    // 2、计算各个分区剩余空间大小，判断是否有分区满足备份条件
    bool isEnough = false;
    QMap<QString, QString> hash = Utils::getLeftSizeOfPartitions();
    QHash<QString, QString> hashPrefixPath;
    QStringList partitions;
    if (m_hasBackupPartition)
    {
        hashPrefixPath.insert("/", "/autobackup");
        hashPrefixPath.insert("/backup", "");
        hashPrefixPath.insert("/data", "/data");

        partitions << "/backup" << "/data" << "/";
    }
    else
    {
        hashPrefixPath.insert("/", "/autobackup");
        hashPrefixPath.insert("/data", "/data");

        partitions << "/data" << "/";
    }
    for (const QString& partition : partitions) {
        QString size = hash.value(partition);
        if (size.endsWith("G")) {
            size.replace("G", "");
            qint64 leftSize = size.toFloat() * GB;
            qDebug() << "分区" << partition << "剩余空间大小" << leftSize << "字节";

            if (NONCOMPRESS_INC == m_calcMethod)
            {
                // 根据备份路径前缀判断所在分区
                QString prefix = hashPrefixPath.value(partition);
                if (partition == "/backup")
                {
                    if (m_backupPoint.m_path != prefix)
                        continue ;
                }
                else if (partition == "/data")
                {
                    if (m_backupPoint.m_path != prefix && !m_backupPoint.m_path.startsWith(prefix + "/"))
                        continue ;
                }
                else
                {
                    if (partition != "/")
                        continue ;
                }

                if (itotalSize < leftSize) {
                    m_backupWrapper.m_prefixDestPath = hashPrefixPath.value(partition);
                    QString curBackupPath = m_backupWrapper.m_prefixDestPath + BACKUP_SNAPSHOTS_PATH;
                    curBackupPath.replace("//", "/");
                    m_backupWrapper.m_backupExcludePaths << curBackupPath;

                    m_backupWrapper.m_type = BackupType::INC_BACKUP_SYSTEM;
                    isEnough = true;
                    break ;
                }

                break ;
            }
            else if (NONCOMPRESS_FULL == m_calcMethod)
            {
                // 备份分区可以用到剩余空间的80%，其它分区默认可以用到剩余空间的70%
                int rate = 70;
                if (partition == "/backup")
                    rate = 80;

                if (itotalSize < leftSize * rate / 100)
                {
                    m_backupWrapper.m_prefixDestPath = hashPrefixPath.value(partition);
                    QString curBackupPath = m_backupWrapper.m_prefixDestPath + BACKUP_SNAPSHOTS_PATH;
                    curBackupPath.replace("//", "/");
                    m_backupWrapper.m_backupExcludePaths << curBackupPath;
                    isEnough = true;
                    break ;
                }
            }
            else
            {
                if (itotalSize < leftSize) {
                    m_backupWrapper.m_prefixDestPath = hashPrefixPath.value(partition);
                    QString curBackupPath = m_backupWrapper.m_prefixDestPath + BACKUP_SNAPSHOTS_PATH;
                    curBackupPath.replace("//", "/");
                    m_backupWrapper.m_backupExcludePaths << curBackupPath;
                    isEnough = true;
                    break ;
                }
            }
        }
    }

    if (isEnough) {
        emit checkResult(int(BackupResult::CHECK_ENV_SUCCESS));
    } else {
        switch (m_calcMethod) {
        case NONCOMPRESS_INC:
            // 原备份分区空间不足，删除已存在的备份点，准备下一轮全量备份空间校验
            removeAutoBackupPoint(m_backupPoint.m_uuid);
            m_calcMethod = NONCOMPRESS_FULL;

            // 2s钟后再执行下一轮校验，等待本次calc进程结束
            QTimer::singleShot(2 * 1000, this, &AutoSystemBackupProxy::calcSizeForBackup);

            return ;
        case NONCOMPRESS_FULL:
            // 全量不压缩备份，空间不足，准备下一轮压缩全量备份空间校验
            m_calcMethod = MKSQUASHFS_COMPRESS;

            // 2s钟后再执行下一轮校验，等待本次calc进程结束
            QTimer::singleShot(2 * 1000, this, [=]() {
                this->checkFreeCapacity(this->m_size);
            });

            return ;
        default:
            break;
        }

        emit checkResult(int(BackupResult::BACKUP_CAPACITY_IS_NOT_ENOUGH));
        return ;
    }

    if (m_isOnlyCheck)
        return ;

    // 3、准备
    if (!doPrepare())
        return ;

    // 4、开始制作img或备份系统
    if (m_calcMethod == MKSQUASHFS_COMPRESS)
        doMksqushfs();
    else
        backupSystem();

    qDebug() << "AutoSystemBackupProxy::checkFreeCapacity invoke end";
}

/**
 * @brief 计算备份所需空间大小
 * @return 计算备份所需空间大小,单位字节
 */
void AutoSystemBackupProxy::calcSizeForBackup()
{
    // 拼接备份源路径和目标路径，测试所需备份空间大小；
    QString srcPath = Utils::getSysRootPath();
    srcPath += "/";
    srcPath.replace("//", "/");

    QString destPath = Utils::getSysRootPath();
    switch (m_calcMethod)
    {
    case NONCOMPRESS_INC:
        destPath += m_backupPoint.m_path + BACKUP_SNAPSHOTS_PATH + "/" + m_backupPoint.m_uuid + "/data";
        destPath.replace("//", "/");

        break ;
    default:
        // 目标路径为一空目录
        destPath += CHECK_PATH;
        destPath.replace("//", "/");
        Utils::mkpath(destPath);

        break;
    }

    QStringList args = getRsyncArgs(AutoSystemBackupScene::TRY_SYSTEM_BACKUP);
    args << srcPath;
    args << destPath;

    if (!m_hasConnectCalcProcess){
        m_hasConnectCalcProcess = true;
        connect(m_calc, &CalcBackupSize::finished, this, &AutoSystemBackupProxy::checkFreeCapacity);
    }
    m_calc->start(args, false);
}

/**
 * @brief 根据场景获取rsync命令参数
 * @param scene，场景
 * @return rsync的参数信息
 */
QStringList AutoSystemBackupProxy::getRsyncArgs(AutoSystemBackupScene scene)
{
    QStringList args;
    QStringList binds;
    QString backupFile = "/tmp/.backup.user";

    switch (scene) {
    case AutoSystemBackupScene::TRY_SYSTEM_BACKUP :
        args << "-aAHXrn";
        args << "--stats";
        args << "--ignore-missing-args";

        // --exclude=排除路径设置
        for (const QString & item : m_backupWrapper.m_backupExcludePaths) {
            args << QString("--exclude=%1").arg(item);
        }

        break ;
    case AutoSystemBackupScene::SYSTEM_BACKUP :
        args << "-avAHXr";
        args << "--info=progress2";
        args << "--no-inc-recursive";
        args << "--ignore-missing-args";
        args << "--delete";

        // --exclude=排除路径设置
        for (const QString & item : m_backupWrapper.m_backupExcludePaths) {
            args << QString("--exclude=%1").arg(item);
        }

        break ;
    case AutoSystemBackupScene::MKSQUASHFS :
        // （在mksquashfs时排除bind挂载的任意一方时，都备份不上）
        Utils::excludeFstabBindPath(binds);
        // --exclude=排除路径设置
        for (QString item : m_backupWrapper.m_backupExcludePaths) {
            if (binds.contains(item))
                continue;
            if (item.endsWith("/*")) {
                item = item.mid(0, item.length() - 2);
            }

            args << "-e" << item;
        }

        break ;
    case AutoSystemBackupScene::TRY_EXTRA_DATA_BACKUP :
        Utils::writeFileByLines(backupFile, m_backExtraPaths);

        args << "-aAHXrn";
        args << "--stats";
        args << "--ignore-missing-args";
        args << "--files-from" << backupFile;

        break ;
    case AutoSystemBackupScene::EXTRA_DATA_BACKUP :
        //文件在计算额外备份空间时已生成，请调用前确保已经调用过计算空间的逻辑
        // Utils::writeFileByLines(backupFile, m_backExtraPaths);

        args << "-avAHXr";
        args << "--info=progress2";
        args << "--no-inc-recursive";
        args << "--ignore-missing-args";
        args << "--delete";
        args << "--files-from" << backupFile;
        break ;
    default:
        break ;
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

    return args;
}

/**
 * @brief mksqushfs
 */
void AutoSystemBackupProxy::doMksqushfs()
{
    qDebug() << "AutoSystemBackupProxy::doMksqushfs invoke begin";

    m_mksquashfs = new MkSquashFSProcess(this);
    connect(m_mksquashfs, &MkSquashFSProcess::progress, this, &AutoSystemBackupProxy::progress);
    connect(m_mksquashfs, &MkSquashFSProcess::finished, this, [=](bool result) {
        // 如果是取消了操作，则不再发送其它信息
        if (m_bCancel)
            return ;

        m_isFinished = true;
        if (result) {
            m_backupPoint.m_state = BACKUP_PARSE_STATE_SUCCESS_STRTING;
            m_backupPoint.m_size = Utils::StringBySize(Utils::getDirOrFileSize(m_destPath));
            QString xmlPath = Utils::getSysRootPath() + m_xmlFile;
            xmlPath.replace("//", "/");
            ParseBackupList parse(xmlPath);
            if (ParseBackupList::ParseResult::SUCCESS != parse.updateItem(m_backupPoint)) {
                qCritical() << "update backuplist.xml error in sendBackupResult";
                result = false;
            } else {
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

    QString srcPath = Utils::getSysRootPath();
    srcPath += "/";
    srcPath.replace("//", "/");
    QString dstImg = m_destPath + "/" + UDISK_MKSQUASHFS_IMG_NAME;
    QStringList args;
    args << srcPath << dstImg;
    args.append(getRsyncArgs(AutoSystemBackupScene::MKSQUASHFS));

    if (m_mksquashfs->start(args)) {
        // 备份额外数据
        extraDataBackup();

        //do_kylin_security(m_destPath);
    } else {
        emit checkResult(int(BackupResult::MKSQUASHFS_DO_FAIL));
    }

    qDebug() << "AutoSystemBackupProxy::doMksqushfs invoke end";
}

/**
 * @brief 备份系统
 * @return true，备份成功；false，备份失败
 */
bool AutoSystemBackupProxy::backupSystem()
{
    qDebug() << "AutoSystemBackupProxy::backupSystem invoke begin";

    QStringList args;
    args = getRsyncArgs(AutoSystemBackupProxy::SYSTEM_BACKUP);

    // 拼接备份源路径和目标路径
    QString srcPath = Utils::getSysRootPath();
    srcPath += "/";
    srcPath.replace("//", "/");
    args << srcPath;
    QString destPath = m_destPath + "/";
    destPath.replace("//", "/");
    args << destPath;

    m_rsync = new RsyncPathToDirProcess(this);
    connect(m_rsync, &RsyncPathToDirProcess::progress, this, &AutoSystemBackupProxy::progress);
    connect(m_rsync, &RsyncPathToDirProcess::finished, this,  [&](bool result) {
        // 如果是取消了操作，则不再发送其它信息
        if (m_bCancel)
            return ;

        m_isFinished = true;
        if (result) {
            m_backupPoint.m_state = BACKUP_PARSE_STATE_SUCCESS_STRTING;
            m_backupPoint.m_size = Utils::StringBySize(Utils::getDirOrFileSize(m_destPath));
            QString xmlPath = Utils::getSysRootPath() + m_xmlFile;
            xmlPath.replace("//", "/");
            ParseBackupList parse(xmlPath);
            parse.updateItem(m_backupPoint);

            Utils::writeBackupLog(m_backupPoint.m_time + ","
                                  + m_curUuid + "," + QString::number(m_backupWrapper.m_type) + ","
                                  + m_backupWrapper.m_note + "," + m_backupPoint.m_size
                                  + ",," + m_backupWrapper.m_backupName);
            Utils::update_backup_unique_settings(m_curUuid, m_backupPoint.m_backupName);
            m_bSuccess = true;
        }
        emit this->workResult(result);
    });
    m_rsync->start(args, false);

    // 备份额外数据
    extraDataBackup();

    //do_kylin_security(m_destPath);

    qDebug() << "AutoSystemBackupProxy::backupSystem invoke end";
    return true;
}

/**
 * @brief 备份准备
 * @return true,准备成功；false，准备失败
 */
bool AutoSystemBackupProxy::doPrepare()
{
    qDebug() << "AutoSystemBackupProxy::doPrepare invoke begin";

    m_bSuccess = false;

    // 1、设置当前备份的Uuid，迁移到前面去
    // m_curUuid += m_backupWrapper.m_uuid;

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

    qDebug() << "AutoSystemBackupProxy::doPrepare invoke end";
    return true;
}

/**
 * @brief 记录/backup/snapshots/backuplist.xml文件
 * @return true-成功；false-失败
 */
bool AutoSystemBackupProxy::recordBackupPoint()
{
    m_backupPoint.m_backupName = m_backupWrapper.m_backupName;
    m_backupPoint.m_uuid = m_curUuid;
    m_backupPoint.m_iPosition = m_backupWrapper.m_iPosition;
    m_backupPoint.m_type = m_backupWrapper.m_type;
    m_backupPoint.m_size = Utils::StringBySize(m_size);
    m_backupPoint.m_time = QDateTime::currentDateTime().toString("yy-MM-dd hh:mm:ss");
    m_backupPoint.m_state = BACKUP_PARSE_STATE_FAIL_STRTING;
    m_backupPoint.m_os = SystemInfo::m_os;
    m_backupPoint.m_note = m_backupWrapper.m_note;
    m_backupPoint.m_arch = SystemInfo::m_arch;
    m_backupPoint.m_archdetect = SystemInfo::m_archDetect;
    m_backupPoint.m_path = m_backupWrapper.m_prefixDestPath;
    m_backupPoint.m_note = m_backupWrapper.m_note;
    QString xmlPath = Utils::getSysRootPath() + m_xmlFile;
    xmlPath.replace("//", "/");
    ParseBackupList parse(xmlPath);
    if (parse.updateItem(m_backupPoint) != ParseBackupList::SUCCESS) {
        emit checkResult(int(BackupResult::WRITE_STORAGEINFO_ADD_ITEM_FAIL));
        return false;
    }

    return true;
}

/**
 * @brief 删除指定备份点
 */
void AutoSystemBackupProxy::removeAutoBackupPoint(const QString& uuid)
{
    QString xmlPath = Utils::getSysRootPath() + m_xmlFile;
    xmlPath.replace("//", "/");
    ParseBackupList parse(xmlPath);
    // 查询节点
    ParseBackupList::BackupPoint backupPoint = parse.findBackupPointByUuid(uuid);

    // 如果节点不存在，则返回
    if (backupPoint.m_uuid.isEmpty())
        return ;

    // 1、删除/backup/snapshots/backuplist.xml对应项
    if (parse.deleteItem(uuid) != ParseBackupList::SUCCESS) {
        qWarning() << QString("failed to delete %1 from backuplist.xml!").arg(uuid);
    }

    // 2、删除/backup/snapshots/{uuid}对应目录
    QString uuidPath = Utils::getSysRootPath() + backupPoint.m_path + BACKUP_SNAPSHOTS_PATH + "/" + uuid;
    uuidPath.replace("//", "/");
    QStringList args;
    args << "-rf";
    args << uuidPath;
    Utils::executeCmd("rm", args);

    // 3、同时删除唯一识别文件中对应的条目
    Utils::deleteBackupUniqueRecord(backupPoint.m_backupName);
}

void AutoSystemBackupProxy::do_kylin_security(const QString& dstDir)
{
    int ret = 0;
}

// 额外数据路径列表
QStringList AutoSystemBackupProxy::extraDataPaths()
{
    QString homeDir = Utils::getSysRootPath() + "/home";
    homeDir.replace("//", "/");
    QStringList users = Utils::getSubDirs(homeDir);

    QStringList extras;
    QStringList paths;
    Utils::extraBackupOrRestorePaths(paths);

    for (const QString& path : paths)
    {
        if (path.startsWith("/home/*")) {
            for (const QString& user : users) {
                if (user == "" || user == "oem")
                    continue ;

                QString extra(path);
                extra.replace("/home/*", QString("/home/") + user);
                extras << extra;
            }
        } else {
            extras << path;
        }
    }

    return extras;
}

// 额外数据备份
void AutoSystemBackupProxy::extraDataBackup()
{
    if (m_backExtraPaths.empty())
        return ;

    qDebug() << "AutoSystemBackupProxy::extraDataBackup invoke begin";

    //准备额外备份备份目录
    m_extraPath = m_backupWrapper.m_prefixDestPath + BACKUP_SNAPSHOTS_PATH + "/" + m_curUuid + "/extra_data";
    m_extraPath.replace("//", "/");
    if (!Utils::mkpath(m_extraPath)) {
        qCritical() << QString("mkdir %1 failed !").arg(m_extraPath) ;
        return ;
    }

    //额外数据备份的文件
    QString extraUserFile = m_backupWrapper.m_prefixDestPath + BACKUP_SNAPSHOTS_PATH + "/" + m_curUuid + "/" + EXTRA_PATHS_USER_FILE;
    extraUserFile.replace("//", "/");
    if (!Utils::writeFileByLines(extraUserFile, m_backExtraPaths)) {
        qCritical() << QString("create file %1 failed !").arg(extraUserFile);
        return ;
    }

    QStringList args = getRsyncArgs(AutoSystemBackupScene::EXTRA_DATA_BACKUP);

    // 拼接备份源路径和目标路径
    QString srcPath = Utils::getSysRootPath();
    srcPath += "/";
    srcPath.replace("//", "/");
    args << srcPath;
    QString destPath = m_extraPath + "/";
    destPath.replace("//", "/");
    Utils::mkpath(destPath);
    args << destPath;

    if (m_p == nullptr)
        m_p = new RsyncPathToDirProcess(this);
    m_p->start(args, false);

    qDebug() << "AutoSystemBackupProxy::extraDataBackup invoke end";
}

/**
 * @brief 计算额外备份数据所需空间大小
 * @return 以字节为单位的空间大小
 * @note  由于当前额外备份数据量很小，瞬间完成，故暂时先以同步阻塞方式进行
 */
int AutoSystemBackupProxy::calcSizeForExtraBackup()
{
    qDebug() << "AutoSystemBackupProxy::calcSizeForExtraBackup";

    // 拼接备份源路径和目标路径，测试所需备份空间大小；目标路径为一虚拟路径
    QString srcPath = Utils::getSysRootPath();
    srcPath += "/";
    srcPath.replace("//", "/");
    QString destPath = Utils::getSysRootPath();
    destPath += CHECK_PATH;
    destPath.replace("//", "/");
    Utils::mkpath(destPath);

    m_backExtraPaths = extraDataPaths();

    if (m_backExtraPaths.empty())
        return 0;

    QStringList args = getRsyncArgs(AutoSystemBackupScene::TRY_EXTRA_DATA_BACKUP);
    args << srcPath;
    args << destPath;

    if (m_calcExtra == nullptr)
        m_calcExtra = new CalcBackupSize(this);

    return m_calcExtra->start(args, true);
}

