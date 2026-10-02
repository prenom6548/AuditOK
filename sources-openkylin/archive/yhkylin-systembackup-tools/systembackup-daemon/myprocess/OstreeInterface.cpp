/*
 *  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
 *  yhkylin-systembackup-tools is licensed under GPL-2+
 *  See LICENSE file for more details.
 */

#include "OstreeInterface.h"
#include <cmath>
#include <QThread>

template <typename T>
T square(T num) {
    return num * num;
}

OstreeInterface::OstreeInterface(QObject* parent) :
    QObject(parent),
    m_process(new QProcess(this))
{
//    connect(m_process, &QProcess::readyRead, this, [&]() {
//        parseResult();
//    });

//    connect(m_process, &QProcess::readyRead, this, [&]() {
//        QString str = QString(m_process->readAll());
//        qDebug() << str;
//        // if (str.contains("B\/s") && str.contains("%")) {
//        if (str.contains("B/s") && str.contains("%")) {
//            if (str.split("%").at(0).length() < 3)
//                return;
//            int tmpRate = str.split("%").at(0).right(3).toInt();
//            if (m_currentRate == tmpRate)
//                return;
//            m_currentRate = tmpRate;
//            emit progress(m_currentRate);
//        }
//    });

    m_currentRate = 0;

    connect(m_process, &QProcess::readyReadStandardError, this, [&]() {
        QByteArray err = m_process->readAllStandardError();
        qCritical("backup process error: %s", err.data());
    });

    connect(m_process, &QProcess::errorOccurred, this, [&](QProcess::ProcessError error) {
        Q_UNUSED(error)
        m_process->kill();
    });

    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processFinish(int, QProcess::ExitStatus)));

    // 进度定时器初始化
    QThread *timerThread = new QThread();
    m_progressTimer = new QTimer();
    m_progressTimer->setInterval(1000);
    m_progressTimer->moveToThread(timerThread);
    connect(this, SIGNAL(progressTimerStart()), m_progressTimer, SLOT(start()));
    connect(this, &OstreeInterface::progressTimerStop, m_progressTimer, &QTimer::stop);
    timerThread->start();
    connect(m_progressTimer, &QTimer::timeout, this, &OstreeInterface::updateTimerProgress, Qt::DirectConnection);
}

OstreeInterface::~OstreeInterface()
{
    if (!m_process && m_process->state() != QProcess::NotRunning) {
        m_process->kill();
    }
}

bool OstreeInterface::checkOstreeRepo(QString backupPath, BackupType type)
{
    QString repoPath = getRepoPath(backupPath, type);

    //如果存在单引号，去掉单引号先
    QString tempPath = repoPath;
    if( repoPath.startsWith("\'") && repoPath.endsWith("\'") )
    {
        tempPath = repoPath.mid(1,repoPath.length()-2);
    }
    repoPath = tempPath;

    //检测repo是否存在
    bool bExist = Utils::fileExists(repoPath);
    qDebug() << "检测repo是否存在  repoPath:" << repoPath;

    if(bExist)
    {
        return true;
    }
    return false;
}

bool OstreeInterface::initOstreeRepo(QString backupPath, BackupType type)
{
    QString repoPath = getRepoPath(backupPath, type);
    //拼接ostree初始化参数
    QStringList args;
    QString argRepoPath = "--repo=" + repoPath;
    args << argRepoPath;
    args << "init";
    args << "--mode=bare";

    //外部执行ostree指令
    start(args);
    return true;
}

QString OstreeInterface::getRepoPath(QString backupPath, BackupType type)
{
    //确定repo存储路径
    QString repoPath = "";

    if(backupPath.isEmpty())
    {
        repoPath = DEFAULT_BACKUP_PATH;
    }else{
        repoPath = backupPath ;
    }

    switch(type)
    {
    case BackupType::SYSTEM_UPDATE :
        repoPath += "repo_system_update";
        break;
    case BackupType::SYSTEM :
        repoPath += "repo_system";
        break;
    case BackupType::APPLICATION:
        repoPath += "repo_application";
        break;
    case BackupType::USER:
        repoPath += "/repo_user";
        break;
    case BackupType::FACTORY:
        repoPath += "repo_factory";
        break;
    }

    //防止外部介质名称带空格等
    if(!backupPath.isEmpty())
    {
        repoPath = "'" + repoPath + "'";
    }

    return repoPath;
}

QStringList OstreeInterface::getSystemHashArgs()
{
    QStringList args;
    QString otherArgs = " awk -F ' ' '{print $3}' ";
    args << "admin";
    args << "status";
    args << "--skip-signatures";
    args << "|";
    args << "grep '*'";
    args << "|";

    args << otherArgs;
    return args;
}

QStringList OstreeInterface::getPendingHashArgs()
{
    QStringList args;
    QString otherArgs = " awk -F ' ' '{print $2}' ";
    args << "admin";
    args << "status";
    args << "--skip-signatures";
    args << "|";
    args << "grep pending";
    args << "|";

    args << otherArgs;
    return args;
}


QStringList OstreeInterface::getDeployArgs(QString bashHash)
{
    QStringList args;
    args << "admin";
    args << "deploy";
    args << bashHash;
    args << "--os='kylin'";
    args << "--retain";
    return args;
}

QStringList OstreeInterface::getUndeployArgs(QString num)
{
    QStringList args;
    args << "admin";
    args << "undeploy";
    args << num;
    return args;
}

QStringList OstreeInterface::getNeedUndeployHashArgs()
{
    QStringList args;
    QString otherArgs1 = " awk -F 'kylin' '{print $2}' ";
    QString otherArgs2 = " awk -F ' ' '{print $1}' ";
    args << "admin";
    args << "status";
    args << "--skip-signatures";
    args << "|";
    args << "grep kylin";
    args << "|";
    args << "grep -v refspec";
    args << "|";
    args << otherArgs1;
    args << "|";
    args << otherArgs2;
    return args;
}

QStringList OstreeInterface::getHashNumberArgs(QString hash)
{
    QStringList args;
    QString otherArgs = " awk -F ':' '{print $1}' ";
    args << "admin";
    args << "status";
    args << "--skip-signatures";
    args << "|";
    args << "grep kylin";
    args << "|";
    args << "grep -v refspec";
    args << "|";
    args << "grep -n";
    args << hash;
    args << "|";

    args << otherArgs;
    return args;
}

QStringList OstreeInterface::getSystemRollBackArgs(QString number)
{
    QStringList args;;
    args << "admin";
    args << "set-default";
    args << number;
    return args;
}


//加入--skip-spec参数，可跳过sbc文件类型等报错类型文件
QStringList OstreeInterface::getCommitArgs(QString repo, QString branch, QString path, QString description)
{
    //拼接ostree commit参数
    QStringList args;
    QString repoPath = "--repo=" + repo;
    QString dirPath = "--tree=dir=" + path;
    args << repoPath;
    args << "commit";
    args << "-b";
    args << branch;
    args << "--skip-spec";
    args << dirPath;
    args << "-s";
    args << description;
    return args;
}

QStringList OstreeInterface::getCheckoutArgs(QString repo, QString destPath, QString hash)
{
    //拼接ostree checkout参数
    QStringList args;
    QString repoPath = "--repo=" + repo;
    QString path = TEMP_RESTORE_PATH + destPath;
    args << repoPath;
    args << "checkout";
    args << hash;
    args << path;
    return args;
}

QStringList OstreeInterface::getCheckoutFullSyncArgs(QString repo, QString destPath, QString hash)
{
    //拼接ostree checkout参数
    QStringList args;
    QString repoPath = "--repo=" + repo;
    QString path =  destPath;
    args << repoPath;
    args << "checkout";
    args << "--process-passthrough-whiteouts";
    args << "--force-copy";
    args << hash;
    args << path;
    args << "--full-sync";
    return args;
}

QStringList OstreeInterface::getRefsArgs(QString repo, QString branch)
{
    //拼接ostree refs参数
    QStringList args;
    QString repoPath = "--repo=" + repo;
    args << repoPath;
    args << "refs";
    args << "--delete";
    args << branch;
    return args;
}

QStringList OstreeInterface::getPruneArgs(QString repo, QString commit_id)
{
    //拼接ostree prune参数
    QStringList args;
    QString repoPath = "--repo=" + repo;
    QString commitID = "--delete-commit=" + commit_id;
    args << repoPath;
    args << "prune";
    args << commitID;
    return args;
}

/**
 * @brief 根据场景获取rsync命令参数
 * @param scene，场景
 * @return rsync的参数信息
 */
QStringList OstreeInterface::getRsyncRepoArgs(QString src, QString desc)
{
    //拼接 rsync commit参数
    QStringList args;
    src = src + "\/";
    src.replace("//", "/");
    desc = desc + "\/";
    desc.replace("//", "/");

    switch(m_rScense)
    {
    case RsyncScense::RESTORE_SYSTEM:
        args << "-avAHXr";
        args << "--info=progress2";
        args << "--no-inc-recursive";
        args << "--ignore-missing-args";
        args << "--delete";
        break;
    case RsyncScense::RESTORE_HOMEDATA:
        args << "-avAHXr";
        args << "--info=progress2";
        args << "--no-inc-recursive";
        args << "--ignore-missing-args";
        args << "--delete";
        args << "--include=*/.*";
        args << "--include=*/*";
        args << "--exclude=.*";
        args << "--include=*";

        break;
    case RsyncScense::RESTORE_HOMEHIDDENDATA:
        args << "-avAHXr";
        args << "--info=progress2";
        args << "--no-inc-recursive";
        args << "--ignore-missing-args";
        args << "--delete";
        args << "--exclude=.box/";
        args << "--include=.*";
        args << "--include=*/.*";
        args << "--include=*/*";
        args << "--exclude=*";
        break;
    case RsyncScense::RESTORE_SYSTEM_SAMESYSTEM:
        args << "-avAHXr";
        args << "--info=progress2";
        args << "--no-inc-recursive";
        args << "--ignore-missing-args";
        args << "--delete";
        // 用户数据白名单，添加排除路径，防止系统哈希没有变化（目标部署点与本地部署点系统哈希相同）情况下rsync同步时用户数据被还原（被删掉或者被备份点中的旧数据覆盖）
        // (1)排除指纹数据，用户密码、角色、权限、生物识别等信息
        args << "--exclude=lib/biometric-auth";
        args << "--exclude=passwd";
        args << "--exclude=shadow";
        args << "--exclude=group";
        args << "--exclude=gshadow";
        args << "--exclude=sudoers";
        // (2)云图片作为桌面背景的路径属于用户数据
        args << "--exclude=lib/AccountsService";
        // (3)保留域用户数据
        args << "--exclude=sssd";
        args << "--exclude=lib/sss";
        args << "--exclude=share/sssd";
        args << "--exclude=ipa";
        args << "--exclude=krb5.keytab";
        args << "--exclude=krb5.conf";
        args << "--exclude=lib/ipa-client";
        args << "--exclude=nsswitch.conf";
        args << "--exclude=pam.d";
        args << "--exclude=hosts";
        args << "--exclude=hostname";
        args << "--exclude=hedron";
        args << "--exclude=kcm";
        args << "--exclude=hedron/hedronagent";
        args << "--exclude=.kyinfo";
        args << "--exclude=LICENSE";
        args << "--exclude=ssl/certs";
        args << "--exclude=share/ca-certificates";
        args << "--exclude=NetworkManager";
        args << "--exclude=lib/pam";
        args << "--exclude=kim";
        args << "--exclude=lib/kim";
        args << "--exclude=systemd/system/multi-user.target.wants/kimbackend.service";
        args << "--exclude=lib/polkit-1/localauthority/40-domain.d";
        args << "--exclude=lib/libsudo-kim.so";
        // （4）保留激活状态
        args << "--exclude=LICENSE";
        args << "--exclude=.kyinfo";
        args << "--exclude=.kyactivation";
        args << "--exclude=.kyhwid";
        args << "--exclude=.product";
        args << "--exclude=share/plymouth";
        break;
    default:
        return args;
    }
    args << src;
    args << desc;
    return args;
}

/**
 * @brief 执行cp操作
 * @param src
 * @return desc
 */
QStringList OstreeInterface::getCpArgs(QString src, QString desc)
{
    //拼接 cp -r 参数
    QStringList args;
    args << "-aL";
    args << "-P";
    args << "-rf";
    args << src;
    args << desc;
    return args;
}

/**
 * @brief 执行rm操作
 * @param src
 * @return desc
 */
QStringList OstreeInterface::getRmArgs(QString path)
{
    //拼接 rm -rf 参数
    QStringList args;
    args << "-rf";
    args << path;
    return args;
}

/**
 * @brief 执行rm操作
 * @param path ： 需要清理的路径
 * @param type ： 需要清理的特殊文件类型，1代表s,针对socket文件;2 字符文件设备 3.块设备文件
 * @return desc
 */
QStringList OstreeInterface::getFindArgs(QString path,int type)
{
    //拼接 cp -r 参数
    QStringList args;

    args << path;
    args << "-type";

    switch(type)
    {
    case 1:
        args << "s";
        break;
    case 2:
        args << "c";
        break;
    case 3:
        args << "b";
        break;
    default:
        args << "s";
    }

    args << "-exec";
//    QString temp = QString(R"(rm -f {} \;)");
//    args << temp;
    args << "rm";
    args << "-f";
    args << "{}";
    args << "\\;";

    return args;
}

void OstreeInterface::setOperationPath(QString path)
{
    m_operPath = path;
}

void OstreeInterface::setBackUpStep(QString step)
{
    m_backUpStep = step;
}

QString OstreeInterface::getBackUpStep()
{
    return m_backUpStep;
}

void OstreeInterface::setOperation(Operation oper)
{
    m_oper = oper;

    // 计算定时器进度系数
    double max_rate;
    double min_rate;
    double ratio;
    double commit_time = 600.0;

    switch(m_oper)
    {
    case COMMIT:
        ratio = 30.0;
        break;
    case RSYNC:
        ratio = 30.0;
        break;
    case RM:
        ratio = 30.0;
        break;
    case CP:
        ratio = 30.0;
        break;
    case FIND:
        ratio = 3.0;
        break;
    default:
        break;
    }

    if("ovl" == m_backUpStep) {
        max_rate = 70.0;
        min_rate = 0;
    } else if("home" == m_backUpStep) {
        max_rate = 100.0;
        min_rate = 70.0;
    }

    m_progressCoefficient = square((max_rate - min_rate) * ratio / 100.0) / commit_time;
}

OstreeInterface::Operation OstreeInterface::getOperation()
{
    return m_oper;
}

void OstreeInterface::setTaskType(TaskType taskType)
{
    m_taskType = taskType;
}

OstreeInterface::TaskType OstreeInterface::getTaskType()
{
    return m_taskType;
}

void OstreeInterface::setRsyncScense(RsyncScense rsyncScense)
{
    m_rScense = rsyncScense;
}

OstreeInterface::RsyncScense OstreeInterface::getRsyncScense()
{
    return m_rScense;
}

void OstreeInterface::start(QStringList args, bool block)
{

    QString operation = "";
    switch(m_oper)
    {
    case INIT:
    case COMMIT:
    case CHECKOUT:
    case STATUS:
    case STATUS_TO_NUM:
    case STATUS_TO_UNDEPLOY:
    case STATUS_TO_PENDING:
    case DEPLOY:
    case UNDEPLOY:
    case ROLLBACK:
    case NOROLLBACK:
    case PRUNE:
    case REFS:
        operation = "ostree";
        break;
    case RSYNC:
        operation = "rsync";
        break;
    case RM:
        operation = "rm";
        break;
    case CP:
        operation = "cp";
        break;
    case FIND:
        operation = "find";
        break;
    }

    emit startOstreeInterfaceOperation();

    qDebug() << "OstreeInterface::" << operation << " invoke begin------------>";
    qDebug() << "zhixing Thread ID:" << quintptr(QThread::currentThreadId());

    QString cmd(operation);
    for (const QString& item : args) {
       cmd += " ";
       cmd += item;
    }
    qDebug() << "执行命令: " << cmd ;

    //m_process->start(operation, args);
    //防止进程还未执行完毕就返回，通过bash方式强行打包执行
    m_process->start("bash", QStringList() << "-c" << cmd);
    if (!m_process->waitForStarted())
    {
        qCritical("process start failed");
        return ;
    }

    // 只在 commit 的时候处理进度
    if (m_oper == COMMIT) {
        qDebug() << "progress timer start";
        emit progressTimerStart();
    } else {
        qDebug() << "progress timer stop";
        emit progressTimerStop();
        m_currentTimeSinceCommit = 0;
    }

    if (block)
    {
        qDebug() << "执行等待" ;
        m_process->waitForFinished(-1);
        qDebug() << "Exit Code:" << m_process->exitCode();
        qDebug() << "执行等待2" ;
    }
    qDebug() << "OstreeInterface::" << operation << " invoke end------------>";
}

void OstreeInterface::startByShell(QStringList args, bool block)
{
    QString operation = "";
    switch(m_oper)
    {
    case INIT:
    case COMMIT:
    case CHECKOUT:
    case STATUS:
    case STATUS_TO_NUM:
    case ROLLBACK:
    case NOROLLBACK:
        operation = "ostree";
        break;
    case RSYNC:
        operation = "rsync";
        break;
    case RM:
        operation = "rm";
        break;
    case CP:
        operation = "cp";
        break;
    case FIND:
        operation = "find";
        break;
    }

    qDebug() << "OstreeInterface::" << operation << " invoke begin------------>";

    QString cmd(operation);
    for (const QString& item : args) {
       cmd += " ";
       cmd += item;
    }
    qDebug() << cmd ;

    m_process->start("bash", QStringList() << "-c" << cmd);
    if (!m_process->waitForStarted())
    {
        qCritical("process start failed");
        return ;
    }

    if (block)
    {
        m_process->waitForFinished(-1);
    }
    qDebug() << "OstreeInterface::" << operation << " invoke end------------>";
}

/**
 * @brief 解析process的输出结果
 */
void OstreeInterface::parseResult()
{
    // 解析结果，结果内容样例如下：
    /*
    init成功之后没有返回值
    checkout成功之后没有返回值
    commit成功之后返回MD5
    */
    QString out(m_process->readAll());
    QStringList lines = out.split("\n");
    qDebug() << out;

    if(out.isEmpty())
    {
        return ;
    }

    if(m_oper == OstreeInterface::Operation::COMMIT)
    {
        //存储commit哈希
        emit commitHash(m_operPath,lines[0]);
        //emit ovlCommitHash(lines[0]);
        qDebug() << "存储commit哈希" << lines[0];
    }
    else if(m_oper == OstreeInterface::Operation::STATUS)
    {
        //存储system哈希
        emit systemHash(lines[0]);
        qDebug() << "存储system哈希" << lines[0];
    }
    else if(m_oper == OstreeInterface::Operation::STATUS_TO_NUM)
    {
        emit hashNumber(lines[0]);
        qDebug() << "存储哈希number" << lines[0];
    }
    else if (m_oper == OstreeInterface::Operation::STATUS_TO_UNDEPLOY)
    {
        emit ostreeHashs(lines);
        qDebug() << "ostree部署点如下：";
        for(int i = 0 ;i < lines.size(); i++)
        {
            qDebug() << lines[i];
        }
    }
    else if(m_oper == OstreeInterface::Operation::STATUS_TO_PENDING)
    {
        emit pendingHash(lines[0]);
        qDebug() << "新部署的哈希：" << lines[0];
    }
}

/**
 * @brief process结束
 * @param exitCode
 */
void OstreeInterface::processFinish(int exitCode, QProcess::ExitStatus)
{
    qDebug() << "OstreeInterface::processFinish invoke begin";

    qDebug() << "progress timer stop";
    emit progressTimerStop();
    m_currentTimeSinceCommit = 0;

    // Q_UNUSED(exitCode)

    if (exitCode == 9) {
        // 取消备份，进程已被强制杀死
        emit finished(false);
    } else {
        //解析返回结果
        parseResult();

        //更新进度
        calcRate();
        qDebug() << "当前的任务进度为：" << m_currentRate;
        emit progress(m_currentRate);

        //执行结束
        emit finished(true);
    }

    qDebug() << "OstreeInterface::processFinish invoke end";
}

void OstreeInterface::updateTimerProgress()
{
    int tmp_rate_increment = sqrt(m_progressCoefficient * ++m_currentTimeSinceCommit);
    qDebug() << "update timer progress: " << m_currentRate + tmp_rate_increment;
    emit progress(m_currentRate + tmp_rate_increment);
}


void OstreeInterface::calcRate()
{
    int maxRate = 0 ;
    int minRate = 0;
    int ratio = 0;
    OstreeInterface::TaskType taskType = getTaskType();
    OstreeInterface::Operation operatorType = getOperation();
    QString step = getBackUpStep();

    qDebug() << "TaskType: " << int(taskType) ;
    qDebug() << "Operation: " << int(operatorType) ;
    qDebug() << "step: " << step ;

    if(OstreeInterface::TaskType::BACKUP == taskType)
    {
        if("ovl" == step)
        {
            maxRate = 70;
            minRate = 0;
        }
//        else if("etc" == step)
//        {
//            maxRate = 40;
//            minRate = 25;
//        }
//        else if("var" == step)
//        {
//            maxRate = 70;
//            minRate = 40;
//        }
        else if("home" == step)
        {
            maxRate = 100;
            minRate = 70;
        }


        switch(operatorType)
        {
        case COMMIT:
            ratio = 30;
            break;
        case RSYNC:
            ratio = 30;
            break;
        case RM:
            ratio = 30;
            break;
        case CP:
            ratio = 30;
            break;
        case FIND:
            ratio = 3;
            break;
        default:
            break;
        }

        m_currentRate += (maxRate - minRate)*ratio/100;
    }
    else if(OstreeInterface::TaskType::RESTORE == taskType)
    {
        if("ovl" == step)
        {
            maxRate = 60;
            minRate = 0;
        }
//        else if("etc" == step)
//        {
//            maxRate = 30;
//            minRate = 20;
//        }
//        else if("var" == step)
//        {
//            maxRate = 60;
//            minRate = 30;
//        }
        else if("home" == step)
        {
            maxRate = 90;
            minRate = 60;
        }

        if(CHECKOUT == operatorType)
        {
            ratio = 50;
        }
        else if(RSYNC == operatorType)
        {
            ratio = 50;
        }
        else if(RM == operatorType)
        {
            m_currentRate = 100 ;
            return;
        }
        m_currentRate += (maxRate - minRate)*ratio/100;
    }
    else if(OstreeInterface::TaskType::IDLE == taskType)
    {
        m_currentRate = 0;
    }
}


