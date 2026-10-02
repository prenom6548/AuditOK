/*
 *  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
 *  yhkylin-systembackup-tools is licensed under GPL-2+
 *  See LICENSE file for more details.
 */

 #include "systembackupproxy.h"
 #include <QStorageInfo>
 #include <QDateTime>
 #include <QDebug>
 #include <QTimer>
 #include <QRegularExpression>
 
 #include "../common/utils.h"
 #include "../common/mydusizetool.h"
 #include "mymountproxy.h"
 
 IMPLEMENT_DYNCREATE(SystemBackupProxy)
 
 SystemBackupProxy::SystemBackupProxy()
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
 
     connect(this, &SystemBackupProxy::cancel, this, &SystemBackupProxy::cancelEx);
 }
 
 SystemBackupProxy::~SystemBackupProxy()
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
 
 void SystemBackupProxy::loadExistsBackupPoint()
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
 bool SystemBackupProxy::checkEnvEx()
 {
     qDebug() << "SystemBackupProxy::checkEnv invoke begin";
 
     // 自动系统备份AUTO_BACKUP_SYSTEM也归为系统备份BACKUP_SYSTEM
     m_backupWrapper.m_type = BackupType::BACKUP_SYSTEM;
     m_curUuid += m_backupWrapper.m_uuid;
     m_xmlFile = SYS_BACKUP_XML_PATH;
 
     m_calcMethod = MKSQUASHFS_COMPRESS;
     //loadExistsBackupPoint();
 
     // 3、计算备份大小 NONCOMPRESS_INC>NONCOMPRESS_FULL>MKSQUASHFS_COMPRESS
     m_hasConnectCalcProcess = false;

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
     qDebug()<<"backup path## "<< m_backupWrapper.m_prefixDestPath;

     if (Utils::getSizeFromFile(m_totalSize, m_leftSize)) {
         // 成功读取，myTotalSize和myLeftSize已经被赋值
         qDebug() << "total: " << m_totalSize << "left: " << m_leftSize;
     } else {
         qDebug() << "read size failed.";
     }

     checkFreeCapacity(m_totalSize);

     qDebug() << "SystemBackupProxy::checkEnv invoke end";
     return true;
 }
 
 /**
  * @brief 执行备份逻辑
  */
 void SystemBackupProxy::doWorkEx()
 {
     qDebug() << "SystemBackupProxy::doWorkEx invoke begin";
 
     m_isOnlyCheck = false;
     // 环境检测
     checkEnvEx();
 
     qDebug() << "SystemBackupProxy::doWorkEx invoke end";
 }
 
 void SystemBackupProxy::cancelEx()
 {
     qDebug() << "SystemBackupProxy::cancelEx invoke begin";

     // m_bCancel = true;
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

     qDebug() << "SystemBackupProxy::cancelEx invoke end";
 }
 
 /**
  * @brief 失败则删除相应数据
  */
 void SystemBackupProxy::deleteFailedData()
 {
     qDebug() << "SystemBackupProxy::deleteFailedData invoke begin";

     if (m_curUuid.isEmpty())
         return;
 
     // // 1、删除备份目录
     // QString destPath = m_backupWrapper.m_prefixDestPath + BACKUP_SNAPSHOTS_PATH + "/" + m_curUuid;
     // destPath.replace("//", "/");
     // QStringList args;
     // args << "-rf";
     // args << destPath;
     // QProcess::execute("rm", args);
 
     // 2、删除xml文件中的备份项
     QString xmlPath = Utils::getBackupRootPath() + m_xmlFile;
     xmlPath.replace("//", "/");
     ParseBackupList parse(xmlPath);
     parse.deleteItem(m_curUuid);
     if (parse.deleteItem(m_curUuid) != ParseBackupList::SUCCESS) {
         qCritical() << QString("failed to delete %1 from backuplist.xml or sys_update_backuplist.xml!").arg(m_curUuid);
     } else {
         qDebug() << "deleteXmlBackupPoint succeeded! ";
     }

     qDebug() << "SystemBackupProxy::deleteFailedData invoke end";
 }
 
 /**
  * @brief 校验剩余空间是否满足备份
  */
 void SystemBackupProxy::checkFreeCapacity(qint64 itotalSize)
 {
     qDebug() << "SystemBackupProxy::checkFreeCapacity invoke begin";
 
     // 如果是取消了操作，则不再发送其它信息
     if (m_bCancel)
         return ;

     // 1、计算待备份数据的大小
     m_size = itotalSize;
     // 备份过程中会有一些临时文件产生，会占用一部分空间，故我们预留500M的空间
     itotalSize += 500 * MB;
     qDebug() << "备份数据大小+500M预留空间约为：" << itotalSize << "字节";

     qDebug()<<"system left size "<< m_leftSize;
     qDebug()<<"itotalSize "<< itotalSize;

     if (m_leftSize < itotalSize) {
         emit checkResult(int(BackupResult::BACKUP_CAPACITY_IS_NOT_ENOUGH));
         return;
     }else{
         emit checkResult(int(BackupResult::CHECK_ENV_SUCCESS));
     }

     if (m_isOnlyCheck)
         return ;
 
     // 3、准备
     if (!doPrepare())
         return ;
 
     // 4、开始备份系统
     backupSystem();
 
     qDebug() << "SystemBackupProxy::checkFreeCapacity invoke end";
 }
 
 /**
  * @brief 计算备份所需空间大小
  * @return 计算备份所需空间大小,单位字节
  */
 void SystemBackupProxy::calcSizeForBackup()
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
   // process.setProgram("ostree");
    QString fullCommand = "ostree " + args.join(" ");
    qDebug() << "fetching ostree commit id ## " << fullCommand;


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
 
 qint64 SystemBackupProxy::calcSizeForHome()
 {
     QString userName = qgetenv("USER");
     QString homePath = "/home/" + m_backupWrapper.m_frontUserName;;
     qDebug()<<"homepath is "<<homePath;
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
 QStringList SystemBackupProxy::getRsyncArgs(AutoSystemBackupScene scene)
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
 void SystemBackupProxy::doMksqushfs()
 {
     qDebug() << "SystemBackupProxy::doMksqushfs invoke begin";
 
     m_mksquashfs = new MkSquashFSProcess(this);
     connect(m_mksquashfs, &MkSquashFSProcess::progress, this, &SystemBackupProxy::progress);
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
 
     qDebug() << "SystemBackupProxy::doMksqushfs invoke end";
 }
 
 /**
  * @brief 备份系统
  * @return true，备份成功；false，备份失败
  */
 bool SystemBackupProxy::backupSystem()
 {
     qDebug() << "SystemBackupProxy::backupSystem invoke begin";
 
     //
     m_pOstree = new OstreeInterface(this);
     //设置工作类型为备份
     m_pOstree->setTaskType(OstreeInterface::TaskType::BACKUP);
     qDebug() << "设置工作类型为:备份" ;


     QString repoDir;
     if (m_backupWrapper.m_iPosition == BackupPosition::LOCAL)
         repoDir = m_backupWrapper.m_prefixDestPath + ".backup/";
     else if (m_backupWrapper.m_iPosition == BackupPosition::UDISK)
         repoDir = m_backupWrapper.m_prefixDestPath + "/" + ".backup/";
 
     QString repoPath = m_pOstree->getRepoPath(repoDir, OstreeInterface::BackupType::SYSTEM);
 
     // 绑定对应的信号与槽
     m_rsync = new RsyncPathToDirProcess(this);
     connect(m_pOstree, &OstreeInterface::progress, this, &SystemBackupProxy::progress, Qt::DirectConnection);
     connect(m_pOstree, &OstreeInterface::commitHash, this, [&](QString path, QString commitHash){
         m_backupPoint.m_systemCommitHashMap.insert(path,commitHash);
     });
     connect(m_pOstree, &OstreeInterface::finished, this,  [&](bool result) {
         // 如果是取消了操作，则不再发送其它信息
         // if (m_bCancel)
         //     return ;
         if (!result) {
             // 取消备份或者拔出U盘（目前共用一个标志位）
             m_bCancel = true;
             return;
         }
 
         OstreeInterface::Operation oper =  m_pOstree->getOperation();
         QString step = m_pOstree->getBackUpStep();
         if(oper == OstreeInterface::Operation::RM && step == "home")
         {
             qDebug() << "备份运行结束！";
 
             //补足进度信号，并等待3秒
             emit progress(100);
             Utils::wait(1);
 
 
             m_isFinished = true;
             if (result)
             {
                 m_backupPoint.m_state = BACKUP_PARSE_STATE_SUCCESS_STRTING;
 
                 //计算备份之前对应的备份仓库大小，用于后期计算备份点大小
                 qDebug() << "repo备份仓库:" << repoPath ;
                 qint64 lastSize = Utils::getDirOrFileSize(repoPath);
                 qDebug() << "repo备份仓库lastSize:" << lastSize ;

                 //用于控制外介质备份数据可能的数据波动导致的负差值
                 qint64 delta = lastSize - this->m_CurrentBackupPointSize;
                 if(delta < 0 )
                 {
                     delta = 0;
                 }

                 m_backupPoint.m_size = Utils::StringBySize(delta);
                 qDebug() << "m_backupPoint.m_size:" << m_backupPoint.m_size ;
 
                 //QString xmlPath = Utils::getBackupRootPath() + m_xmlFile;

                 QString xmlPath;
                 if(m_backupPoint.m_iPosition == 0 )//本地存储
                 {
                     xmlPath = Utils::getBackupRootPath() + m_xmlFile;
                 }else{
                     //U盘路径
                     xmlPath = m_backupPoint.m_path + "/.backup" + UDISK_XML;
                 }

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
         }
     });
     connect(m_pOstree, &OstreeInterface::startOstreeInterfaceOperation, this, &SystemBackupProxy::startOstreeInterfaceOperation);
 
     //针对系统可变目录的备份操作
     qDebug() << "针对系统可变目录的备份操作:开始";
     //检查repo点是否存在
     if(!m_pOstree->checkOstreeRepo(repoDir,OstreeInterface::BackupType::SYSTEM))
     {
         //不存在则创建
         m_pOstree->initOstreeRepo(repoDir,OstreeInterface::BackupType::SYSTEM);
     }

     //--------------------------------直接提交commit ovl数据 START-------------------------------


     //对多个路径分别执行备份
     for(auto dirPath:m_pathNameList)
     {
         //先设定备份步骤，用于计算备份进度
         m_pOstree->setBackUpStep(dirPath);
         qDebug() << "设置备份步骤为:" << dirPath ;

         //拼接需要执行备份的ovl目录
         QString ovlPath = "";
         ovlPath = ovlPath + "/ostree/pkgs/" + "ovl-" + m_backupPoint.m_systemHash + "/";

         //执行commit操作

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

         QStringList commitArgs;
         commitArgs= m_pOstree->getCommitArgs(repoPath,m_backupPoint.m_branchName,ovlPath,description);
         m_pOstree->setOperation(OstreeInterface::Operation::COMMIT);
         qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::COMMIT);
         qDebug() << "diaoyong Thread ID:" << QThread::currentThreadId();
         m_pOstree->setOperationPath(dirPath);
         if (!m_bCancel)
             m_pOstree->start(commitArgs, true);
         else
             return false;
     }

     //--------------------------------直接提交commit ovl数据 END-------------------------------
     qDebug() << "针对系统可变目录的备份操作:结束";
 
     //针对家目录的备份操作
     qDebug() << "针对家目录的备份操作:开始";
 
     //先设定备份步骤，用于计算备份进度
     m_pOstree->setBackUpStep("home");
 
     //获取用户名以及目录
     //QString userName = qgetenv("USER");
     QString userName = m_backupWrapper.m_frontUserName;
     QString userBasePath = DEFAULT_HOME_PATH + userName;
 
     //对目录要备份的数据做预处理,用Rsync将部分数据导出
     m_pOstree->setRsyncScense(OstreeInterface::RsyncScense::RESTORE_HOMEHIDDENDATA);
     QStringList rsyncArgs= m_pOstree->getRsyncRepoArgs(userBasePath,TEMP_HOME_PATH);
     m_pOstree->setOperation(OstreeInterface::Operation::RSYNC);
     qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::RSYNC);
     m_pOstree->setOperationPath(userBasePath);
     if (!m_bCancel)
         m_pOstree->start(rsyncArgs, true);
     else
         return false;
 
     //清理特殊文件，比如套接字文件，s
     QStringList findArgs= m_pOstree->getFindArgs(TEMP_HOME_PATH,1);
     m_pOstree->setOperation(OstreeInterface::Operation::FIND);
     qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::FIND);
     if (!m_bCancel)
         m_pOstree->start(findArgs, true);
     else
         return false;
 
     findArgs= m_pOstree->getFindArgs(TEMP_HOME_PATH,2);
     if (!m_bCancel)
         m_pOstree->start(findArgs, true);
     else
         return false;
 
     findArgs= m_pOstree->getFindArgs(TEMP_HOME_PATH,3);
     if (!m_bCancel)
         m_pOstree->start(findArgs, true);
     else
         return false;
 
     //制作备份点，执行commit操作
 
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
 
     QStringList commitArgs;
     commitArgs= m_pOstree->getCommitArgs(repoPath,m_backupPoint.m_branchName,TEMP_HOME_PATH,description);
     m_pOstree->setOperation(OstreeInterface::Operation::COMMIT);
     qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::COMMIT);
     m_pOstree->setOperationPath("/home");
     if (!m_bCancel)
         m_pOstree->start(commitArgs, true);
     else
         return false;
 
     //清理临时home目录
     //执行rm操作
     QStringList rmArgs= m_pOstree->getRmArgs(TEMP_HOME_PATH);
     m_pOstree->setOperation(OstreeInterface::Operation::RM);
     qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::RM);
     if (!m_bCancel)
         m_pOstree->start(rmArgs, true);
     else
         return false;
 
     qDebug() << "针对家目录的备份操作:结束";

     qDebug() << "针对维护模式的备份操作:开始";
     QString modeFilePath = "";
     modeFilePath = modeFilePath + "/ostree/pkgs/" + "system-ovl.conf";

     bool bExist = Utils::isDirExists(TEMP_OTHERS_PATH);
     if(!bExist)
     {
         if (!Utils::mkpath(TEMP_OTHERS_PATH))
         {
             qCritical() << QString("mkdir %1 failed !").arg(TEMP_OTHERS_PATH) ;
         }
     }

     QStringList cpArgs= m_pOstree->getCpArgs(modeFilePath, TEMP_OTHERS_PATH);
     m_pOstree->setOperation(OstreeInterface::Operation::CP);
     qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::CP);
     m_pOstree->start(cpArgs, true);

     QString modeFlagFilePath = "";
     modeFlagFilePath = modeFlagFilePath + "/ostree/pkgs/" + "mm.lock";

     if (!Utils::fileExists(modeFlagFilePath)) {
         qDebug() << modeFlagFilePath << " not exists";
     } else {
         QStringList cpArgs= m_pOstree->getCpArgs(modeFlagFilePath, TEMP_OTHERS_PATH);
         m_pOstree->setOperation(OstreeInterface::Operation::CP);
         qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::CP);
         m_pOstree->start(cpArgs, true);
     }

     //执行commit操作
     QStringList extraDataCommitArgs;
     extraDataCommitArgs= m_pOstree->getCommitArgs(repoPath, m_backupPoint.m_branchName, TEMP_OTHERS_PATH, description);
     m_pOstree->setOperation(OstreeInterface::Operation::COMMIT);
     qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::COMMIT);
     qDebug() << "diaoyong Thread ID:" << QThread::currentThreadId();
     m_pOstree->setOperationPath("others");
     m_pOstree->start(extraDataCommitArgs, true);

     //执行rm操作
     QStringList extraDataRmArgs= m_pOstree->getRmArgs(TEMP_OTHERS_PATH);
     m_pOstree->setOperation(OstreeInterface::Operation::RM);
     qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::RM);
     m_pOstree->start(extraDataRmArgs, true);

     qDebug() << "针对维护模式的备份操作:结束";
 
     qDebug() << "针对kylin-system-updater.db文件的备份操作:开始";
 
     QString filePath = "/var/cache/kylin-system-updater/kylin-system-updater.db";
     if (cpFile(filePath)) {
         qDebug() << "拷贝完成";
     } else {
         qDebug() << "拷贝失败";
     }
 
     qDebug() << "针对kylin-system-updater.db文件的备份操作:结束";
 
     //执行完毕，设置工作类型为空闲
     m_pOstree->setTaskType(OstreeInterface::TaskType::IDLE);
     qDebug() << "SystemBackupProxy::backupSystem invoke end";
     return true;
 }
 
 /**
  * @brief 备份准备
  * @return true,准备成功；false，准备失败
  */
 bool SystemBackupProxy::doPrepare()
 {
     qDebug() << "SystemBackupProxy::doPrepare invoke begin";
 
     m_bSuccess = false;
     //1.检测ostree仓库是否存在

     //实际的存储位置
     QString repoDir;
     if (m_backupWrapper.m_iPosition == BackupPosition::LOCAL)
         repoDir = m_backupWrapper.m_prefixDestPath + ".backup/";
     else if (m_backupWrapper.m_iPosition == BackupPosition::UDISK)
         repoDir = m_backupWrapper.m_prefixDestPath + "/" + ".backup/";

     m_pOstree = new OstreeInterface(this);
     m_pOstree->setOperation(OstreeInterface::Operation::INIT);
     qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::INIT);
     if(!m_pOstree->checkOstreeRepo(repoDir,OstreeInterface::BackupType::SYSTEM))
     {
         //不存在则创建
         m_pOstree->initOstreeRepo(repoDir,OstreeInterface::BackupType::SYSTEM);
     }
 
     //计算备份之前对应的备份仓库大小，用于后期计算备份点大小
     QString repoPath = m_pOstree->getRepoPath(repoDir,OstreeInterface::BackupType::SYSTEM);
     qDebug() << "repo仓库路径：" << repoPath ;
     this->m_CurrentBackupPointSize = Utils::getDirOrFileSize(repoPath);
     qDebug() << "当前仓库大小：" << this->m_CurrentBackupPointSize ;
 
     // 2.绑定对应的信号与槽
     connect(m_pOstree, &OstreeInterface::systemHash, this, [&](QString systemHash){
         m_backupPoint.m_systemHash = systemHash;
     });
 
     //调用不可变接口，获取当前系统分支哈希
     QStringList systemHashArgs= m_pOstree->getSystemHashArgs();
     m_pOstree->setOperation(OstreeInterface::Operation::STATUS);
     qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::STATUS);
     if (!m_bCancel)
         m_pOstree->start(systemHashArgs, true);
     else
         return false;
 
     // 3、记录/data/.backup/snapshots/backuplist.xml文件
     if (!recordBackupPoint())
     {
         qCritical() << "add or update item to backuplist.xml failed !";
         return false;
     }
 
     qDebug() << "SystemBackupProxy::doPrepare invoke end";
     return true;
 }
 
 /**
  * @brief 记录/backup/snapshots/backuplist.xml文件
  * @return true-成功；false-失败
  */
 bool SystemBackupProxy::recordBackupPoint()
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

     QString xmlPath;
     if(m_backupPoint.m_iPosition == 0 )//本地存储
     {
         xmlPath = Utils::getBackupRootPath() + m_xmlFile;
     }else{
         //U盘路径
         xmlPath = m_backupPoint.m_path + "/.backup" + UDISK_XML;
     }

     //QString xmlPath = Utils::getBackupRootPath() + m_xmlFile;
     xmlPath.replace("//", "/");
     qDebug() << "元数据存储位置：" << xmlPath;
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
 void SystemBackupProxy::removeAutoBackupPoint(const QString& uuid)
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
 QStringList SystemBackupProxy::extraDataPaths()
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
 void SystemBackupProxy::extraDataBackup()
 {
     if (m_backExtraPaths.empty())
         return ;
 
     qDebug() << "SystemBackupProxy::extraDataBackup invoke begin";
 
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
 
     qDebug() << "SystemBackupProxy::extraDataBackup invoke end";
 }
 
 /**
  * @brief 计算额外备份数据所需空间大小
  * @return 以字节为单位的空间大小
  * @note  由于当前额外备份数据量很小，瞬间完成，故暂时先以同步阻塞方式进行
  */
 int SystemBackupProxy::calcSizeForExtraBackup()
 {
     qDebug() << "SystemBackupProxy::calcSizeForExtraBackup";
 
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
 
 bool SystemBackupProxy::cpFile(const QString &filePath)
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
