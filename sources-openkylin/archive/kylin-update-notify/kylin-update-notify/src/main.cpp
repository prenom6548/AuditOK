#include "core.h"
#include <QApplication>
#include <QDebug>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <ukui-log4qt.h>
#include<qdebug.h>
#include <QLockFile>

#define PROGRAM_NAME "kylin-background-upgrade"
#define LOG_NAME "/.config/kylin-background-upgrade/kylin-background-upgrade.log"
#define LOG_SILENT_NAME "/.config/kylin-background-upgrade/kylin-background-upgrade-silent.log"
#define LOG_MANUL_NAME "/.config/kylin-background-upgrade/kylin-background-upgrade-manul.log"

static FILE *fp = NULL;
QString toDir;
static bool _silentOrManul = true;

void msgHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    static char logPath[255] = {0};
    int uid = -1;
    static int pid = 0;

    Q_UNUSED(context);

    QDateTime currentTime = QDateTime::currentDateTime();
    QString timeStr = currentTime.toString("yy.MM.dd hh:mm:ss +zzz");
    pid = (int)getpid();
    uid = (int)getuid();
    // 获取用于控制命令行输出的环境变量
    char *ctrlEnv = getenv("BACKGROUND_UPGRADE_DEBUG");
    QString env;

    // 格式化输出字符串，添加消息发生时间、消息等级
    QString outMsg;
    switch (type) {
    case QtDebugMsg:
        outMsg = QString("[%1 D .%2 :%4]: %3").arg(timeStr).arg(pid).arg(msg).arg(uid);
        break;
    case QtInfoMsg:
        outMsg = QString("[%1 D .%2 :%4]: %3").arg(timeStr).arg(pid).arg(msg).arg(uid);
        break;
    case QtWarningMsg:
        outMsg = QString("[%1 D .%2 :%4]: %3").arg(timeStr).arg(pid).arg(msg).arg(uid);
        break;
    case QtCriticalMsg:
        outMsg = QString("[%1 D .%2 :%4]: %3").arg(timeStr).arg(pid).arg(msg).arg(uid);
        break;
    case QtFatalMsg:
        outMsg = QString("[%1 D .%2 :%4]: %3").arg(timeStr).arg(pid).arg(msg).arg(uid);
    }

    if (fp != NULL) {
        // 日志文件存在，则输出到日志文件中
        fprintf(fp, "%s\n", outMsg.toUtf8().data());
        fflush(fp);
    }

    if (ctrlEnv != NULL) {
        // 环境变量为true或者1，则将信息输出到命令行
        env = QString(ctrlEnv).toLower();
        if ((env == "true") || (env == "1")) {
            fprintf(stdout, "%s\n", outMsg.toStdString().c_str());
            fflush(stdout);
        }
    }

    // 遇到致命错误，需要终止程序（这里终止程序是可选的）
    if (type == QtFatalMsg)
        abort();
}

void log_env_init(QString arg){
    static int uid = -1;
    static char logPath[98] = {0};
    bool _is_root = false;
    toDir = QDir::homePath();
    toDir += "/.config/kylin-background-upgrade/";
    QDir dir(toDir);
    if(!dir.exists()) {
        dir.mkdir(toDir);
    }

    // 初始执行时，设置log文件路径
    if (uid == -1) {
        uid = getuid();
    }
    if (uid == 0) {
        // root user
        sprintf(logPath, "/var/log/kylin-background-upgrade/kylin-background-upgrade.log");
        _is_root = true;
    } else {
        // normal user
    //    sprintf(logPath, "/run/user/%d/%s.log", uid, PROGRAM_NAME);
        //QString 转 const char*
        std::string str = QDir::homePath().toStdString();
        const char* ch = str.c_str();
        if (arg == "--check-upgrade" || arg == "--check-immediately") {
            sprintf(logPath, "%s%s", ch, LOG_MANUL_NAME);
        } else {
            sprintf(logPath, "%s%s", ch, LOG_SILENT_NAME);
        }
    }

    if (access(logPath, F_OK|W_OK) == 0) { // log文件存在且可写
        if (fp == NULL)
            fp = fopen(logPath, "a+");
        QFileInfo info(logPath);
        if (info.size() > 1024*1024*20) { //20MB
            fclose(fp); fp = NULL;
            QFile fileclean(logPath);
            fileclean.open(QIODevice::WriteOnly | QIODevice::Text | QFile::Truncate);
            fileclean.close();
            if (fp == NULL)
                fp = fopen(logPath, "a+");
        }
    } else if (access(logPath, F_OK) != 0){ //文件不存在
        if (fp == NULL) {
            if (_is_root) { //root用户日志文件
                QDir* dir = new QDir();
                if(!dir->exists("/var/log/kylin-background-upgrade/")){
                    dir->mkpath("/var/log/kylin-background-upgrade/");
                }
                fp = fopen(logPath, "a+");
                if (fp == NULL)
                    printf("Can't open logfile!\n");
            } else {
                fp = fopen(logPath, "a+");
                if (fp == NULL)
                    printf("Can't open logfile!\n");
            }
        }
    } else {
        // log文件不可写，则需要判断是否被打开过，被打开过就需要关闭
        if (fp != NULL)
            fclose(fp);
        fp = NULL;
    }
}

bool _is_openkylin()
{
    QString OPENKYLIN_DISTTRIBUTOR = "Openkylin";
    QString command = "lsb_release -i -s";
    QProcess process;
    process.start(command);
    process.waitForFinished();

    QString result = process.readAllStandardOutput();
    int ret = result.left(OPENKYLIN_DISTTRIBUTOR.length()).compare(OPENKYLIN_DISTTRIBUTOR.left(OPENKYLIN_DISTTRIBUTOR.length()));
    return (0 == ret)?true:false;
}

int main(int argc, char *argv[])
{
    QString arg=QString::fromLatin1(argv[1]);
    log_env_init(arg);
    qInstallMessageHandler(msgHandler);
    char *dis_env=getenv("DISPLAY");
    QString display_env(dis_env);
    if(display_env != NULL){
        qDebug()<<"The desktop graphics environment is normal, DISPLAY="<<dis_env;
    } else {
        qDebug()<<"The desktop graphics environment is abnormal, DISPLAY="<<dis_env<<", exit!";
        exit(0);
    }

    QLockFile manul_lock(toDir+"manul.lock");
    QLockFile silent_lock(toDir+"silent.lock");
    qDebug()<<"";
    qDebug()<<"---------------------------------------------------------------------------------------------------";
    if (arg == "--check-upgrade" || arg == "--check-immediately") {
        qDebug()<<"Start check upgradable ...";
        if (arg == "--check-immediately") {
            qDebug()<<"Perform a boot update check ...";
        }
        if (!manul_lock.tryLock(0)) {
            qDebug()<<"Another process is checking for updates, exit!";
            exit(0);
        }
    } else {
        qDebug()<<"Start silent upgrade ...";
        if (!silent_lock.tryLock(0)) {
            qDebug()<<"Another process is silently upgrading, exit!";
            exit(0);
        }
        if (_is_openkylin()) {
            qDebug()<<"openkylin, exit!";
            exit(0);
        }
    }

#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
        QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    setenv("QT_QPA_PLATFORMTHEME","ukui",true);
    qunsetenv("SESSION_MANAGER");
    QApplication a(argc, argv);
    core m_core(arg);
//    QObject::connect(m_core,SIGNAL(execSignal()),&a,SLOT(quit()));
    return a.exec();
}
