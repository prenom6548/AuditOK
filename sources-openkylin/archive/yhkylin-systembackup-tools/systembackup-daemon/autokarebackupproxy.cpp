/*
 *  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
 *  yhkylin-systembackup-tools is licensed under GPL-2+
 *  See LICENSE file for more details.
 */

#include "autokarebackupproxy.h"
#include <QStorageInfo>
#include <QDateTime>
#include <QDebug>
#include <QTimer>
#include <QRegularExpression>

#include "../common/utils.h"
#include "../common/mydusizetool.h"
#include "mymountproxy.h"

#include "parsebackuplist.h"

IMPLEMENT_DYNCREATE(AutoKareBackupProxy)

AutoKareBackupProxy::AutoKareBackupProxy()
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

    connect(this, &AutoKareBackupProxy::cancel, this, &AutoKareBackupProxy::cancelEx);
}

AutoKareBackupProxy::~AutoKareBackupProxy()
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
    if (!m_bSuccess)
    {
        deleteFailedData();
    }
}

void AutoKareBackupProxy::loadExistsBackupPoint()
{
    QString uniXmlPath = Utils::getBackupRootPath() + BACKUP_XML_PATH;
    uniXmlPath.replace("//", "/");
    ParseBackupList uniParse(uniXmlPath);
    m_backupPoint = uniParse.findBackupPointByUuid(m_curUuid);
    QString xmlPath = Utils::getBackupRootPath() + SYS_UPDATE_BACKUP_XML_PATH;
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
bool AutoKareBackupProxy::checkEnvEx()
{
    qDebug() << "AutoKareBackupProxy::checkEnv invoke begin";

    // 自动系统备份AUTO_BACKUP_SYSTEM也归为系统备份BACKUP_SYSTEM
    m_backupWrapper.m_type = BackupType::BACKUP_SYSTEM;
    m_curUuid += m_backupWrapper.m_uuid;
    m_xmlFile = SYS_KARE_BACKUP_XML_PATH;

    // 3、计算备份大小 NONCOMPRESS_INC>NONCOMPRESS_FULL>MKSQUASHFS_COMPRESS
    m_hasConnectCalcProcess = false;
    calcSizeForBackup();

    qDebug() << "AutoKareBackupProxy::checkEnv invoke end";
    return true;
}

/**
 * @brief 执行备份逻辑
 */
void AutoKareBackupProxy::doWorkEx()
{
    qDebug() << "AutoKareBackupProxy::doWorkEx invoke begin";

    m_isOnlyCheck = false;
    //1.环境检测
    if (!doPrepare())
        return ;

    //2、开始备份Kare
    backupSystem();

    qDebug() << "AutoKareBackupProxy::doWorkEx invoke end";
}

void AutoKareBackupProxy::cancelEx()
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
void AutoKareBackupProxy::deleteFailedData()
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
    QString xmlPath = Utils::getBackupRootPath() + m_xmlFile;
    xmlPath.replace("//", "/");
    ParseBackupList parse(xmlPath);
    parse.deleteItem(m_curUuid);
}

/**
 * @brief 校验剩余空间是否满足备份
 */
void AutoKareBackupProxy::checkFreeCapacity(qint64 itotalSize)
{
    qDebug() << "AutoSystem checkFreeCapacity invoke begin";

    // 如果是取消了操作，则不再发送其它信息
    if (m_bCancel)
        return ;

    // 1、计算待备份数据的大小
    m_size = itotalSize;
    // 备份过程中会有一些临时文件产生，会占用一部分空间，故我们预留500M的空间
    itotalSize += 500 * MB;
    qDebug() << "备份数据大小+500M预留空间约为：" << itotalSize << "字节";

    qint64 leftsize = Utils::getLeftSize("/data");
    qDebug()<<"system left size "<< leftsize;

    if (leftsize < itotalSize) {
        emit checkResult(int(BackupResult::BACKUP_CAPACITY_IS_NOT_ENOUGH));
        emit workResult(false);
        qDebug()<<"left size is not enough,emit signal";
        return;
    }else{
        qDebug()<<"left size is big enough";
        emit checkResult(int(BackupResult::CHECK_ENV_SUCCESS));
    }

    if (m_isOnlyCheck)
    {
        qDebug()<<"onlychecking........###";
        return ;
    }else{
        qDebug()<<"notonlychecking.....###";
    }

    // 3、准备
    if (!doPrepare())
        return ;

    // 4、检查是否超过五个升级备份点，超过则删除最旧的备份点
    checkAndRemoveBackupPoint();

    // 5、开始备份系统
    backupSystem();

    qDebug() << "AutoKareBackupProxy::checkFreeCapacity invoke end";
}

/**
 * @brief 计算备份所需空间大小
 * @return 计算备份所需空间大小,单位字节
 */
void AutoKareBackupProxy::calcSizeForBackup()
{
    QString sysPath = "/ostree/pkgs/ovl-";
    qint64 size=0;

    // get ostree commit id
    QStringList args;
    QString otherArgs = " awk -F ' ' '{print $3}' ";
    args << "admin";
    args << "status";
    args << "|";
    args << "grep '*'";
    args << "|";
    args << otherArgs;

    QProcess process;
    QString fullCommand = "ostree " + args.join(" ");
    qDebug() << "auto systembackup ostree cmd ## " << fullCommand;

    process.start("bash", QStringList() << "-c" << "ostree admin status | grep '*' | awk '{print $3}'");
    process.waitForFinished(-1);

    if (process.exitCode() != 0) {
        qDebug() << "Error:" << process.readAllStandardError();
        return ;
    }
    QString output = process.readAllStandardOutput().trimmed();
    if (output.isEmpty()) {
        qDebug() << "No hidden files found or command failed.";
        return ;
    }else{
        sysPath += output;
        qDebug()<<"system path "<< sysPath;
        size = Utils::getDirectorySize(sysPath);
    }
    qDebug()<<"ostree syspath size ## "<< size;

    qint64 tmpSize = calcSizeForHome();
    if(tmpSize>0){
        size += tmpSize;
        qDebug()<<"syspath + homePath size "<< size;
    }
    checkFreeCapacity(size);
}

qint64 AutoKareBackupProxy::calcSizeForHome()
{
    QString homePath = "/home/" + m_backupWrapper.m_frontUserName;
    if (m_backupWrapper.m_frontUserName == "root") {
        homePath = "/root";
    }

    qDebug()<<"homepath is "<< homePath;
    QString command = QString("du -sb %1/.[^.]* | awk '{sum+=$1} END {print sum}'").arg(homePath);

    // 使用QProcess执行命令
    QProcess process;
    process.start("bash", QStringList() << "-c" << command);
    process.waitForFinished();

    // 检查命令是否执行成功
    if (process.exitCode() != 0) {
        qDebug() << "Error:" << process.readAllStandardError();
        return -1;
    }

    // 获取输出并转换为 qint64
    QByteArray output = process.readAllStandardOutput().trimmed();
    if (output.isEmpty()) {
        qDebug() << "No hidden files found or command failed.";
        return -1;
    }

    bool ok;
    qint64 totalBytes = output.toLongLong(&ok);  // 转换为 qint64

    if (!ok) {
        qDebug() << "Failed to parse output as integer.";
        return -1;
    }

    // 输出结果（qint64）
    qDebug() << "Total hidden files size (bytes):" << totalBytes;
    return totalBytes;
}
/**
 * @brief 根据场景获取rsync命令参数
 * @param scene，场景
 * @return rsync的参数信息
 */
QStringList AutoKareBackupProxy::getRsyncArgs(AutoSystemBackupScene scene)
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

    return args;
}

/**
 * @brief mksqushfs
 */
void AutoKareBackupProxy::doMksqushfs()
{
    qDebug() << "AutoKareBackupProxy::doMksqushfs invoke begin";

    m_mksquashfs = new MkSquashFSProcess(this);
    connect(m_mksquashfs, &MkSquashFSProcess::progress, this, &AutoKareBackupProxy::progress);
    connect(m_mksquashfs, &MkSquashFSProcess::finished, this, [=](bool result) {
        // 如果是取消了操作，则不再发送其它信息
        if (m_bCancel)
            return ;

        m_isFinished = true;
        if (result) {
            m_backupPoint.m_state = BACKUP_PARSE_STATE_SUCCESS_STRTING;
            m_backupPoint.m_size = Utils::StringBySize(Utils::getDirOrFileSize(m_destPath));
            QString xmlPath = Utils::getBackupRootPath() + m_xmlFile;
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

    QString srcPath = Utils::getBackupRootPath();
    srcPath += "/";
    srcPath.replace("//", "/");
    QString dstImg = m_destPath + "/" + UDISK_MKSQUASHFS_IMG_NAME;
    QStringList args;
    args << srcPath << dstImg;
    args.append(getRsyncArgs(AutoSystemBackupScene::MKSQUASHFS));

    if (m_mksquashfs->start(args)) {
        // 备份额外数据
        extraDataBackup();


    } else {
        emit checkResult(int(BackupResult::MKSQUASHFS_DO_FAIL));
    }

    qDebug() << "AutoKareBackupProxy::doMksqushfs invoke end";
}

/**
 * @brief 备份系统
 * @return true，备份成功；false，备份失败
 */
bool AutoKareBackupProxy::backupSystem()
{
    qDebug() << "AutoKareBackupProxy::backupSystem invoke begin";

    int progress =0;
    int total = m_backupWrapper.m_backupPaths.size();
    //加入目标路径
    for(int i=0; i < total; i++)
    {
        //1.cp参数拼接，接受输入参数，目录杂乱，rsync对于未明确且不确定层级的目录难以支持，改用通过cp实现
        QStringList args;
        args << "-aL";
        args << "-P";
        args << "-rf";

        QString  item = m_backupWrapper.m_backupPaths[i];

        //拼接备份源路径
        QString srcPath = item + "/*";
        srcPath.replace("//", "/");
        args << srcPath;
        //拼接备份目标路径
        QString destPath = m_destPath + "/" + item;
        destPath.replace("//", "/");
        //创建对应路径，用于存储
        if (!Utils::mkpath(destPath))
        {
            qCritical() << QString("mkdir %1 failed !").arg(destPath) ;
            emit workResult(false);
            return false;
        }
        args << destPath;

        //执行拷贝过程
        Utils::executeCmd("cp", args);

        //计算当前进度值
        progress = (double)(i + 1)/total * 100;
        qDebug() << "i:"  << i << "   total:"<< total;
        qDebug() << "当前进度"  << progress;
        emit this->progress(progress);
    }

    //更新xml元数据
    m_backupPoint.m_state = BACKUP_PARSE_STATE_SUCCESS_STRTING;
    m_backupPoint.m_size = Utils::StringBySize(Utils::getDirOrFileSize(m_destPath));
    QString xmlPath = Utils::getBackupRootPath() + SYS_KARE_BACKUP_XML_PATH;
    xmlPath.replace("//", "/");
    ParseBackupList parse(xmlPath);
    parse.updateItem(m_backupPoint);
    //记录操作日志
    Utils::writeBackupLog(m_backupPoint.m_time + ","
                          + m_curUuid + "," + QString::number(m_backupWrapper.m_type) + ","
                          + m_backupWrapper.m_note + "," + m_backupPoint.m_size
                          + ",," + m_backupWrapper.m_backupName);
    Utils::update_backup_unique_settings(m_curUuid, m_backupPoint.m_backupName);
    m_bSuccess = true;


    qDebug() << "已备份百分比：" << 100 << "%";
    emit this->progress(100); //补齐进度100%
    emit this->workResult(m_bSuccess);
    qDebug() << "Kare备份运行结束！";
    //补足进度信号，并等待3秒
    Utils::wait(3);

    qDebug() << "AutoKareBackupProxy::backupSystem invoke end";
    return true;
}

/**
 * @brief 备份准备
 * @return true,准备成功；false，准备失败
 */
bool AutoKareBackupProxy::doPrepare()
{
    qDebug() << "AutoKareBackupProxy::doPrepare invoke begin";

    m_bSuccess = false;

    // 自动系统备份AUTO_BACKUP_SYSTEM也归为系统备份BACKUP_SYSTEM
    m_backupWrapper.m_type = BackupType::BACKUP_SYSTEM;
    m_curUuid += m_backupWrapper.m_uuid;
    m_xmlFile = SYS_KARE_BACKUP_XML_PATH;

    //获取需要备份的Kare相关数据路径
    QStringList karePaths = m_backupWrapper.m_backupPaths;
    if (karePaths.size() > 1)
    {
        for(QString karePath:karePaths)
        {
            if(Utils::isDirExists(karePath))
            {

            }
            else
            {
                qDebug() << "需要备份的Kare数据：" << karePath << "不存在";
                emit workResult(false);
                return false;
            }

        }
    }

    //1.检测/data/.backup/temp/是否存在
    bool bExist = Utils::isDirExists(KARE_BACKUP_PATH);
    if(!bExist)
    {
        if (!Utils::mkpath(KARE_BACKUP_PATH))
        {
            qCritical() << QString("mkdir %1 failed !").arg(KARE_BACKUP_PATH) ;
            emit workResult(false);
            return false;
        }
    }

    // 2、准备目标目录及文件
    QString temp = KARE_BACKUP_PATH;
    m_destPath = temp + "/" + m_curUuid + "/data";
    m_destPath.replace("//", "/");
    if (!Utils::mkpath(m_destPath)) {
        qCritical() << QString("mkdir %1 failed !").arg(m_destPath) ;
        emit checkResult(int(BackupResult::WRITE_BACKUP_PATHS_TO_USER_FAILED));
        emit workResult(false);
        return false;
    }

    // 3、记录/data/.backup/snapshots/backuplist.xml文件
    if (!recordBackupPoint())
    {
        qCritical() << "add or update item to backuplist.xml failed !";
        emit workResult(false);
        return false;
    }

    qDebug() << "AutoKareBackupProxy::doPrepare invoke end";
    return true;
}

void AutoKareBackupProxy::checkAndRemoveBackupPoint()
{
    // 初始化备份点元数据xml
    QString xmlPath = Utils::getBackupRootPath() + m_xmlFile;
    xmlPath.replace("//", "/");
    ParseBackupList parse(xmlPath);

    //查询是否超过五个备份点
    int num = parse.countBackupPoint();

    //预留一个位置给新建的元数据
    if(num > 5)
    {
        qDebug() << "备份点超过五个，需要删除最老的备份点";
        ParseBackupList::BackupPoint backupPoint ;
        parse.getOldestBackupPoint(backupPoint);
        qDebug() << "最旧的备份点UUID：" << backupPoint.m_uuid;
        //删除repo仓库提交数据
        if(deleteBackupPointCommits(backupPoint))
        {
            qDebug() << "repo仓库数据删除成功";
        }
        else
        {
            qDebug() << "repo仓库数据删除失败";
        }
        //删除xml对应元数据
        parse.deleteItem(backupPoint.m_uuid);
    }
    else
    {
        qDebug() << "备份点未超过五个,当前备份点数量：" << num;
    }
}

/**
 * @brief 记录/backup/snapshots/backuplist.xml文件
 * @return true-成功；false-失败
 */
bool AutoKareBackupProxy::recordBackupPoint()
{
    m_backupPoint.m_backupName = m_backupWrapper.m_backupName;
    m_backupPoint.m_uuid = m_curUuid;
    m_backupPoint.m_iPosition = m_backupWrapper.m_iPosition;
    m_backupPoint.m_type = m_backupWrapper.m_type;
    m_backupPoint.m_size = Utils::StringBySize(m_size);
    m_backupPoint.m_time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    m_backupPoint.m_state = BACKUP_PARSE_STATE_FAIL_STRTING;
    m_backupPoint.m_os = SystemInfo::m_os;
    m_backupPoint.m_note = m_backupWrapper.m_note;
    m_backupPoint.m_arch = SystemInfo::m_arch;
    m_backupPoint.m_archdetect = SystemInfo::m_archDetect;
    m_backupPoint.m_path = m_backupWrapper.m_prefixDestPath;
    m_backupPoint.m_note = m_backupWrapper.m_note;
    m_backupPoint.m_user = m_backupWrapper.m_frontUserName;
    //每次备份新建一个分支，先给分支名命名，命名格式：branch+时间戳
    QString timestampStr = m_backupPoint.m_time;
    QString formattedTimestamp = timestampStr.replace("-", "").replace(" ", "").replace(":", "");
    QString branchName = "branch-" + formattedTimestamp;
    qDebug() << "分支名：" << branchName;
    m_backupPoint.m_branchName = branchName;

    //将kare备份路径加入元数据中管理
    m_backupPoint.m_karePaths = m_backupWrapper.m_backupPaths.join(";");

    QString xmlPath = Utils::getBackupRootPath() + m_xmlFile;
    xmlPath.replace("//", "/");
    qDebug() << xmlPath;
    ParseBackupList parse(xmlPath);
    if (parse.addItem(m_backupPoint) != ParseBackupList::SUCCESS) {
        emit checkResult(int(BackupResult::WRITE_STORAGEINFO_ADD_ITEM_FAIL));
        return false;
    }

    return true;
}

/**
 * @brief 删除指定备份点
 */
void AutoKareBackupProxy::removeAutoBackupPoint(const QString& uuid)
{
    QString xmlPath = Utils::getBackupRootPath() + m_xmlFile;
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
    QString uuidPath = Utils::getBackupRootPath() + backupPoint.m_path + BACKUP_SNAPSHOTS_PATH + "/" + uuid;
    uuidPath.replace("//", "/");
    QStringList args;
    args << "-rf";
    args << uuidPath;
    Utils::executeCmd("rm", args);

    // 3、同时删除唯一识别文件中对应的条目
    Utils::deleteBackupUniqueRecord(backupPoint.m_backupName);
}


// 额外数据路径列表
QStringList AutoKareBackupProxy::extraDataPaths()
{
    QString homeDir = Utils::getBackupRootPath() + "/home";
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
void AutoKareBackupProxy::extraDataBackup()
{
    if (m_backExtraPaths.empty())
        return ;

    qDebug() << "AutoKareBackupProxy::extraDataBackup invoke begin";

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
    QString srcPath = Utils::getBackupRootPath();
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

    qDebug() << "AutoKareBackupProxy::extraDataBackup invoke end";
}

/**
 * @brief 计算额外备份数据所需空间大小
 * @return 以字节为单位的空间大小
 * @note  由于当前额外备份数据量很小，瞬间完成，故暂时先以同步阻塞方式进行
 */
int AutoKareBackupProxy::calcSizeForExtraBackup()
{
    qDebug() << "AutoKareBackupProxy::calcSizeForExtraBackup";

    // 拼接备份源路径和目标路径，测试所需备份空间大小；目标路径为一虚拟路径
    QString srcPath = Utils::getBackupRootPath();
    srcPath += "/";
    srcPath.replace("//", "/");
    QString destPath = Utils::getBackupRootPath();
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

bool AutoKareBackupProxy::cpFile(const QString &filePath)
{
    // 检查文件是否存在
    if (!QFileInfo::exists(filePath)) {
        qWarning() << "文件不存在:" << filePath;
        return false;
    }

    // 构造备份文件名（原路径 + .bak）
    QString backupPath = filePath + ".bak";

    // 如果备份文件已存在，先删除旧的备份
    if (QFileInfo::exists(backupPath)) {
        if (!QFile::remove(backupPath)) {
            qWarning() << "无法删除旧备份文件:" << backupPath;
            return false;
        }
    }

    // 复制文件
    if (!QFile::copy(filePath, backupPath)) {
        qWarning() << "复制备份失败:" << filePath << "->" << backupPath;
        return false;
    }

    qDebug() << "备份成功:" << backupPath;
    return true;
}


/**
 * @brief 删除ostree 中对应的branch和commit
 */
bool AutoKareBackupProxy::deleteBackupPointCommits(ParseBackupList::BackupPoint backupPoint)
{
    qDebug() << "AutoKareBackupProxy::deleteBackupPointCommits invoke start";

    qDebug()<<"删除系统升级备份";

    // 初始化ostree
    m_pOstree = new OstreeInterface(this);
    //设置工作类型为备份点管理
    m_pOstree->setTaskType(OstreeInterface::TaskType::BACKUPPOINT_MANAGEMENT);
    qDebug() << "设置工作类型为:备份点管理" ;

    //检查repo点是否存在
    if(!m_pOstree->checkOstreeRepo("",OstreeInterface::BackupType::SYSTEM_UPDATE))
    {
        qDebug() << "备份点ostree仓库目录不存在";
        m_pOstree->setTaskType(OstreeInterface::TaskType::IDLE);
        return false;
    }

    //获取ostree仓库目录
    QString repoPath = m_pOstree->getRepoPath("", OstreeInterface::BackupType::SYSTEM_UPDATE);

    QStringList pathNameList = {"usr","etc","var","home"};

    // 第一步：删除分支，解除引用
    QStringList refsArgs = m_pOstree->getRefsArgs(repoPath, backupPoint.m_branchName);
    m_pOstree->setOperation(OstreeInterface::Operation::REFS);
    qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::REFS);
    m_pOstree->start(refsArgs, true);

    // 第二步：删除4个commit对应的目录
    for (auto dirPath : pathNameList)
    {
        QString commitID = backupPoint.m_systemCommitHashMap.value(dirPath);
        QStringList pruneArgs = m_pOstree->getPruneArgs(repoPath, commitID);
        m_pOstree->setOperation(OstreeInterface::Operation::PRUNE);
        qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::PRUNE);
        m_pOstree->start(pruneArgs, true);
    }

    //执行完毕，设置工作类型为空闲
    m_pOstree->setTaskType(OstreeInterface::TaskType::IDLE);

    qDebug() << "AutoKareBackupProxy::deleteBackupPointCommits invoke end";
    return true;
}
