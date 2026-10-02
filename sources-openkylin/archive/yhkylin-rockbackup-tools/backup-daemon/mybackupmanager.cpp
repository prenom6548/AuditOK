/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
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
#include <QDBusInterface>

/**
 * @brief 构造函数
 */
MyBackupManager::MyBackupManager()
{
    // 注册BackupWrapper类型，之后qdbus接口才能使用
    BackupWrapper::registerMetaType();
    // TUDO:dbus管控优化
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

// TUDO:dbus管控优化
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
    // TUDO:dbus管控优化
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

    Worker* worker = WorkerFactory::createWorker(backupWrapper.m_type, backupWrapper.m_iPosition);
    if (nullptr == worker) {
        emit sendEnvCheckResult(int(BackupResult::NO_FOUND_DEALCLASS));
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

/**
 * @brief 备份
 * @param backupWrapper
 * @return 0，正确启动备份；非0，出现错误
 */
int MyBackupManager::goBackup(const BackupWrapper& backupWrapper)
{
    qDebug("MyBackupManager::goBackup invoke begin");
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
        case int(BackupResult::MKSQUASHFS_START_SUCCESS) :
        case int(BackupResult::BACKUP_START_SUCCESS) :
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
        emit this->backupFinished(result);
        this->finished();
    });
    worker->moveToThread(&workerThread);
    connect(&workerThread, &MyThread::started, worker, &Worker::doWork);
    connect(&workerThread, &MyThread::finished, worker, &Worker::deleteLater);
    connect(&workerThread, &MyThread::cancelWork, worker, &Worker::cancel);

    workerThread.start();

    qDebug("MyBackupManager::goBackup invoke end");
    return int(BackupResult::BACKUP_RESULT_INIT);
}

/**
 * @brief 还原
 * @param backupWrapper
 * @return
 */
int MyBackupManager::goRestore(const BackupWrapper& backupWrapper)
{
    qDebug("MyBackupManager::goRestore invoke begin");
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
        case int(BackupResult::RESTORE_START_SUCCESS) :
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
        emit this->sendRestoreResult(result);
        this->finished();
    });
    worker->moveToThread(&workerThread);
    connect(&workerThread, &MyThread::started, worker, &Worker::doWork);
    connect(&workerThread, &MyThread::finished, worker, &Worker::deleteLater);

    workerThread.start();

    qDebug("MyBackupManager::goRestore invoke end");
    return int(BackupResult::BACKUP_RESULT_INIT);
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

/**
 * @brief 控制面板调用的备份接口，重构暂时先兼容以前的老接口
 * @param backupName 备份名称
 * @param createNote 创建时备注
 * @param incNote  增量备注
 * @param frontUserName 前端用户名
 * @param frontUid 前端用户id
 */
void MyBackupManager::autoBackUpForSystemUpdate_noreturn(const QString& backupName, const QString& createNote, const QString& incNote, const QString& frontUserName, int frontUid)
{
    qDebug("MyBackupManager::autoBackUpForSystemUpdate_noreturn invoke begin");
    if (!isAllowedCaller() || m_isActive || !lock(frontUid)) {
        emit sendStartBackupResult(int(BackupResult::LOCK_PROGRAM_FAIL));
        return ;
    }

    BackupWrapper backupWrapper;
    backupWrapper.m_uuid = AUTO_BACKUP_UUID;
    QString xmlPath = Utils::getSysRootPath() + SYS_UPDATE_BACKUP_XML_PATH;
    xmlPath.replace("//", "/");
    ParseBackupList parseXml(xmlPath);
    ParseBackupList::BackupPoint backupPoint = parseXml.findBackupPointByUuid(backupWrapper.m_uuid);
    if (backupName.isEmpty()) {
        if (backupPoint.m_backupName.isEmpty())
            backupWrapper.m_backupName = QDateTime::currentDateTime().toString("yy-MM-dd hh:mm:ss");
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
    backupWrapper.m_frontUserName = frontUserName;
    backupWrapper.m_frontUid = frontUid;
    backupWrapper.m_note = createNote.isEmpty() ? incNote : createNote;

    Worker* worker = WorkerFactory::createWorker(backupWrapper.m_type, backupWrapper.m_iPosition);
    if (nullptr == worker) {
        emit sendStartBackupResult(int(BackupResult::NO_FOUND_DEALCLASS));
        return ;
    }

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
            if (!Utils::isRunning(BACKUP_CLI_NAME)) {
                m_needUmount = true;
                this->umountBackup();
            }
            break;
        }
    });
    connect(worker, &Worker::progress, this, [&](int rate) {
        emit this->sendRate(int(BackupState::WORKING), rate);
    });
    connect(worker, &Worker::workResult, this, [&] (bool result) {
        emit this->sendBackupResult(result);
        this->finished();
        if (!Utils::isRunning(BACKUP_CLI_NAME)) {
            m_needUmount = true;
            this->umountBackup();
        }
    });
    worker->moveToThread(&workerThread);
    connect(&workerThread, &MyThread::started, worker, &Worker::doWork);
    connect(&workerThread, &MyThread::finished, worker, &Worker::deleteLater);

    workerThread.start();

    qDebug("MyBackupManager::autoBackUpForSystemUpdate_noreturn invoke end");
}

/**
 * @brief 控制面板调用的系统自动备份点还原接口
 * @note 此接口是后来加的，系统更新模块意图在更新失败后直接调用还原接口进行还原
 */
void MyBackupManager::autoRestoreForSystemUpdate(const QString& frontUserName, int frontUid)
{
    qDebug("MyBackupManager::autoRestoreForSystemUpdate invoke begin");
    if (!isAllowedCaller() || m_isActive || !lock(frontUid)) {
        emit sendAutoRestoreResult(false);
        return ;
    }
    Mount_backup_partition();

    BackupWrapper backupWrapper;
    backupWrapper.m_uuid = AUTO_BACKUP_UUID;
    backupWrapper.m_type = BackupType::RESTORE_SYSTEM;
    backupWrapper.m_iPosition = BackupPosition::CUSTOMIZE;
    backupWrapper.m_frontUserName = frontUserName;
    backupWrapper.m_frontUid = frontUid;

    Worker* worker = WorkerFactory::createWorker(backupWrapper.m_type, backupWrapper.m_iPosition);
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
            if (!Utils::isRunning(BACKUP_CLI_NAME)) {
                m_needUmount = true;
                this->umountBackup();
            }
            break;
        }
    });
    connect(worker, &Worker::progress, this, [&](int rate) {
        emit this->sendRate(int(BackupState::WORKING), rate);
    });
    connect(worker, &Worker::workResult, this, [&] (bool result) {
        emit this->sendAutoRestoreResult(result);
        if(result)
        {
            reboot();
        }
        this->finished();
        if (!Utils::isRunning(BACKUP_CLI_NAME)) {
            m_needUmount = true;
            this->umountBackup();
        }
    });
    worker->moveToThread(&workerThread);
    connect(&workerThread, &MyThread::started, worker, &Worker::doWork);
    connect(&workerThread, &MyThread::finished, worker, &Worker::deleteLater);

    workerThread.start();

    qDebug("MyBackupManager::autoRestoreForSystemUpdate invoke end");
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
    QString xmlPath = Utils::getSysRootPath() + SYS_UPDATE_BACKUP_XML_PATH;
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
    if(isAllowedCaller()==false){
        return int(m_backupState);
    }else{
    isActive = m_isActive;
    return int(m_backupState);

    }
}

/**
 * @brief 取消操作
 */
int MyBackupManager::cancel()
{
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
    return ::reboot(RB_AUTOBOOT);;
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
    QString backupPath = Utils::getSysRootPath();
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
        Utils::rmLockFile();
        return false;
    }

    // 用于替换上面的文件锁的，对应需求8688
//    if (!inhibit())
//        return false;
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
    QString why("The backup and restore tool is busy, please wait.");
    // 区分中英文
    QString locale = QLocale::system().name();
    if (locale == "zh_CN") {
        why = QString("备份还原工具正在忙碌，请稍等");
    } else if (locale == "bo_CN") {
        why = QString("གྲབས་ཉར་སོར་ཆུད་ལག་ཆ་བྲེལ་བཞིན་ཡོད་།ཏོག་ཙམ་སྒུག་རོགས་།");
    }

    // QVariantList({what, who, why, mode})
    QVariantList args;
    args << QString("shutdown:sleep");
    args << QString("backup-daemon");
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

