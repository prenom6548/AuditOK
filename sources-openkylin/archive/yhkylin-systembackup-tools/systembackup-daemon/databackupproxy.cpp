/*
 *  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
 *  yhkylin-systembackup-tools is licensed under GPL-2+
 *  See LICENSE file for more details.
 */

#include "databackupproxy.h"
#include <QStorageInfo>
#include <QDateTime>
#include <QDebug>
#include <QTimer>
#include <QRegularExpression>

#include "../common/utils.h"
#include "../common/mydusizetool.h"
#include "mymountproxy.h"

IMPLEMENT_DYNCREATE(DataBackupProxy)

DataBackupProxy::DataBackupProxy()
{
    m_bSuccess = false;
    m_isFinished = false;
    m_size = 0;
    m_calc = new CalcBackupSize(this);
    m_isOnlyCheck = true;
    m_mksquashfs = nullptr;
    m_p = nullptr;
    m_calcExtra = nullptr;

    //m_bCancel = false;
 //   m_rsync = nullptr;

    connect(this, &DataBackupProxy::cancel, this, &DataBackupProxy::cancelEx);
}

DataBackupProxy::~DataBackupProxy()
{
    delete m_calc;
    m_calc = nullptr;

    delete m_mksquashfs;
    m_mksquashfs = nullptr;

    delete m_p;
    m_p = nullptr;

 //   delete m_rsync;
 //   m_rsync = nullptr;

    // 如果备份失败，则删除备份数据
    if (!m_bSuccess)
    {
        deleteFailedData();
    }
}

void DataBackupProxy::loadExistsBackupPoint()
{
    QString uniXmlPath = Utils::getBackupRootPath() + BACKUP_XML_PATH;
    uniXmlPath.replace("//", "/");
    ParseBackupList uniParse(uniXmlPath);
    m_backupPoint = uniParse.findBackupPointByUuid(m_curUuid);
    QString xmlPath = Utils::getBackupRootPath() + SYS_BACKUP_XML_PATH;
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
bool DataBackupProxy::checkEnvEx()
{
    qDebug() << "DataBackupProxy::checkEnv invoke begin";

    m_backupWrapper.m_type = BackupType::BACKUP_DATA;

    m_calcMethod = MKSQUASHFS_COMPRESS;
    //loadExistsBackupPoint();

    // 计算备份大小 NONCOMPRESS_INC>NONCOMPRESS_FULL>MKSQUASHFS_COMPRESS
    m_hasConnectCalcProcess = false;
    bool bExist = Utils::isDirExists(m_backupWrapper.m_prefixDestPath);
    if(!bExist)
    {
        if (!Utils::mkpath(m_backupWrapper.m_prefixDestPath))
        {
            qCritical() << QString("mkdir %1 failed !").arg(m_backupWrapper.m_prefixDestPath) ;
        }
    }

    /*
     * 如果是U盘，判断是否是vfat或udf类型，如果是，返回错误
    */
    if(m_backupWrapper.m_iPosition == 1)
    {
        qDebug()<<"dest path "<< m_backupWrapper.m_prefixDestPath;
        QStorageInfo storage(m_backupWrapper.m_prefixDestPath);
        QString fsType = storage.fileSystemType().toLower();
        if(fsType != "ext4")
        {    
            emit checkResult(int(BackupResult::UDISK_FILESYSTEM_TYPE_IS_NOT_EXT4));
            return false;
        }
    }
    calcSizeForBackup();

    qDebug() << "DataBackupProxy::checkEnv invoke end";
    return true;
}

/**
 * @brief 执行备份逻辑
 */
void DataBackupProxy::doWorkEx()
{
    qDebug() << "DataBackupProxy::doWorkEx invoke begin";

    if (!doPrepare())
    {
        qDebug()<<"data backup prepare failed";
        return ;
    }

    // 4、开始备份系统
//    bool ret =backupData();
//    if(!ret){
//        emit this->workResult(false);
//    }
    backupData();

    //qDebug() << "DataBackupProxy::checkFreeCapacity invoke end";
   // checkEnvEx();

    qDebug() << "DataBackupProxy::doWorkEx invoke end";
}

void DataBackupProxy::cancelEx()
{
    qDebug() << "DataBackupProxy::cancelEx invoke begin";

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

    qDebug() << "DataBackupProxy::cancelEx invoke begin";
}

/**
 * @brief 失败则删除相应数据
 */
void DataBackupProxy::deleteFailedData()
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
void DataBackupProxy::checkFreeCapacity(qint64 itotalSize)
{
    qDebug() << "DataBackupProxy::checkFreeCapacity invoke begin";

    // 如果是取消了操作，则不再发送其它信息
    if (m_bCancel)
        return ;

    // 备份过程中会有一些临时文件产生，会占用一部分空间，故我们预留500M的空间
    itotalSize += 500 * MB;
    qDebug() << "备份数据大小+500M预留空间约为：" << itotalSize << "字节";

    qint64 leftsize = Utils::getLeftSize(m_backupWrapper.m_prefixDestPath);
    qDebug()<<"data left size "<< leftsize;
    if (leftsize < itotalSize) {
        emit checkResult(int(BackupResult::BACKUP_CAPACITY_IS_NOT_ENOUGH));
    }else{
        emit checkResult(int(BackupResult::CHECK_ENV_SUCCESS));
    }

    /*
    if (m_isOnlyCheck)
        return ;

    // 3、准备
    if (!doPrepare())
        return ;

    // 4、开始备份系统
    backupData();
*/
    qDebug() << "DataBackupProxy::checkFreeCapacity invoke end";
}

/**
 * @brief 计算备份所需空间大小
 * @return 计算备份所需空间大小,单位字节
 */
void DataBackupProxy::calcSizeForBackup()
{
    /*QString userName = qgetenv("USER");

    if (!m_hasConnectCalcProcess){
        m_hasConnectCalcProcess = true;
        connect(m_calc, &CalcBackupSize::finished, this, &DataBackupProxy::checkFreeCapacity);
    }
*/
    QStringList filelist = m_backupWrapper.m_backupPaths;
    for (const QString &path : filelist) {
        qint64 size = Utils::getDirectorySize(path);
        if (size < 0) {
            qWarning() << "计算路径大小失败：" << path;
            return ;
        }
        m_size += size;
        qDebug() << "路径：" << path << "，大小：" << size << "字节";
    }
   // m_calc->calcFilelist(filelist);
    checkFreeCapacity(m_size);
    //m_calc->calcByDu(args, false);
}

/**
 * @brief 根据场景获取rsync命令参数
 * @param scene，场景
 * @return rsync的参数信息
 */
QStringList DataBackupProxy::getRsyncArgs(AutoSystemBackupScene scene)
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
void DataBackupProxy::doMksqushfs()
{
    qDebug() << "DataBackupProxy::doMksqushfs invoke begin";

    m_mksquashfs = new MkSquashFSProcess(this);
    connect(m_mksquashfs, &MkSquashFSProcess::progress, this, &DataBackupProxy::progress);
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

    qDebug() << "DataBackupProxy::doMksqushfs invoke end";
}

/**
 * @brief 备份系统
 * @return true，备份成功；false，备份失败
 */
bool DataBackupProxy::backupData()
{
    qDebug() << "DataBackupProxy::backupData invoke begin";

    if(!m_pOstree){
        m_pOstree = new OstreeInterface(this);
    }
    //设置工作类型为备份
    m_pOstree->setTaskType(OstreeInterface::TaskType::BACKUP);
    qDebug() << "设置工作类型为:备份" ;

    QString m_dir = m_backupWrapper.m_prefixDestPath;
    qDebug()<<"backup location "<<m_dir;
    repoPath = m_pOstree->getRepoPath(m_dir, OstreeInterface::BackupType::USER);


    connect(m_pOstree, &OstreeInterface::startOstreeInterfaceOperation, this, &DataBackupProxy::startOstreeInterfaceOperation);
    //检查repo点是否存在
    if(!m_pOstree->checkOstreeRepo(repoPath,OstreeInterface::BackupType::USER))
    {
        //不存在则创建
        m_pOstree->initOstreeRepo(repoPath,OstreeInterface::BackupType::USER);
    }

    // 数据备份仓库路径去除单引号，防止commit失败
    if( repoPath.startsWith("\'") && repoPath.endsWith("\'") )
    {
        repoPath = repoPath.mid(1,repoPath.length()-2);
    }

    qDebug() << "用户数据备份操作:开始";

    //先设定备份步骤，用于计算备份进度
    //m_pOstree->setBackUpStep("home");

    //获取用户名以及目录
    //QString userName = qgetenv("USER");
   // QString userName = m_backupWrapper.m_frontUserName;
   // QString userBasePath = DEFAULT_HOME_PATH + userName;
    QString description;
    if(Utils::hasChinese(m_backupWrapper.m_backupName))
    {
        QString tempName = "BackupPoint";
        description = "\"" + tempName + "\"";
    }
    else
    {
        description = "\"" + m_backupWrapper.m_backupName + "\"";
    }

    QStringList all_Paths = m_backupWrapper.m_backupPaths;

    m_backupPoint.m_dataCommitHashMap.clear();

    // 分离文件和目录
    QStringList filesToBackup;
    QStringList dirsToBackup;

    //包含用户目录下的内容，会包含.box,需要额外处理
    QStringList includeBoxPaths;
    QStringList users = Utils::getAllUsers();
    bool ifIncludeBox ;

    for (const QString &path : all_Paths)
    {
       ifIncludeBox = false;
       QFileInfo fi(path);
       if (!fi.exists()) {
           qWarning() << "Path does not exist:" << path;
           continue;
       }

       //如果路径为/data/，/home/kylin下，可能会包含.box目录导致备份失败
       for(const QString &user : users)
       {
           QString userPath = "/home/" + user;
           if(path == userPath)
           {
               ifIncludeBox = true;
               break;
           }
       }

       if(fi.isDir())
       {
           if(ifIncludeBox)
           {
               includeBoxPaths.append(path);
           }
           else
           {
               dirsToBackup.append(path);
           }

       }
       else
       {
           filesToBackup.append(path);
       }
    }

    m_totalItems = filesToBackup.size() + dirsToBackup.size();
    int m_progress = 0;

    // 提交目录（直接提交）
    QString commit = "";
    qDebug()<<"branch ### "<< m_backupPoint.m_branchName;
    for (const QString &dirPath : dirsToBackup)
    {
       QString tmp_path = normalizePath(dirPath);
       //预先清理SBC文件
       cleanSBCFilesByOstreeClass(dirPath);
       commit = commitDirectory(repoPath, m_backupPoint.m_branchName, tmp_path, description);
       qDebug()<<"##path "<< dirPath << " ####commit " <<commit;
       if(commit.isEmpty()){
           qDebug()<<"directory "<< dirPath <<" commit failed,reutrn";
           if (!m_bCancel)
               emit this->workResult(false);
           return false;
       }else{
           // 记录目录信息
           entry.isDirectory = true;
           entry.hash = commit;
           m_backupPoint.m_dataCommitHashMap[dirPath] = entry;
           updateProgress(++m_progress);
       }
    }

    // 提交文件（从临时目录提交）
    if (!filesToBackup.isEmpty())
    {
        //清空临时目标文件夹
        if(!clearDirectory(HARD_LINK_PATH)) {
            emit this->workResult(false);
            return false;
        }

        QString filesPath = "";

        //对所有分散文件做硬链接到指定路径下
        for (const QString &filePath : filesToBackup)
        {
            qDebug()<<"文件名： "<< filePath;
            linkFile(filePath);
            filesPath += filePath + ";";
        }

        //提交
        commit = commitDirectory(repoPath, m_backupPoint.m_branchName, HARD_LINK_PATH,description);
        if(commit.isEmpty())
        {
            qDebug()<<"files commit failed";
            if (!m_bCancel)
                emit this->workResult(false);
            return false;
        }
        entry.isDirectory = false;
        entry.hash = commit;
        m_backupPoint.m_dataCommitHashMap[filesPath] = entry;
        //updateProgress(++m_progress);

    }

    // 提交文件（从临时目录提交）
    if (!includeBoxPaths.isEmpty())
    {
        qDebug()<<"包含.box目录的提交";
        // 记录文件信息（路径保持原始值）
        for (const QString &includeBoxPath : includeBoxPaths)
        {
            //清空临时目标文件夹
            if(!clearDirectory(TEMP_HOME_PATH)) {
                emit this->workResult(false);
                return false;
            }

            //对目录要备份的数据做预处理,用Rsync将部分数据导出
            m_pOstree->setRsyncScense(OstreeInterface::RsyncScense::RESTORE_HOMEDATA);
            QStringList rsyncArgs= m_pOstree->getRsyncRepoArgs(includeBoxPath,TEMP_HOME_PATH);
            m_pOstree->setOperation(OstreeInterface::Operation::RSYNC);
            qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::RSYNC);
            m_pOstree->setOperationPath(includeBoxPath);
            m_pOstree->start(rsyncArgs, true);

//            //拷贝文件到临时目的文件夹
//            if(0 != syncHomeFile(includeBoxPath,TEMP_HOME_PATH)){
//                qDebug()<<"sync file failed "<< includeBoxPath;
//                emit this->workResult(false);
//                return false;
//            }

            //预先清理SBC文件
            cleanSBCFilesByOstreeClass(includeBoxPath);
            //提交
            commit = commitDirectory(repoPath, m_backupPoint.m_branchName, TEMP_HOME_PATH,description);
            qDebug()<<"##path "<< includeBoxPath << " ####commit " <<commit;
            if(commit.isEmpty())
            {
                qDebug()<<"file commit failed" << includeBoxPath;
                if (!m_bCancel)
                    emit this->workResult(false);
                return false;
            }
            entry.isDirectory = false;
            entry.hash = commit;
            m_backupPoint.m_dataCommitHashMap[includeBoxPath] = entry;
            updateProgress(++m_progress);
        }

    }

    //执行完所有提交后，再清除临时目录
    clearDirectory(TEMP_RESTORE_PATH);

    //更新备份点

    m_backupPoint.m_state = BACKUP_PARSE_STATE_SUCCESS_STRTING;

    //计算备份之前对应的备份仓库大小，用于后期计算备份点大小
    qDebug() << "repo备份仓库:" << repoPath ;
    qint64 lastSize = Utils::getDirOrFileSize(repoPath);
    qDebug() << "repo备份仓库lastSize:" << lastSize ;
    if (m_backupWrapper.m_bIncrement) {
        // 更新备份时备份点大小：旧备份点大小+更新大小
        m_backupPoint.m_size = Utils::StringBySize( lastSize - this->m_CurrentBackupPointSize + Utils::SizeByString(m_backupWrapper.m_size));
    } else {
        m_backupPoint.m_size = Utils::StringBySize( lastSize - this->m_CurrentBackupPointSize);
    }
   // m_backupPoint.m_uuid = m_backupWrapper.m_uuid;
    qDebug() << "m_backupPoint.m_size:" << m_backupPoint.m_size ;

    QString xmlPath;
    if(m_backupPoint.m_iPosition == 0 )//本地存储
    {
        xmlPath = m_backupPoint.m_path + USER_XML;
    }else{
        //U盘路径
        xmlPath = m_backupPoint.m_path + USER_XML;
    }

   // QString xmlPath = Utils::getBackupRootPath() + m_xmlFile;
    xmlPath.replace("//", "/");
    ParseBackupList parse(xmlPath);
    parse.updateItem(m_backupPoint);

    Utils::writeBackupLog(m_backupPoint.m_time + ","
                          + m_backupPoint.m_uuid + "," + QString::number(m_backupWrapper.m_type) + ","
                          + m_backupWrapper.m_note + "," + m_backupPoint.m_size
                          + ",," + m_backupWrapper.m_backupName);
    Utils::update_backup_unique_settings(m_backupPoint.m_uuid, m_backupPoint.m_backupName);
    m_bSuccess = true;

    emit this->workResult(true);
    m_pOstree->setTaskType(OstreeInterface::TaskType::IDLE);
    qDebug() << "databackup finish....";

    return true;
}

/**
 * @brief 判断是否增量备份
 * @return true,增量备份； false，全量备份
 */
void DataBackupProxy::isIncBackup()
{
    QString xmlPath;
    if(m_backupWrapper.m_iPosition == 0 )//本地存储
    {
        xmlPath = m_backupWrapper.m_prefixDestPath + USER_XML;
    }else{
        xmlPath = m_backupWrapper.m_prefixDestPath + UDISK_XML;
    }
    qDebug()<<"inc backup, xmlpath ## ## "<< xmlPath;

    QFile file(xmlPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "无法打开文件: " << xmlPath;
        m_backupWrapper.m_bIncrement = false;
        return;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (!m_backupWrapper.m_uuid.isEmpty() && line.contains(m_backupWrapper.m_uuid)) {
            file.close();
            qDebug()<<"it's incrrrrrr data backup ";
            m_backupWrapper.m_bIncrement = true;
            return;
        }
    }

    file.close();
    qDebug()<<"it's normalllll data backup ";
    m_backupWrapper.m_bIncrement = false;
    return ;
}

/**
 * @brief 备份准备
 * @return true,准备成功；false，准备失败
 */
bool DataBackupProxy::doPrepare()
{
    qDebug() << "DataBackupProxy::doPrepare invoke begin";

    //判断是否是增量备份
    isIncBackup();

    if (m_backupWrapper.m_bIncrement) {
        m_backupWrapper.m_type = BackupType::INC_BACKUP_DATA;
    } else {
        m_backupWrapper.m_type = BackupType::BACKUP_DATA;
    }

    // 设置当前备份的Uuid
    if (m_backupWrapper.m_uuid.isEmpty()) {
        // 新增备份点，不指定uuid的场景
        m_curUuid = Utils::createUuid();
        m_curUuid.replace("{", "");
        m_curUuid.replace("}", "");
    } else {
        // 增量备份
        m_curUuid = m_backupWrapper.m_uuid;
    }

    qDebug()<<"uuid###: "<<m_curUuid;

    m_bSuccess = false;
    //1.检测ostree仓库是否存在
    m_pOstree = new OstreeInterface(this);
    m_pOstree->setOperation(OstreeInterface::Operation::INIT);
    qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::INIT);

    QString m_dir = m_backupWrapper.m_prefixDestPath;
    qDebug()<<"backup location "<<m_dir;
    if(!m_pOstree->checkOstreeRepo(m_dir,OstreeInterface::BackupType::USER))
    {
        //不存在则创建
        m_pOstree->initOstreeRepo(m_dir,OstreeInterface::BackupType::USER);
    }

    //计算备份之前对应的备份仓库大小，用于后期计算备份点大小

    repoPath = m_pOstree->getRepoPath(m_dir,OstreeInterface::BackupType::USER);
    qDebug() << "user data repo路径：" << repoPath ;
    this->m_CurrentBackupPointSize = Utils::getDirOrFileSize(repoPath);
    qDebug() << "user data repo大小：" << this->m_CurrentBackupPointSize ;

    // 2.绑定对应的信号与槽
    connect(m_pOstree, &OstreeInterface::systemHash, this, [&](QString systemHash){
        m_backupPoint.m_systemHash = systemHash;
    });

    //调用不可变接口，获取当前系统分支哈希
    QStringList systemHashArgs= m_pOstree->getSystemHashArgs();
    m_pOstree->setOperation(OstreeInterface::Operation::STATUS);
    qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::STATUS);
    m_pOstree->start(systemHashArgs, true);

    // 3、记录/data/.backup/snapshots/backuplist.xml文件
    if (!recordBackupPoint())
    {
        qCritical() << "add or update item to backuplist.xml failed !";
        return false;
    }

    qDebug() << "DataBackupProxy::doPrepare invoke end";
    return true;
}

/**
 * @brief 记录/backup/snapshots/backuplist.xml文件
 * @return true-成功；false-失败
 */
bool DataBackupProxy::recordBackupPoint()
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
    m_backupPoint.m_user = m_backupWrapper.m_frontUserName;

    //每次备份新建一个分支，先给分支名命名，命名格式：branch+时间戳
    QString timestampStr = m_backupPoint.m_time;
    QString formattedTimestamp = timestampStr.replace("-", "").replace(" ", "").replace(":", "");
    QString branchName = "branch-" + formattedTimestamp;
    qDebug() << "分支名：" << branchName;
    m_backupPoint.m_branchName = branchName;

    QString xmlPath;
    if(m_backupPoint.m_iPosition == 0 )//本地存储
    {
        xmlPath = m_backupPoint.m_path + USER_XML;
    }else{
        //U盘路径
        xmlPath = m_backupPoint.m_path + USER_XML;
    }
    qDebug()<<"record backup point xml "<< xmlPath;
   // QString xmlPath = Utils::getBackupRootPath() + m_xmlFile;
    xmlPath.replace("//", "/");
    qDebug() << xmlPath;
    ParseBackupList parse(xmlPath);

    if (m_backupWrapper.m_bIncrement)
    {
        qDebug()<<"it's incre ment when record point";
        qDebug()<<"uuidddd "<< m_backupPoint.m_uuid;
        // 先不更新，备份成功之后再更新
//        if (parse.updateItem(m_backupPoint) != ParseBackupList::SUCCESS) {
//            emit checkResult(int(BackupResult::WRITE_STORAGEINFO_UPDATE_ITEM_FAIL));
//            return false;
//        }
    } else
    {
        qDebug()<<"it's normal when record point";
        qDebug()<<"uuidddd "<< m_backupPoint.m_uuid;
        if (parse.addItem(m_backupPoint) != ParseBackupList::SUCCESS) {
            emit checkResult(int(BackupResult::WRITE_STORAGEINFO_ADD_ITEM_FAIL));
            return false;
        }
    }

    return true;
}

/**
 * @brief 删除指定备份点
 */
void DataBackupProxy::removeAutoBackupPoint(const QString& uuid)
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
QStringList DataBackupProxy::extraDataPaths()
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
void DataBackupProxy::extraDataBackup()
{
    if (m_backExtraPaths.empty())
        return ;

    qDebug() << "DataBackupProxy::extraDataBackup invoke begin";

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

    qDebug() << "DataBackupProxy::extraDataBackup invoke end";
}

/**
 * @brief 计算额外备份数据所需空间大小
 * @return 以字节为单位的空间大小
 * @note  由于当前额外备份数据量很小，瞬间完成，故暂时先以同步阻塞方式进行
 */
int DataBackupProxy::calcSizeForExtraBackup()
{
    qDebug() << "DataBackupProxy::calcSizeForExtraBackup";

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

void DataBackupProxy::cleanSBCFilesByOstreeClass(QString dirPath)
{
    if(!m_pOstree){
        m_pOstree = new OstreeInterface(this);
    }
    //清理特殊文件，比如套接字文件，s
    QStringList findArgs= m_pOstree->getFindArgs(dirPath,1);
    m_pOstree->setOperation(OstreeInterface::Operation::FIND);
    qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::FIND);
    m_pOstree->start(findArgs, true);

    findArgs= m_pOstree->getFindArgs(dirPath,2);
    m_pOstree->start(findArgs, true);

    findArgs= m_pOstree->getFindArgs(dirPath,3);
    m_pOstree->start(findArgs, true);
}

void DataBackupProxy::cleanSBCFiles(QString dirPath)
{
    QProcess clean;
    clean.setProgram("find");
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("LANG", "en_US.UTF-8");  // 或 "zh_CN.UTF-8"
    env.insert("LC_ALL", "en_US.UTF-8");
    clean.setProcessEnvironment(env);

    QStringList s_args = {
        dirPath,
        "-type",
        "s",
        "-exec",
        "rm",
        "-f",
        "{}" ,
        "\\;"
    };

    QStringList c_args = {
        dirPath,
        "-type",
        "c",
        "-exec",
        "rm",
        "-f",
        "{}" ,
        "\\;"
    };

    QStringList b_args = {
        dirPath,
        "-type",
        "b",
        "-exec",
        "rm",
        "-f",
        "{}" ,
        "\\;"
    };

    // 打印完整命令
    QString fullCommand = "find " + s_args.join(" ");
    qDebug() << "Executing:" << fullCommand;

    // 设置参数并执行
    clean.setArguments(s_args);
    clean.start();

    if (!clean.waitForFinished(-1) || clean.exitCode() != 0)
    {
        qWarning() << "clean S failed:" << clean.errorString() ;
    }

    clean.setArguments(c_args);
    clean.start();

    if (!clean.waitForFinished(-1) || clean.exitCode() != 0)
    {
        qWarning() << "clean C failed:" << clean.errorString() ;
    }

    clean.setArguments(b_args);
    clean.start();

    if (!clean.waitForFinished(-1) || clean.exitCode() != 0)
    {
        qWarning() << "clean B failed:" << clean.errorString() ;
    }
    return ;
}


QString DataBackupProxy::commitDirectory(const QString &repoPath, const QString &branchName, const QString &dirPath, const QString &description)
{
    QString encodedPath = QDir::toNativeSeparators(dirPath);
    QProcess ostree;
    ostree.setProgram("ostree");
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("LANG", "en_US.UTF-8");  // 或 "zh_CN.UTF-8"
    env.insert("LC_ALL", "en_US.UTF-8");
    ostree.setProcessEnvironment(env);

    QStringList args = {
        "commit",
        "--repo", repoPath,
        "--branch", branchName,
        "--skip-spec",
        "--verbose",
        "-s", description,
        "--tree=dir=" + dirPath
    };

    // 打印完整命令
    QString fullCommand = "ostree " + args.join(" ");
    qDebug() << "Executing:" << fullCommand;

    // 开始执行ostree命令
    emit startOstreeInterfaceOperation();

    connect(&ostree, &QProcess::readyReadStandardError, [&](){
        qDebug() << "[OSTree Error] " << ostree.readAllStandardError();
    });

    // 设置参数并执行
    ostree.setArguments(args);
    qDebug() << "m_bCancel:" << m_bCancel;
    if (!m_bCancel)
    {
        ostree.start();
    }
    else
    {
        qDebug() << "备份过程已取消:";
        return "";
    }


  //  ostree.start();
    if (!ostree.waitForFinished(-1) || ostree.exitCode() != 0) {
        qWarning() << "OSTree commit failed:" << ostree.errorString() <<" current path "<<dirPath;
        qDebug() << "Exit Code:" << ostree.exitCode();
        if (ostree.exitCode() == 9){
            // 取消备份，进程已被强制杀死
            m_bCancel = true;
        }

        return "";
    }

    QString out(ostree.readAll());
    QStringList lines = out.split("\n");
    qDebug() <<"path "<< dirPath<< " commit value: "<< out;

    if(out.isEmpty())
    {
        return "" ;
    }

    return lines.at(0);
}

bool DataBackupProxy::clearDirectory(const QString &dirPath)
{
    QDir dir(dirPath);

    // 如果目录不存在，则创建
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            qWarning() << "Failed to create directory:" << dirPath;
            return false;
        }
        qDebug() << "Directory created:" << dirPath;
        return true; // 新创建的目录本身就是空的
    }

    // 使用系统命令快速清空目录（包括隐藏文件）
    int ret = QProcess::execute("bash", {
        "-c",
        QString("shopt -s dotglob && rm -rf %1/*").arg(dirPath)
    });

    if (ret != 0) {
        qWarning() << "Failed to clear directory (command exited with" << ret << ")";
        return false;
    }

    // 验证目录是否为空（排除 . 和 ..）
    bool isEmpty = dir.entryList(QDir::NoDotAndDotDot | QDir::AllEntries).isEmpty();
    if (!isEmpty) {
        qWarning() << "Directory is not empty after clearing:" << dirPath;
    }
    return isEmpty;
}

int DataBackupProxy::linkFile(QString src)
{
    // 验证路径是否存在
    QFileInfo fileInfo(src);

    if (!fileInfo.exists()) {
        qWarning() << "Source file does not exist:" << src;
        return -1;
    }

    //获取文件名
    QString fileName = fileInfo.fileName();

    //如果源文件不是/data开头，需要补齐，否则无法实现硬链接
    if(!src.startsWith("/data/"))
    {
        QString prePath = "/data/";
        src = prePath + src;
    }

    // 构建ln命令
    QStringList args;
    QString destPath = HARD_LINK_PATH + fileName;
    args << src << destPath;

    QProcess m_process;
    m_process.start("ln", args);

    qDebug() << "硬链接地址：" << destPath;

    if (!m_process.waitForStarted()) {
        qCritical() << "Failed to start rsync process";
        return -1;
    }

    if (!m_process.waitForFinished(-1)) {
        qCritical() << "rsync process failed to finish";
        return -1;
    }

    return m_process.exitCode();
}

int DataBackupProxy::syncFile(const QString &src, const QString &dest)
{
    // 验证路径是否存在
    if (!QFileInfo(src).exists()) {
        qWarning() << "Source file does not exist:" << src;
        return -1;
    }

    // 构建rsync命令
    QStringList args;
    args << "-aAHX" << src << dest;

    QProcess m_process;
    m_process.start("rsync", args);

    if (!m_process.waitForStarted()) {
        qCritical() << "Failed to start rsync process";
        return -1;
    }

    if (!m_process.waitForFinished(-1)) {
        qCritical() << "rsync process failed to finish";
        return -1;
    }

    return m_process.exitCode();
}

int DataBackupProxy::syncHomeFile(const QString &src, const QString &dest)
{
    // 验证路径是否存在
    if (!QFileInfo(src).exists()) {
        qWarning() << "Source file does not exist:" << src;
        return -1;
    }

    // 构建rsync命令
    QStringList args;
    args << "-aAHX" << "--include='*\/.*' --include='*\/*' --exclude='.*' --include='*'" << src + "/" << dest;

//    args << "-avAHXr";
//    args << "--info=progress2";
//    args << "--no-inc-recursive";
//    args << "--ignore-missing-args";
//    args << "--delete";
//    args << "--include=*/.*";
//    args << "--include=*/*";
//    args << "--exclude=.*";
//    args << "--include=*";

    qDebug() << args;
    QProcess m_process;
    m_process.start("rsync", args);
//    QString cmd("rsync");
//    for (const QString& item : args) {
//       cmd += " ";
//       cmd += item;
//    }
//    m_process.start("bash", QStringList() << "-c" << cmd);

    if (!m_process.waitForStarted()) {
        qCritical() << "Failed to start rsync process";
        return -1;
    }

    if (!m_process.waitForFinished(-1)) {
        qCritical() << "rsync process failed to finish";
        return -1;
    }

    return m_process.exitCode();
}

void DataBackupProxy::updateProgress(int processed)
{
    int percent = static_cast<int>((processed * 100.0) / m_totalItems);
    emit progress(percent);
}

QString DataBackupProxy::normalizePath(const QString &path)
{
    return QDir::toNativeSeparators(path).normalized(QString::NormalizationForm_C);
}
