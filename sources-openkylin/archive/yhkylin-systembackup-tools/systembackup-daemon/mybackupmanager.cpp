/*
 *  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
 *  yhkylin-systembackup-tools is licensed under GPL-2+
 *  See LICENSE file for more details.
 */

/**
  * brief 程序比较小，将接口层和控制层合到一起；主要做一些任务分发，状态监控等一些简单工作；复杂的耗时的逻辑放到任务中moveToThread中去执行
  */

#include <QTimer>
#include <QDateTime>
#include <QDBusArgument>
#include <QDBusMessage>
#include <QDBusPendingReply>
#include <QDBusMessage>
#include <QDBusUnixFileDescriptor>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <mutex>
#include <sys/reboot.h>
#include "mybackupmanager.h"
#include "../common/utils.h"
#include "mymountproxy.h"
#include "workerfactory.h"
#include "parsebackuplist.h"
#include "translations.h"
#include <QDebug>
#include <QRegularExpression>
#include "myprocess/OstreeInterface.h"
#include "myprocess/calcbackupsize.h"

/**
 * @brief 构造函数
 */
MyBackupManager::MyBackupManager()
{
    // 注册BackupWrapper类型，之后qdbus接口才能使用
    BackupWrapper::registerMetaType();

//    m_programWhiteList.push_back("/usr/bin/kylin-unattended-upgrade-shutdown");
//    m_programWhiteList.push_back("/usr/bin/ukui-control-center");
//    m_programWhiteList.push_back("/usr/share/kylin-system-updater/kylin-system-updater");
//    m_programWhiteList.push_back("/usr/bin/kybackup");
//    m_programWhiteList.push_back("/usr/bin/kybackupcli");

}

/**
 * @brief 析构函数
 */
MyBackupManager::~MyBackupManager()
{
    if (workerThread.isRunning()) {
        workerThread.quit();
        workerThread.wait();
    }
}
/**
 * @brief 检测应用程序的环境变量是否有黑名单内的
 * @param pid，进程id
 * @return false-检测未通过，有安全风险；true-检测通过
 */
//bool MyBackupManager::checkEnviron(int pid) {
//    QString filePath = QString("/proc/%1/environ").arg(pid);
//    QFile file(filePath);
//    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
//        qDebug() << "file.open failed" << filePath;
//        return false;
//    }
//    QTextStream in(&file);
//    QString data = in.readAll();
//    file.close();
//    data.replace('\0', '\n');
//    QStringList environVars = data.split('\n');
//    for (const QString &var : environVars){
//        if (!var.isEmpty()) {
//            int pos = var.indexOf('=');
//            if (pos != -1) {
//                QString key = var.left(pos);
//                if (key == "LD_PRELOAD" || key == "LD_LIBRARY_PATH" || key == "LD_AUDIT"){
//                    return false;
//                }
//            }
//        }
//    }
//    return true;
//}

bool MyBackupManager::isAllowedCaller(){
    return true;

    QString clientService = message().service();
    QDBusConnectionInterface *intf = connection().interface();
    unsigned int  pid = intf->servicePid(clientService).value();

    /*check caller environ*/
//    if(!checkEnviron(pid)){
//        sendErrorReply(QDBusError::ErrorType::Failed,QString("[%2] dbus method control,env forbidden").arg((pid)));
//        return false;
//    }

    /*check caller white list*/
    QFileInfo file(QString("/proc/%1/exe").arg(pid));
    QString execPath="";
    if (file.exists()) {
        execPath= file.canonicalFilePath();
        for (const QString& whitePath : m_programWhiteList) {
            if (execPath == whitePath){
               return true;
            }
        }

        // python脚本执行时上面逻辑有漏洞，执行命令如：/usr/bin/python3 /usr/bin/d-feet
        if (execPath.startsWith("/usr/bin/python")) {
            QString cmd = QString("ps -ef | grep /usr/bin/python | awk -v pid=%1 '{ if($2 == pid) print $9}' | tr -d '\n'").arg(pid);
            QString execPyPath;
            Utils::executeCMD(cmd.toStdString().c_str(), execPyPath);
            for (const QString& whitePath : m_programWhiteList) {
                if (execPyPath == whitePath){
                   return true;
                }
            }
        }
    }

    sendErrorReply(QDBusError::ErrorType::Failed,QString("[%2] dbus method control,operation not permitted").arg((execPath)));
    return false;
}

/**
 * @brief 挂载backup分区，兼容提供给自动更新模块的老接口；仅供外部接口调用
 * @return
 */
int MyBackupManager::Mount_backup_partition()
{
    qDebug("MyBackupManager::Mount_backup_partition invoke");

    if (isAllowedCaller() == false){
        return int(MountResult::NO_MOUNTED);
    } else {
        m_needUmount = false;
        MyMountProxy mymount;
        return int(mymount.mountBackupPartition());
    }
}

/**
 * @brief 卸载backup分区；仅供外部接口调用
 * @return
 */
bool MyBackupManager::umountBackupPartition()
{
    qDebug("MyBackupManager::umountBackupPartition invoke");

    if (isAllowedCaller() == false){
        return false;
    } else {
        m_needUmount = true;
        return umountBackup();
    }
}

/**
 * @brief 卸载backup分区
 * @return
 */
bool MyBackupManager::umountBackup()
{
    qDebug("MyBackupManager::umountBackup invoke");

    if (m_needUmount && m_isActive) {
        QTimer::singleShot(5*1000, this, &MyBackupManager::umountBackup);
        return true;
    } else if (m_needUmount) {
        MyMountProxy mymount;
        return mymount.umountBackupPartition();
    }

    return true;
}

/**
 * @brief 试用模式重新挂载分区
 * @return bool
 */
bool MyBackupManager::remountPartitionsForCasperMode()
{
    if(isAllowedCaller()==false){
        return false;

    }else{
    MyMountProxy mymount;
    return mymount.remountPartitionsForCasperMode();
    }
}

/**
 * @brief 环境检测
 * @param backupWrapper，参数
 * @return 0，正确启动；非0，出现错误
 */
int MyBackupManager::checkEnv(const BackupWrapper& backupWrapper)
{
    qDebug("MyBackupManager::checkEnv invoke begin");
    if (!isAllowedCaller() || m_isActive || !lock(backupWrapper.m_frontUid)) {
        emit sendEnvCheckResult(int(BackupResult::LOCK_PROGRAM_FAIL));
        return int(BackupResult::LOCK_PROGRAM_FAIL);
    }

    //1.检测/data/.backup/temp/是否存在
    bool bExist = Utils::isDirExists(DEFAULT_BACKUP_PATH);
    if(!bExist)
    {
        if (!Utils::mkpath(DEFAULT_BACKUP_PATH))
        {
            qCritical() << QString("mkdir %1 failed !").arg(DEFAULT_BACKUP_PATH) ;
        }
    }

    bExist = Utils::isDirExists(TEMP_SNAPSHOTS_PATH);
    if(!bExist)
    {
        if (!Utils::mkpath(TEMP_SNAPSHOTS_PATH))
        {
            qCritical() << QString("mkdir %1 failed !").arg(TEMP_SNAPSHOTS_PATH) ;
        }
    }

    bExist = Utils::isDirExists(TEMP_RESTORE_PATH);
    if(!bExist)
    {
        if (!Utils::mkpath(TEMP_RESTORE_PATH))
        {
            qCritical() << QString("mkdir %1 failed !").arg(TEMP_RESTORE_PATH) ;
        }
    }

    qDebug()<<"type is "<< backupWrapper.m_type;
    qDebug()<<"position is "<< backupWrapper.m_iPosition;
    qDebug()<<"user is "<< backupWrapper.m_User;

    Worker* worker = WorkerFactory::createWorker(backupWrapper.m_type, backupWrapper.m_iPosition, backupWrapper.m_User);
    if (nullptr == worker) {
        emit sendEnvCheckResult(int(BackupResult::NO_FOUND_DEALCLASS));
        //如果出错，执行unlock，不影响后续执行逻辑
        this->finished();
        return int(BackupResult::NO_FOUND_DEALCLASS);
    }

    worker->setParam(backupWrapper);
    connect(worker, &Worker::checkResult, this, [&] (int result) {
        emit this->sendEnvCheckResult(result);
        this->finished();
    });
    worker->moveToThread(&workerThread);
    connect(&workerThread, &MyThread::started, worker, &Worker::checkEnv);
    connect(&workerThread, &MyThread::finished, worker, &Worker::deleteLater);

    workerThread.start();

    qDebug("MyBackupManager::checkEnv invoke end");
    return int(BackupResult::BACKUP_RESULT_INIT);
}


int MyBackupManager::checkEnvForKareUpdate(QString paths)
{
    //1.计算所需要备份的路径的大小
    QString cmd = "" ;
    cmd += "du -scb ";
    //获取需要备份的Kare相关数据路径
    QStringList karePaths = paths.split(";");
    if (karePaths.size() > 1)
    {
        for(QString karePath:karePaths)
        {
            if(Utils::isDirExists(karePath))
            {
                cmd +=  karePath + " ";
                qDebug() << "需要备份的Kare数据：" << karePath;
            }
            else
            {
                qDebug() << "需要备份的Kare数据：" << karePath << "不存在";
                return 1;
            }

        }
    }

    cmd += " | tail -1 | cut -f1";

    QString msg;
    Utils::executeCMD(cmd.toStdString().c_str(), msg);
    qDebug() << "所需要备份的Kare总数据量大小：" << msg << "字节";

    // 备份过程中会有一些临时文件产生，会占用一部分空间，故我们预留500M的空间
    qint64 itotalSize = msg.toULongLong();
    itotalSize += 500 * MB;
    qDebug() << "备份数据大小+500M预留空间约为：" << itotalSize << "字节";


    // 2、计算data目录剩余空间大小
    qint64 leftsize = Utils::getLeftSize("/data");
    qDebug()<<"/data目录分区剩余空间大小： "<< leftsize;

    if (leftsize < itotalSize)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/**
 * @brief 删除备份
 * @param backupWrapper
 * @return
 */
int MyBackupManager::deleteBackupPoint(const BackupWrapper& backupWrapper)
{
    qDebug("MyBackupManager::deleteBackupPoint invoke begin");
    if (!isAllowedCaller() || m_isActive || !lock(backupWrapper.m_frontUid)) {
        emit sendEnvCheckResult(int(BackupResult::LOCK_PROGRAM_FAIL));
        return int(BackupResult::LOCK_PROGRAM_FAIL);
    }

    if(!Utils::isValidPath(backupWrapper.m_prefixDestPath)){
        qDebug() << "字符串包含非法字符:" << backupWrapper.m_prefixDestPath;
        return 0;
    }

    Worker* worker = WorkerFactory::createWorker(backupWrapper.m_type, backupWrapper.m_iPosition);
    if (nullptr == worker) {
        emit sendEnvCheckResult(int(BackupResult::NO_FOUND_DEALCLASS));
        return int(BackupResult::NO_FOUND_DEALCLASS);
    }

    worker->setParam(backupWrapper);
    connect(worker, &Worker::workResult, this, [&] (bool result) {
        emit this->sendDeleteResult(result);
        this->finished();
    });
    worker->moveToThread(&workerThread);
    connect(&workerThread, &MyThread::started, worker, &Worker::doWork);
    connect(&workerThread, &MyThread::finished, worker, &Worker::deleteLater);

    workerThread.start();

    qDebug("MyBackupManager::deleteBackupPoint invoke end");
    return int(BackupResult::BACKUP_RESULT_INIT);
}

/**
 * @brief ghost镜像
 * @param backupWrapper
 * @return
 */
int MyBackupManager::ghostBackup(const BackupWrapper& backupWrapper)
{
    qDebug("MyBackupManager::ghostBackup invoke begin");
    if (!isAllowedCaller() || m_isActive || !lock(backupWrapper.m_frontUid)) {
        emit sendEnvCheckResult(int(BackupResult::LOCK_PROGRAM_FAIL));
        return int(BackupResult::LOCK_PROGRAM_FAIL);
    }

    Worker* worker = WorkerFactory::createWorker(backupWrapper.m_type, backupWrapper.m_iPosition);
    if (nullptr == worker) {
        emit sendEnvCheckResult(int(BackupResult::NO_FOUND_DEALCLASS));
        return int(BackupResult::NO_FOUND_DEALCLASS);
    }

    worker->setParam(backupWrapper);
    connect(worker, &Worker::checkResult, this, [&](int result) {
        emit this->sendEnvCheckResult(result);

        switch (result) {
        case int(BackupResult::CHECK_ENV_SUCCESS) :
        case int(BackupResult::GHOST_START_SUCCESS) :
        case int(BackupResult::MKSQUASHFS_DO_SUCCESS) :
        case int(BackupResult::START_CANCEL) :
            break;
        default:
            this->finished();
            break;
        }
    });
    connect(worker, &Worker::progress, this, [&](int rate) {
        emit this->progress(int(BackupState::WORKING), rate);
    });
    connect(worker, &Worker::workResult, this, [&] (bool result) {
        emit this->sendGhostBackupResult(result);
        this->finished();
    });
    worker->moveToThread(&workerThread);
    connect(&workerThread, &MyThread::started, worker, &Worker::doWork);
    connect(&workerThread, &MyThread::finished, worker, &Worker::deleteLater);
    connect(&workerThread, &MyThread::cancelWork, worker, &Worker::cancel);

    workerThread.start();

    qDebug("MyBackupManager::ghostBackup invoke end");
    return int(BackupResult::BACKUP_RESULT_INIT);
}

#include<QDBusContext>
#include<QDBusMessage>
#include<QDBusConnectionInterface>
#include<QDBusConnection>

QString MyBackupManager::getProcessName(uint pid)
{
    // 读取 /proc/[pid]/comm 获取进程名
    QFile file(QString("/proc/%1/comm").arg(pid));
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString processName = file.readLine().trimmed();
        file.close();
        return processName;
    }
    return "Unknown Process";
}

//QString MyBackupManager::getCallerInfo()
//{
//    QDBusContext *qDBusContext = new QDBusContext();

//    qDebug("用户手动系统备份：开始>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>1");
//    // 获取调用方的 bus name
//    QString callerBusName = qDBusContext->message().service();
//    qDebug("用户手动系统备份：开始>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>2");
//    qDebug() << "Caller Bus Name:" << callerBusName;

//    // 获取 QDBusConnectionInterface
//    QDBusConnectionInterface *interface = QDBusConnection::sessionBus().interface();
//    if (!interface) {
//        return "Failed to get QDBusConnectionInterface.";
//    }

//    // 获取调用方的进程 ID
//    QDBusReply<uint> pidReply = interface->call("GetConnectionUnixProcessID", callerBusName);
//    if (!pidReply.isValid()) {
//        return "Failed to get caller PID.";
//    }
//    uint pid = pidReply.value();
//    qDebug() << "Caller PID:" << pid;

//    // 获取进程名称（使用 /proc 文件系统或 ps 命令）
//    QString processName = getProcessName(pid);
//    qDebug() << "Caller Process Name:" << processName;

//    return QString("Caller Bus Name: %1, PID: %2, Process Name: %3")
//        .arg(callerBusName)
//        .arg(pid)
//        .arg(processName);
//}


void MyBackupManager::autoBackUpForKareUpdate(const QString& backupName, const QString& paths)
{
    qDebug("系统升级kare备份：开始>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
    qDebug("MyBackupManager::autoBackUpForKareUpdate invoke begin");
    //qDebug() << lock(frontUid) ;

    //加锁
    if (!lock(1000)) {
        emit sendStartBackupResult(int(BackupResult::LOCK_PROGRAM_FAIL));
        qDebug("MyBackupManager::autoBackUpForSystemUpdate：有操作正在进行，请稍后再试！");
        return ;
    }

    //1.检测/data/.backup/temp/是否存在
    bool bExist = Utils::isDirExists(DEFAULT_BACKUP_PATH);
    if(!bExist)
    {
        if (!Utils::mkpath(DEFAULT_BACKUP_PATH))
        {
            qCritical() << QString("mkdir %1 failed !").arg(DEFAULT_BACKUP_PATH) ;
            return ;
        }
    }

    bExist = Utils::isDirExists(TEMP_SNAPSHOTS_PATH);
    if(!bExist)
    {
        if (!Utils::mkpath(TEMP_SNAPSHOTS_PATH))
        {
            qCritical() << QString("mkdir %1 failed !").arg(TEMP_SNAPSHOTS_PATH) ;
            return ;
        }
    }

    bExist = Utils::isDirExists(TEMP_RESTORE_PATH);
    if(!bExist)
    {
        if (!Utils::mkpath(TEMP_RESTORE_PATH))
        {
            qCritical() << QString("mkdir %1 failed !").arg(TEMP_RESTORE_PATH) ;
            return ;
        }
    }

    qDebug("----------------1");

    BackupWrapper backupWrapper;
    //backupWrapper.m_uuid = AUTO_BACKUP_UUID;
    backupWrapper.m_uuid = Utils::createUuid();
    qDebug() << "UUID:" << backupWrapper.m_uuid;
    backupWrapper.m_uuid.replace("{", "");
    backupWrapper.m_uuid.replace("}", "");

    backupWrapper.m_backupName = backupName;


    //2.获取需要备份的Kare相关数据路径
    QStringList karePaths = paths.split(";");
    if (karePaths.size() > 1)
    {
        for(QString karePath:karePaths)
        {
            backupWrapper.m_backupPaths << karePath;
            qDebug() << "需要备份的Kare数据：" << karePath;
        }
    }


    //创建工作类
    Worker* worker = qobject_cast<Worker *>(Reflect::createObject("AutoKareBackupProxy"));
    if (nullptr == worker) {
        emit sendStartBackupResult(int(BackupResult::NO_FOUND_DEALCLASS));
        return ;
    }

    qDebug("----------------2");

    worker->setParam(backupWrapper);
    connect(worker, &Worker::checkResult, this, [&](int result) {
        switch (result) {
        case int(BackupResult::CHECK_ENV_SUCCESS) :
        case int(BackupResult::MKSQUASHFS_START_SUCCESS) :
        case int(BackupResult::BACKUP_START_SUCCESS) :
            break;
        default:
            emit this->sendStartBackupResult(result);
            this->finished();
            break;
        }
    });
    connect(worker, &Worker::progress, this, [&](int rate) {
        emit this->sendRate(int(BackupState::WORKING),rate);
    });
    connect(worker, &Worker::workResult, this, [&] (bool result) {
        emit this->sendAutoBackupKareResult(result);
        this->finished();
    });
    qDebug("----------------3");
    worker->moveToThread(&workerThread);
    connect(&workerThread, &MyThread::started, worker, &Worker::doWork);
    connect(&workerThread, &MyThread::finished, worker, &Worker::deleteLater);

    workerThread.start();

    qDebug("MyBackupManager::autoBackUpForKareUpdate invoke end");
}

/**
 * @brief 控制面板调用的备份接口，重构暂时先兼容以前的老接口
 * @param backupName 备份名称
 * @param frontUserName 前端用户名
 * @param frontUid 前端用户id
 */
void MyBackupManager::autoBackUpForSystemUpdate(const QString& backupName, const QString& frontUserName, int frontUid)
{
    qDebug("系统升级系统备份：开始>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
    qDebug("MyBackupManager::autoBackUpForSystemUpdate invoke begin");
    //qDebug() << lock(frontUid) ;

    //加锁
    if (!lock(frontUid)) {
        emit sendStartBackupResult(int(BackupResult::LOCK_PROGRAM_FAIL));
        qDebug("MyBackupManager::autoBackUpForSystemUpdate：有操作正在进行，请稍后再试！");
        return ;
    }

    //1.检测/data/.backup/temp/是否存在
    bool bExist = Utils::isDirExists(DEFAULT_BACKUP_PATH);
    if(!bExist)
    {
        if (!Utils::mkpath(DEFAULT_BACKUP_PATH))
        {
            qCritical() << QString("mkdir %1 failed !").arg(DEFAULT_BACKUP_PATH) ;
            return ;
        }
    }

    bExist = Utils::isDirExists(TEMP_SNAPSHOTS_PATH);
    if(!bExist)
    {
        if (!Utils::mkpath(TEMP_SNAPSHOTS_PATH))
        {
            qCritical() << QString("mkdir %1 failed !").arg(TEMP_SNAPSHOTS_PATH) ;
            return ;
        }
    }

    bExist = Utils::isDirExists(TEMP_RESTORE_PATH);
    if(!bExist)
    {
        if (!Utils::mkpath(TEMP_RESTORE_PATH))
        {
            qCritical() << QString("mkdir %1 failed !").arg(TEMP_RESTORE_PATH) ;
            return ;
        }
    }

    qDebug("----------------1");

    BackupWrapper backupWrapper;
    //backupWrapper.m_uuid = AUTO_BACKUP_UUID;
    backupWrapper.m_uuid = Utils::createUuid();
    qDebug() << "UUID:" << backupWrapper.m_uuid;
    backupWrapper.m_uuid.replace("{", "");
    backupWrapper.m_uuid.replace("}", "");
    QString xmlPath = Utils::getBackupRootPath() + SYS_UPDATE_BACKUP_XML_PATH;
    xmlPath.replace("//", "/");
    ParseBackupList parseXml(xmlPath);
    ParseBackupList::BackupPoint backupPoint = parseXml.findBackupPointByUuid(backupWrapper.m_uuid);
    if (backupName.isEmpty()) {
        if (backupPoint.m_backupName.isEmpty())
            backupWrapper.m_backupName = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        else
            backupWrapper.m_backupName = backupPoint.m_backupName;
    } else {
        backupWrapper.m_backupName = backupName;
    }
    backupWrapper.m_backupPaths << "/";
    backupWrapper.m_backupExcludePaths = Utils::getFromExcludePathsFile();
    backupWrapper.m_backupExcludePaths << "/home";
    backupWrapper.m_backupExcludePaths << "/root";
    backupWrapper.m_backupExcludePaths << "/data";
    backupWrapper.m_type = BackupType::AUTO_BACKUP_SYSTEM;
    backupWrapper.m_iPosition = BackupPosition::CUSTOMIZE;
    backupWrapper.m_User = BackupUser::AUTO;
    backupWrapper.m_frontUserName = frontUserName;
    backupWrapper.m_frontUid = frontUid;
//  backupWrapper.m_note = createNote.isEmpty() ? incNote : createNote;

    Worker* worker = WorkerFactory::createWorker(backupWrapper.m_type, backupWrapper.m_iPosition,backupWrapper.m_User);
    if (nullptr == worker) {
        emit sendStartBackupResult(int(BackupResult::NO_FOUND_DEALCLASS));
        return ;
    }

    qDebug("----------------2");

    worker->setParam(backupWrapper);
    connect(worker, &Worker::checkResult, this, [&](int result) {
        switch (result) {
        case int(BackupResult::CHECK_ENV_SUCCESS) :
        case int(BackupResult::MKSQUASHFS_START_SUCCESS) :
        case int(BackupResult::BACKUP_START_SUCCESS) :
            break;
        default:
            emit this->sendStartBackupResult(result);
            this->finished();
            break;
        }
    });
    connect(worker, &Worker::progress, this, [&](int rate) {
        emit this->sendRate(int(BackupState::WORKING), rate);
    });
    connect(worker, &Worker::workResult, this, [&] (bool result) {
        emit this->sendAutoBackupResult(result);
        this->finished();
    });
    qDebug("----------------3");
    worker->moveToThread(&workerThread);
    connect(&workerThread, &MyThread::started, worker, &Worker::doWork);
    connect(&workerThread, &MyThread::finished, worker, &Worker::deleteLater);

    workerThread.start();

    qDebug("MyBackupManager::autoBackUpForSystemUpdate invoke end");
}

/**
 * @brief 控制面板调用的系统备份接口
 * @param backupName 备份名称
 * @param frontUserName 前端用户名
 * @param frontUid 前端用户id，默认1000
 */
void MyBackupManager::backupSystemByUser(BackupWrapper backupWrapper)
{
    qDebug("用户手动系统备份：开始>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
    qDebug("MyBackupManager::backupSystemByUser invoke begin");

    if (!lock(backupWrapper.m_frontUid)) {
        emit sendStartBackupResult(int(BackupResult::LOCK_PROGRAM_FAIL));
        qDebug("MyBackupManager::backupSystemByUser：有操作正在进行，请稍后再试！");
        return ;
    }

    if(!Utils::isValidPath(backupWrapper.m_prefixDestPath)){
        qDebug() << "字符串包含非法字符:" << backupWrapper.m_prefixDestPath;
        return;
    }

    // 检查存储路径是否存在
    if (backupWrapper.m_iPosition == BackupPosition::LOCAL)
    {
        bool bExist = Utils::isDirExists(DEFAULT_BACKUP_PATH);
        if(!bExist)
        {
            if (!Utils::mkpath(DEFAULT_BACKUP_PATH))
            {
                qCritical() << QString("mkdir %1 failed !").arg(DEFAULT_BACKUP_PATH) ;
                return ;
            }
        }

        // 检查snapshots目录是否存在
        bExist = Utils::isDirExists(TEMP_SNAPSHOTS_PATH);
        if(!bExist)
        {
            if (!Utils::mkpath(TEMP_SNAPSHOTS_PATH))
            {
                qCritical() << QString("mkdir %1 failed !").arg(TEMP_SNAPSHOTS_PATH) ;
                return ;
            }
        }

        // 检查temp目录是否存在
        bExist = Utils::isDirExists(TEMP_RESTORE_PATH);
        if(!bExist)
        {
            if (!Utils::mkpath(TEMP_RESTORE_PATH))
            {
                qCritical() << QString("mkdir %1 failed !").arg(TEMP_RESTORE_PATH) ;
                return ;
            }
        }
    } else if (backupWrapper.m_iPosition == BackupPosition::UDISK) {
        // 检查存储路径是否存在
        QString backupPath = backupWrapper.m_prefixDestPath + "/.backup";
        bool bExist = Utils::isDirExists(backupPath);
        if(!bExist)
        {
            if (!Utils::mkpath(backupPath))
            {
                qCritical() << QString("mkdir %1 failed !").arg(backupPath) ;
                return ;
            }
        }

        // 检查snapshot目录是否存在
        QString snapshotPath = backupWrapper.m_prefixDestPath + "/.backup/snapshots";
        qDebug()<<"snapshotPath "<< snapshotPath;
        bExist = Utils::isDirExists(snapshotPath);
        if(!bExist)
        {
            if (!Utils::mkpath(snapshotPath))
            {
                qCritical() << QString("mkdir %1 failed !").arg(snapshotPath) ;
                return ;
            }
        }

        // 检查temp目录是否存在
        QString tempPath = backupWrapper.m_prefixDestPath + "/.backup/temp";
        qDebug()<<"temp path is "<< tempPath;
        bExist = Utils::isDirExists(tempPath);
        if(!bExist)
        {
            if (!Utils::mkpath(tempPath))
            {
                qCritical() << QString("mkdir %1 failed !").arg(tempPath) ;
                return ;
            }
        }
    }

    qDebug("----------------1");

//    BackupWrapper backupWrapper;
//    //backupWrapper.m_uuid = AUTO_BACKUP_UUID;
//    backupWrapper.m_uuid = Utils::createUuid();
//    qDebug() << "UUID:" << backupWrapper.m_uuid;
//    backupWrapper.m_uuid.replace("{", "");
//    backupWrapper.m_uuid.replace("}", "");
//    QString xmlPath = Utils::getBackupRootPath() + SYS_BACKUP_XML_PATH;
//    xmlPath.replace("//", "/");
//    ParseBackupList parseXml(xmlPath);
//    ParseBackupList::BackupPoint backupPoint = parseXml.findBackupPointByUuid(backupWrapper.m_uuid);
//    if (backupName.isEmpty()) {
//        if (backupPoint.m_backupName.isEmpty())
//            backupWrapper.m_backupName = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
//        else
//            backupWrapper.m_backupName = backupPoint.m_backupName;
//    } else {
//        backupWrapper.m_backupName = backupName;
//    }
//    backupWrapper.m_backupPaths << "/";
//    backupWrapper.m_backupExcludePaths = Utils::getFromExcludePathsFile();
//    backupWrapper.m_backupExcludePaths << "/home";
//    backupWrapper.m_backupExcludePaths << "/root";
//    backupWrapper.m_backupExcludePaths << "/data";
//    backupWrapper.m_type = BackupType::BACKUP_SYSTEM;
//    backupWrapper.m_iPosition = BackupPosition::CUSTOMIZE;
//    backupWrapper.m_User = BackupUser::USER;
//    backupWrapper.m_frontUserName = frontUserName;
//    backupWrapper.m_frontUid = frontUid;


    qDebug()<< "position is "<< backupWrapper.m_iPosition <<"type is "<< backupWrapper.m_type;
    Worker* worker = WorkerFactory::createWorker(backupWrapper.m_type, backupWrapper.m_iPosition,backupWrapper.m_User);
    if (nullptr == worker) {
        emit sendStartBackupResult(int(BackupResult::NO_FOUND_DEALCLASS));
        return ;
    }

    qDebug("----------------2");

    worker->setParam(backupWrapper);
    connect(worker, &Worker::checkResult, this, [&](int result) {
        emit this->sendEnvCheckResult(result);

        switch (result) {
        case int(BackupResult::CHECK_ENV_SUCCESS) :
        case int(BackupResult::MKSQUASHFS_START_SUCCESS) :
        case int(BackupResult::BACKUP_START_SUCCESS) :
        case int(BackupResult::START_CANCEL) :
            break;
        default:
            emit this->sendStartBackupResult(result);
            this->finished();
            break;
        }
    });

    connect(worker, &Worker::progress, this, [&](int rate) {
        qDebug() << "sendRate:" << rate;
        emit this->progress(int(BackupState::WORKING), rate);
    });
    connect(worker, &Worker::workResult, this, [&] (bool result) {
        emit this->backupFinished(result);
        this->finished();
    });
    qDebug("----------------3");
    worker->moveToThread(&workerThread);
    connect(&workerThread, &MyThread::started, worker, &Worker::doWork);
    connect(&workerThread, &MyThread::finished, worker, &Worker::deleteLater);
    connect(&workerThread, &MyThread::cancelWork, worker, &Worker::cancel);
    connect(&workerThread, &MyThread::cancelWork, [&](){
        if (m_isStartOstreeInterfaceOperation) {
            qDebug() << "cancelWork signal received";
            doAfterBothSignals();
        }
    });
    connect(worker, &Worker::startOstreeInterfaceOperation, this, [&]() {
        m_isStartOstreeInterfaceOperation = true;
        qDebug() << "startOstreeInterfaceOperation signal received";
    });

    workerThread.start();

    qDebug("MyBackupManager::backupSystemByUser invoke end");
}


/**
 * @brief 用户数据备份接口
 */
void MyBackupManager::backupDataByUser(BackupWrapper backupWrapper)
{
    qDebug("用户数据备份：开始>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
    qDebug("MyBackupManager::backupDataByUser invoke begin");

    if (!lock(backupWrapper.m_frontUid)) {
        emit sendStartBackupResult(int(BackupResult::LOCK_PROGRAM_FAIL));
        qDebug("MyBackupManager::backupDataByUser：有操作正在进行，请稍后再试！");
        return ;
    }

    //1.检测存储路径是否存在
    bool bExist = Utils::isDirExists(backupWrapper.m_prefixDestPath);
    if(!bExist)
    {
        if (!Utils::mkpath(backupWrapper.m_prefixDestPath))
        {
            qCritical() << QString("mkdir %1 failed !").arg(DEFAULT_BACKUP_PATH) ;
            return ;
        }
    }

    //snapshot 是否存在
    QString snapshotPath = backupWrapper.m_prefixDestPath + "/snapshots";
    qDebug()<<"snapshotPath "<< snapshotPath;
    bExist = Utils::isDirExists(snapshotPath);
    if(!bExist)
    {
        if (!Utils::mkpath(snapshotPath))
        {
            qCritical() << QString("mkdir %1 failed !").arg(snapshotPath) ;
            return ;
        }else{
            qDebug()<<"mkdir "<< snapshotPath << " success";
        }
    }

    //temp路径是否存在

    QString tempPath = backupWrapper.m_prefixDestPath + "/temp";
    qDebug()<<"temp path is "<< tempPath;
    bExist = Utils::isDirExists(tempPath);
    if(!bExist)
    {
        if (!Utils::mkpath(tempPath))
        {
            qCritical() << QString("mkdir %1 failed !").arg(tempPath) ;
            return ;
        }else{
            qDebug()<<"mkdir "<< tempPath << " success";
        }
    }

    Worker* worker = WorkerFactory::createWorker(backupWrapper.m_type, backupWrapper.m_iPosition,backupWrapper.m_User);
    if (nullptr == worker) {
        emit sendStartBackupResult(int(BackupResult::NO_FOUND_DEALCLASS));
        return ;
    }

    qDebug("----------------2");

    worker->setParam(backupWrapper);
    connect(worker, &Worker::checkResult, this, [&](int result) {
        emit this->sendEnvCheckResult(result);

        switch (result) {
        case int(BackupResult::CHECK_ENV_SUCCESS) :
        case int(BackupResult::MKSQUASHFS_START_SUCCESS) :
        case int(BackupResult::BACKUP_START_SUCCESS) :
        case int(BackupResult::START_CANCEL) :
            break;
        default:
            emit this->sendStartBackupResult(result);
            this->finished();
            break;
        }
    });

    connect(worker, &Worker::progress, this, [&](int rate) {
        qDebug() << "sendRate:" << rate;
        emit this->progress(int(BackupState::WORKING), rate);
    });

    connect(worker, &Worker::workResult, this, [&] (bool result) {
        emit this->backupFinished(result);
        this->finished();
    });
    qDebug("----------------3");
    worker->moveToThread(&workerThread);
    connect(&workerThread, &MyThread::started, worker, &Worker::doWork);
    connect(&workerThread, &MyThread::finished, worker, &Worker::deleteLater);
    connect(&workerThread, &MyThread::cancelWork, worker, &Worker::cancel);
    connect(&workerThread, &MyThread::cancelWork, [&](){
        if (m_isStartOstreeInterfaceOperation) {
            doAfterBothSignals();
        }
    });
    connect(worker, &Worker::startOstreeInterfaceOperation, this, [&]() {
        m_isStartOstreeInterfaceOperation = true;
    });

    workerThread.start();

    qDebug("MyBackupManager::backupDataByUser invoke end");
}

QString MyBackupManager::autoBackUpList()
{
    // QString backupList;
    qDebug() << "MyBackupManager::autoBackUpList start;";
    QString backupLast;
    QString xmlPath = Utils::getBackupRootPath() + SYS_UPDATE_BACKUP_XML_PATH;
    xmlPath.replace("//", "/");
    ParseBackupList parse(xmlPath);
    QList<ParseBackupList::BackupPoint> points = parse.getBackupPointList();
    if(points.size() == 0)
    {
        qDebug() << "备份点列表为空！";
        return "";
    }
    ParseBackupList::BackupPoint lastBackupPoint = points.last();
    backupLast += lastBackupPoint.m_backupName + ",";
    backupLast += lastBackupPoint.m_uuid + ",";
    backupLast += lastBackupPoint.m_time + ";";
    // for(auto point:points)
    // {
    //     backupList += point.m_backupName + ",";
    //     backupList += point.m_uuid + ",";
    //     backupList += point.m_time + ";";
    // }
    // qDebug() << backupList ;
    return backupLast;
}

QString MyBackupManager::systemBackUpList()
{
    QString backupList;
    QString xmlPath = Utils::getBackupRootPath() + SYS_BACKUP_XML_PATH;
    xmlPath.replace("//", "/");
    ParseBackupList parse(xmlPath);
    QList<ParseBackupList::BackupPoint> points = parse.getBackupPointList();
    for(auto point:points)
    {
        backupList += point.m_backupName + ",";
        backupList += point.m_uuid + ",";
        backupList += point.m_time + ";";
    }
    qDebug() << backupList ;
    return backupList;
}

bool MyBackupManager::checkKareBackupPoint()
{
    qDebug() << "MyBackupManager::checkKareBackupPoint start;";
    QString backupLast;
    QString xmlPath = Utils::getBackupRootPath() + SYS_KARE_BACKUP_XML_PATH;
    xmlPath.replace("//", "/");
    ParseBackupList parse(xmlPath);
    QList<ParseBackupList::BackupPoint> points = parse.getBackupPointList();
    if(points.size() == 0)
    {
        qDebug() << "备份点列表为空！";
        return false;
    }
    return true;
    qDebug() << "MyBackupManager::checkKareBackupPoint end;";
}

/**
 * @brief 控制面板调用的系统自动备份点还原接口
 * @note 此接口是后来加的，系统更新模块意图在更新失败后直接调用还原接口进行还原
 */
void MyBackupManager::autoRestoreForSystemUpdate(const QString& backupUuid, const QString& frontUserName, int frontUid)
{
    qDebug("系统升级系统还原：开始<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
    qDebug("MyBackupManager::autoRestoreForSystemUpdate invoke begin");

    // liuc 2026-1-6
    // 重置语言环境
    Translations::instance().resetLocale(frontUid);

    m_isActive =false;

    if (!lock(frontUid)) {
        emit sendAutoRestoreResult(false);
        qDebug("MyBackupManager::autoRestoreForSystemUpdate：有操作正在进行，请稍后再试！");
        return ;
    }

    //1.检测/data/.backup/temp/是否存在
    bool bExist = Utils::isDirExists(DEFAULT_BACKUP_PATH);
    if(!bExist)
    {
        if (!Utils::mkpath(DEFAULT_BACKUP_PATH))
        {
            qCritical() << QString("mkdir %1 failed !").arg(DEFAULT_BACKUP_PATH) ;
            return ;
        }
    }

    bExist = Utils::isDirExists(TEMP_RESTORE_PATH);
    if(!bExist)
    {
        if (!Utils::mkpath(TEMP_RESTORE_PATH))
        {
            qCritical() << QString("mkdir %1 failed !").arg(TEMP_RESTORE_PATH) ;
            return ;
        }
    }

    BackupWrapper backupWrapper;
    backupWrapper.m_uuid = backupUuid;
    backupWrapper.m_type = BackupType::AUTO_RESTORE_SYSTEM;
    backupWrapper.m_iPosition = BackupPosition::CUSTOMIZE;
    backupWrapper.m_User = BackupUser::AUTO;
    backupWrapper.m_frontUserName = frontUserName;
    backupWrapper.m_frontUid = frontUid;

    Worker* worker = WorkerFactory::createWorker(backupWrapper.m_type, backupWrapper.m_iPosition,backupWrapper.m_User);
    if (nullptr == worker) {
        emit sendAutoRestoreResult(false);
        return ;
    }

    worker->setParam(backupWrapper);
    connect(worker, &Worker::checkResult, this, [&](int result) {
        switch (result) {
        case int(BackupResult::CHECK_ENV_SUCCESS) :
        case int(BackupResult::RESTORE_START_SUCCESS) :
            break;
        default:
            emit this->sendAutoRestoreResult(false);
            this->finished();
            break;
        }
    });

    plymountMode();
    connect(worker, &Worker::progress, this, [&](int rate) {
        emit this->sendRate(int(BackupState::WORKING), rate);

        if (100 == rate)
        {
            if(m_firstTime)
            {
                this->plymountMessage(Translations::instance().tr("Restore Progress : %1 %").arg(rate));
                m_firstTime = false;
            }
            Utils::wait(1);
            this->plymountMessage(Translations::instance().tr("Saving cached data, please wait"));
        }
        else
        {
            this->plymountMessage(Translations::instance().tr("Restore Progress : %1 %").arg(rate));
        }

    });
    connect(worker, &Worker::workResult, this, [&] (bool result) {
        emit this->sendAutoRestoreResult(result);
        this->finished();
        if(result)
        {
            qDebug() << "系统重启！！！";
            Utils::wait(5);
            this->reboot();
        }

    });
    worker->moveToThread(&workerThread);
    connect(&workerThread, &MyThread::started, worker, &Worker::doWork);
    connect(&workerThread, &MyThread::finished, worker, &Worker::deleteLater);

    workerThread.start();

    qDebug("MyBackupManager::autoRestoreForSystemUpdate invoke end");
}

void MyBackupManager::autoRestoreForKareUpdate()
{
    qDebug("Kare升级系统还原：开始<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
    qDebug("MyBackupManager::autoRestoreForKareUpdate invoke begin");

    // liuc 2026-1-6
    // 重置语言环境
    Translations::instance().resetLocale(1000);

    m_isActive =false;

    if (!lock(1000)) {
        emit sendAutoRestoreResult(false);
        qDebug("MyBackupManager::autoRestoreForSystemUpdate：有操作正在进行，请稍后再试！");
        return ;
    }

    //1.检测/data/.backup/temp/是否存在
    bool bExist = Utils::isDirExists(DEFAULT_BACKUP_PATH);
    if(!bExist)
    {
        if (!Utils::mkpath(DEFAULT_BACKUP_PATH))
        {
            qCritical() << QString("mkdir %1 failed !").arg(DEFAULT_BACKUP_PATH) ;
            return ;
        }
    }

    bExist = Utils::isDirExists(TEMP_RESTORE_PATH);
    if(!bExist)
    {
        if (!Utils::mkpath(TEMP_RESTORE_PATH))
        {
            qCritical() << QString("mkdir %1 failed !").arg(TEMP_RESTORE_PATH) ;
            return ;
        }
    }

    BackupWrapper backupWrapper;

    Worker* worker = qobject_cast<Worker *>(Reflect::createObject("AutoKareRestoreProxy"));
    if (nullptr == worker) {
        emit sendAutoRestoreKareResult(false);
        return ;
    }

    worker->setParam(backupWrapper);
    connect(worker, &Worker::checkResult, this, [&](int result) {
        switch (result) {
        case int(BackupResult::CHECK_ENV_SUCCESS) :
        case int(BackupResult::RESTORE_START_SUCCESS) :
            break;
        default:
            emit this->sendAutoRestoreKareResult(false);
            this->finished();
            break;
        }
    });

    plymountMode();
    connect(worker, &Worker::progress, this, [&](int rate) {
        emit this->sendRate(int(BackupState::WORKING), rate);

        if (100 == rate)
        {
            if(m_firstTime)
            {
                this->plymountMessage(Translations::instance().tr("Restore Progress : %1 %").arg(rate));
                m_firstTime = false;
            }
            Utils::wait(1);
            this->plymountMessage(Translations::instance().tr("Saving cached data, please wait"));
        }
        else
        {
            this->plymountMessage(Translations::instance().tr("Restore Progress : %1 %").arg(rate));
        }

    });
    connect(worker, &Worker::workResult, this, [&] (bool result) {
        emit this->sendAutoRestoreKareResult(result);
        this->finished();
        if(result)
        {
            qDebug() << "系统重启！！！";
            Utils::wait(5);
            this->reboot();
        }

    });
    worker->moveToThread(&workerThread);
    connect(&workerThread, &MyThread::started, worker, &Worker::doWork);
    connect(&workerThread, &MyThread::finished, worker, &Worker::deleteLater);

    workerThread.start();

    qDebug("MyBackupManager::autoRestoreForKareUpdate invoke end");
}

/**
 * @brief 控制面板调用的系统自动备份点还原接口
 * @note 此接口是后来加的，系统更新模块意图在更新失败后直接调用还原接口进行还原
 */
void MyBackupManager::restoreSystemByUser(BackupWrapper backupWrapper)
{
    qDebug("用户手动系统还原：开始<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
    qDebug("MyBackupManager::restoreSystemByUser invoke begin");

    m_isActive =false;

    if (!lock(backupWrapper.m_frontUid)) {
        qDebug("MyBackupManager::restoreSystemByUser：有操作正在进行，请稍后再试！");
        emit sendAutoRestoreResult(false);
        return ;
    }

    if(!Utils::isValidPath(backupWrapper.m_prefixDestPath)){
        qDebug() << "字符串包含非法字符:" << backupWrapper.m_prefixDestPath;
        return;
    }

    //1.检测/data/.backup/temp/是否存在
    bool bExist = Utils::isDirExists(DEFAULT_BACKUP_PATH);
    if(!bExist)
    {
        if (!Utils::mkpath(DEFAULT_BACKUP_PATH))
        {
            qCritical() << QString("mkdir %1 failed !").arg(DEFAULT_BACKUP_PATH) ;
            return ;
        }
    }

    bExist = Utils::isDirExists(TEMP_RESTORE_PATH);
    if(!bExist)
    {
        if (!Utils::mkpath(TEMP_RESTORE_PATH))
        {
            qCritical() << QString("mkdir %1 failed !").arg(TEMP_RESTORE_PATH) ;
            return ;
        }
    }

//    BackupWrapper backupWrapper;
//    backupWrapper.m_uuid = backupUuid;
//    backupWrapper.m_type = BackupType::RESTORE_SYSTEM;
//    backupWrapper.m_iPosition = BackupPosition::CUSTOMIZE;
//    backupWrapper.m_User = BackupUser::USER;
//    backupWrapper.m_frontUserName = frontUserName;
//    backupWrapper.m_frontUid = frontUid;

    qDebug() << "backupWrapper.m_type:"  << backupWrapper.m_type;
    qDebug() << "backupWrapper.m_iPosition:"  << backupWrapper.m_iPosition;
    qDebug() << "backupWrapper.m_User:"  << backupWrapper.m_User;
    Worker* worker = WorkerFactory::createWorker(backupWrapper.m_type, backupWrapper.m_iPosition,backupWrapper.m_User);
    if (nullptr == worker) {
        emit sendAutoRestoreResult(false);
        return ;
    }

    qDebug() << "tttttttttttt4" ;
    worker->setParam(backupWrapper);
    connect(worker, &Worker::checkResult, this, [&](int result) {
        switch (result) {
        case int(BackupResult::CHECK_ENV_SUCCESS) :
        case int(BackupResult::RESTORE_START_SUCCESS) :
            break;
        default:
            emit this->sendAutoRestoreResult(false);
            this->finished();
//            if (!Utils::isRunning(BACKUP_CLI_NAME)) {
//                m_needUmount = true;
//                this->umountBackup();
//            }
            break;
        }
    });

//    connect(worker, &Worker::progress, this, [&](int rate) {
//        qDebug() << "sendRate:" << rate;
//        emit this->progress(int(BackupState::WORKING), rate);
//    });

    //备份还原应用不需要plymount占屏
    //plymountMode();
    connect(worker, &Worker::progress, this, [&](int rate) {
        qDebug() << "sendRate:" << rate;
        emit this->progress(int(BackupState::WORKING), rate);

//        this->plymountMessage(Translations::instance().tr("Restore Progress : %1 %").arg(rate));
//        if (100 == rate) {
//            Utils::wait(1);
//            this->plymountMessage(Translations::instance().tr("Saving cached data, please wait"));
//        }

    });

    connect(worker, &Worker::workResult, this, [&] (bool result) {
        emit this->sendRestoreResult(result);
        this->finished();
        if(result)
        {
            qDebug() << "系统重启！！！";
            Utils::wait(5);
            this->reboot();
        }

    });
    worker->moveToThread(&workerThread);
    connect(&workerThread, &MyThread::started, worker, &Worker::doWork);
    connect(&workerThread, &MyThread::finished, worker, &Worker::deleteLater);

    workerThread.start();

    qDebug("MyBackupManager::restoreSystemByUser invoke end");
}

/**
 * @brief 用户数据还原
 */
void MyBackupManager::restoreDataByUser(BackupWrapper backupWrapper)
{
    qDebug("用户数据还原：开始<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
    qDebug("MyBackupManager::restoreDataByUser invoke begin");

    m_isActive =false;

    if (!lock(backupWrapper.m_frontUid)) {
        qDebug("MyBackupManager::restoreDataByUser：有操作正在进行，请稍后再试！");
        emit sendAutoRestoreResult(false);
        return ;
    }

    //1.检测/data/.backup/temp/是否存在
    bool bExist = Utils::isDirExists(DEFAULT_BACKUP_PATH);
    if(!bExist)
    {
        if (!Utils::mkpath(DEFAULT_BACKUP_PATH))
        {
            qCritical() << QString("mkdir %1 failed !").arg(DEFAULT_BACKUP_PATH) ;
            return ;
        }
    }

    bExist = Utils::isDirExists(TEMP_RESTORE_PATH);
    if(!bExist)
    {
        if (!Utils::mkpath(TEMP_RESTORE_PATH))
        {
            qCritical() << QString("mkdir %1 failed !").arg(TEMP_RESTORE_PATH) ;
            return ;
        }
    }

    qDebug() << "backupWrapper.m_type:"  << backupWrapper.m_type;
    qDebug() << "backupWrapper.m_iPosition:"  << backupWrapper.m_iPosition;
    qDebug() << "backupWrapper.m_User:"  << backupWrapper.m_User;
    Worker* worker = WorkerFactory::createWorker(backupWrapper.m_type, backupWrapper.m_iPosition,backupWrapper.m_User);
    if (nullptr == worker) {
        emit sendAutoRestoreResult(false);
        return ;
    }

    worker->setParam(backupWrapper);
    connect(worker, &Worker::checkResult, this, [&](int result) {
        switch (result) {
        case int(BackupResult::CHECK_ENV_SUCCESS) :
        case int(BackupResult::RESTORE_START_SUCCESS) :
            break;
        default:
            emit this->sendAutoRestoreResult(false);
            this->finished();
//            if (!Utils::isRunning(BACKUP_CLI_NAME)) {
//                m_needUmount = true;
//                this->umountBackup();
//            }
            break;
        }
    });

    connect(worker, &Worker::progress, this, [&](int rate) {
        qDebug() << "sendRate:" << rate;
        emit this->progress(int(BackupState::WORKING), rate);
    });
    connect(worker, &Worker::workResult, this, [&] (bool result) {
        emit this->sendRestoreResult(result);
        this->finished();
        if(result)
        {
            qDebug() << "用户数据还原完成！！！";
        }
    });
    worker->moveToThread(&workerThread);
    connect(&workerThread, &MyThread::started, worker, &Worker::doWork);
    connect(&workerThread, &MyThread::finished, worker, &Worker::deleteLater);

    workerThread.start();

    qDebug("MyBackupManager::restoreDataByUser invoke end");
}

/**
 * @brief 控制面板调用的获取系统备份接口
 * @param state
 * @return
 */
QString MyBackupManager::getBackupCommentForSystemUpdate(QString& state)
{
    if(isAllowedCaller()==false){
        return "";
    }else{
    QString xmlPath = Utils::getBackupRootPath() + SYS_UPDATE_BACKUP_XML_PATH;
    xmlPath.replace("//", "/");
    ParseBackupList parseXml(xmlPath);
    ParseBackupList::BackupPoint backupPoint = parseXml.findBackupPointByUuid(AUTO_BACKUP_UUID);
    state = backupPoint.m_state;
    return backupPoint.m_backupName;
    }
}


/**
 * @brief 获取备份状态
 * @param isActive，是否活动状态
 * @return
 */
int MyBackupManager::getBackupState(bool& isActive)
{
    qDebug("MyBackupManager::getBackupState invoke start");
    qDebug() << "m_backupState:" << int(m_backupState) ;
    if(isAllowedCaller()==false)
    {
        return int(m_backupState);
    }
    else
    {
        isActive = m_isActive;
        return int(m_backupState);
    }
}

/**
 * @brief 取消操作
 */
int MyBackupManager::cancel()
{
    qDebug("MyBackupManager::cancel invoke start");

    if(isAllowedCaller()==false){
       return 0;
    }else{
    if (workerThread.isRunning())
        emit workerThread.cancelWork();
    else
        return 1;
    }
    return 0;
}

/**
 * @brief 重启操作
 * @return
 */
int MyBackupManager::reboot()
{
    return system("reboot");
    //return ::reboot(RB_AUTOBOOT);;
}

/**
 * @brief plymouth出厂还原场景时，翻译不生效。
 * @param locale
 */
void MyBackupManager::setLocale(const QString& locale)
{
    if(isAllowedCaller()==false){
        return;
    }else{
    Translations::instance().setlocale(locale);

    }
}

/**
 * @brief 唤醒后台daemon进程
 */
int MyBackupManager::wakeUpDaemon(const BackupWrapper &backupWrapper)
{
    qDebug("MyBackupManager::wakeUpDaemon invoke start");

    if(isAllowedCaller()==false){
       return 0;
    }else{
        g_userName = backupWrapper.m_frontUserName;
        return 1;
    }
}

/**
 * @brief 任务结束
 */
void MyBackupManager::finished()
{
    if (workerThread.isRunning()) {
        workerThread.quit();
        workerThread.wait();
    }

    unlock();
}

/**
 * @brief 系统备份信息列表是否存在备份节点
 */
bool MyBackupManager::existsSystemBackupNode()
{
    if(isAllowedCaller()==false){
        return false;
    }else{
    Mount_backup_partition();

    QList<ParseBackupList::BackupPoint> list;
    QString backupPath = Utils::getBackupRootPath();
    backupPath += SYS_UPDATE_BACKUP_XML_PATH;
    ParseBackupList parse(backupPath);
    list.append(parse.getBackupPointList());

    for (ParseBackupList::BackupPoint &item : list) {
        if ((item.m_uuid == AUTO_BACKUP_UUID) && (int(BackupType::BACKUP_SYSTEM) == item.m_type || int(BackupType::INC_BACKUP_SYSTEM) == item.m_type) && (BACKUP_PARSE_STATE_SUCCESS_STRTING == item.m_state)) {
            return true;
        }
    }
    }

    return false;
}
/**
 * @brief 锁定应用
 * @param frontUid，锁定应用的用户id
 * @return bool
 */
bool MyBackupManager::lock(int frontUid)
{
    std::lock_guard<spinlock_mutex> lock(m_mutex);

    int lock_file_fd = Utils::lockProgram(frontUid);
    if (lock_file_fd < 0) {
        //Utils::rmLockFile();
        return false;
    }

    // 用于替换上面的文件锁的，对应需求8688
//    if (!inhibit())
//        return false;

    // liuc 2026-1-6
    // 重置语言环境，inhibit 中有翻译
    Translations::instance().resetLocale(frontUid);
    inhibit();

    m_fdLockFile = lock_file_fd;
    m_backupState = BackupState::PREPARING;
    m_isActive = true;


    return true;
}

/**
 * @brief 阻止session关机用dbus新接口Inhibit来替换锁文件的方式
 * Inhibti函数有四个参数，按顺序分别为：
        what:需要阻止的是哪些过程，如关机、重启；可以输入的值有：”shutdown”(会同时阻止关机重启)，”sleep”（会同时阻止睡眠休眠），需要同时阻止关机和睡眠可以输入 ”shutdown:sleep”
        who:是谁要阻止，调用该接口的程序名，如”ukui-sessions”
        why：阻止的原因，简单描述即可，如”package installing”
        mode:阻止的方式；可输入的值有”block”(无限期阻止，等到主动取消阻止才会取消)，”delay”（超时阻止，超时一段时间后不管阻止锁，直接执行）
        建议使用”block”的方式阻止，加锁后记得解锁
        可以使用同服务下ListInhibitors方法获取当前的inhibit锁，测试验证是否成功注册
 * @return
 */
bool MyBackupManager::inhibit()
{
    QDBusMessage message;
    message = QDBusMessage::createMethodCall("org.freedesktop.login1",
                                             "/org/freedesktop/login1",
                                             "org.freedesktop.login1.Manager",
                                             QStringLiteral("Inhibit"));

    QString why = Translations::instance().tr("The backup and restore tool is busy, please wait");

    // QVariantList({what, who, why, mode})
    QVariantList args;
    args << QString("shutdown:sleep");
    args << QString("systembackup-daemon");
    args << why;
    args << QString("block");
    message.setArguments(args);

    QDBusPendingReply<QDBusUnixFileDescriptor> reply = QDBusConnection::systemBus().call(message);

    if (!reply.isValid()) {
        qDebug() << "inhibit faile!";
        return false;
    }
    QDBusUnixFileDescriptor inhibitFileDescriptor = reply.value();
    inhibitFileDescriptor.swap(m_inhibitFileDescriptor);
    qDebug() << "inhibit success!";
    return true;
}

void MyBackupManager::uninhibit()
{
    if (!m_inhibitFileDescriptor.isValid()) {
        return;
    }

    qDebug() << "uninhibit success!";
    m_inhibitFileDescriptor = QDBusUnixFileDescriptor();
}

/**
 * @brief 解锁应用
 * @return bool
 */
bool MyBackupManager::unlock()
{
    std::lock_guard<spinlock_mutex> lock(m_mutex);


    Utils::unLockProgram(m_fdLockFile);
    uninhibit();

    m_fdLockFile = -1;
    m_backupState = BackupState::BACKUP_STATE_INIT;
    m_isActive = false;

    return true;
}


void MyBackupManager::plymountMode()
{

    QString msg;
    Utils::executeCMD("/sbin/plymouthd --mode=boot --attach-to-session", msg);
    qDebug() << msg;

    Utils::executeCMD("/bin/plymouth show-splash --wait", msg);
    qDebug() << msg;

    plymountMessage(Translations::instance().tr("Do not perform other operations during restore to avoid data loss"));

    Utils::executeCMD("systemctl stop lightdm", msg);
    qDebug() << msg;
}

void MyBackupManager::plymountMessage(const QString& message)
{
    QString msg;
    std::string tmp = QString("/bin/plymouth message --text '%1' ").arg(message).toStdString();
    Utils::executeCMD(tmp.data(), msg);
    qDebug() << msg;
}

void MyBackupManager::doAfterBothSignals()
{
    // 重置
    m_isStartOstreeInterfaceOperation = false;

    QStringList commandPatternList;
    commandPatternList << "cp -aL -P -rf /ostree/pkgs/ovl-"
                       << "rsync -avAHXr --info=progress2 --no-inc-recursive --ignore-missing-args --delete --exclude=.box/ --include=.* --include=*/.* --include=*/* --exclude=*"
                       << "find /data/.backup/temp/"
                       << "ostree --repo=/data/.backup/repo_system commit -b branch-"
                       << "ostree --repo=/run/media/kylin"
                       << "rm -rf /data/.backup/temp/"
                       << "ostree commit --repo /data/.backup/repo_user --branch branch-"
                       << "ostree admin status"
                       << "ostree --repo=/data/.backup/repo_user/repo_user init";

    foreach (const QString &commandPattern, commandPatternList) {
        qDebug() << "commandPattern: " << commandPattern;

        // 执行ps -ef命令获取进程列表
        QProcess psProcess;
        psProcess.start("ps", QStringList() << "-ef");
        psProcess.waitForFinished();

        // 读取ps命令输出
        QString psOutput = psProcess.readAllStandardOutput();
        QStringList lines = psOutput.split('\n');

        bool processFound = false;

        // 遍历所有进程
        foreach (const QString &line, lines) {
            if (line.contains(commandPattern) && !line.contains("grep")) {
                qDebug() << "line: " << line;
                // 提取PID
                QStringList parts = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
                if (parts.size() > 1) {
                    QString pid = parts[1];
                    qDebug() << "找到进程(PID:" << pid << ")，正在杀死...";

                    // 杀死进程
                    QProcess killProcess;
                    killProcess.start("kill", QStringList() << "-9" << pid);
                    killProcess.waitForFinished();

                    if (killProcess.exitCode() == 0) {
                        qDebug() << "进程" << pid << "已成功终止";
                    } else {
                        qDebug() << "无法终止进程" << pid;
                    }

                    processFound = true;
                }
            }
        }

        if (!processFound) {
            qDebug() << "未找到执行" << commandPattern << "* 的进程";
        }
    }
}

