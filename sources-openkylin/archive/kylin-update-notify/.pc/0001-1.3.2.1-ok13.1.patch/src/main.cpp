#include "core.h"
#include <QApplication>
#include <QDebug>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <ukui-log4qt.h>
#include<qdebug.h>

#define PROGRAM_NAME "kylin-background-upgrade"

static FILE *fp = NULL;

void msgHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    static char logPath[255] = {0};
    static int uid = -1;

    Q_UNUSED(context);

    QDateTime currentTime = QDateTime::currentDateTime();
    QString timeStr = currentTime.toString("yy.MM.dd hh:mm:ss +zzz");

    // 获取用于控制命令行输出的环境变量
    char *ctrlEnv = getenv("XXXX_DEBUG");
    QString env;

    // 格式化输出字符串，添加消息发生时间、消息等级
    QString outMsg;
    switch (type) {
    case QtDebugMsg:
        outMsg = QString("[%1 D]: %2").arg(timeStr).arg(msg);
        break;
    case QtInfoMsg:
        outMsg = QString("[%1 I]: %2").arg(timeStr).arg(msg);
        break;
    case QtWarningMsg:
        outMsg = QString("[%1 W]: %2").arg(timeStr).arg(msg);
        break;
    case QtCriticalMsg:
        outMsg = QString("[%1 C]: %2").arg(timeStr).arg(msg);
        break;
    case QtFatalMsg:
        outMsg = QString("[%1 F]: %2").arg(timeStr).arg(msg);
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

void log_env_init(){
    static int uid = -1;
    static char logPath[255] = {0};
    bool _is_root = false;

    // 初始执行时，设置log文件路径
    if (uid == -1) {
        uid = getuid();

    }
    if (uid == 0) {
        // root user
        sprintf(logPath, "/var/log/kylin-background-upgrade/kylin-background-upgrade.log", uid, PROGRAM_NAME);
        _is_root = true;
    } else {
        // normal user
        sprintf(logPath, "/run/user/%d/%s.log", uid, PROGRAM_NAME);
    }
    printf("Logfile path: %s\n", logPath);

    if (access(logPath, F_OK|W_OK) == 0) { // log文件存在且可写
        if (fp == NULL)
            fp = fopen(logPath, "a+");
        QFileInfo info(logPath);
        if (info.size() > 1024*1024*200) { //200MB
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

int main(int argc, char *argv[])
{
    QString arg=QString::fromLatin1(argv[1]);
//    initUkuiLog4qt("kylin-background-upgrade");
    log_env_init();
    qInstallMessageHandler(msgHandler);
    char *x=getenv("DISPLAY");

    qDebug()<<"";
    qDebug()<<"---------------------------------------------------------------------------------------------------";
    qDebug()<<"Start check upgradable ...";

    QString s1(x);
    if(s1 != NULL){
        qDebug()<<"The desktop graphics environment is normal .";
    } else {
        qDebug()<<"The desktop graphics environment is abnormal .";
        putenv("DISPLAY=:0");
    }
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
        QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
//    callPyFunc *lxy = new callPyFunc();

    setenv("QT_QPA_PLATFORMTHEME","ukui",true);
    qunsetenv("SESSION_MANAGER");
    QApplication a(argc, argv);
    core *m_core= new core(arg);
    QObject::connect(m_core,SIGNAL(execSignal()),&a,SLOT(quit()));
    return a.exec();
}
