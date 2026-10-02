#include "mainwindow.h"
#include "mainwindowresult.h"

#include <string.h>
#include <iostream>
#include <QApplication>
#include <QThread>
#include <QDebug>
#include <qlogging.h>
#include <sys/types.h>
#include <stdio.h>
#include <QDateTime>
#include <unistd.h>
#include <QMessageBox>
#include <QTranslator>
#include "./single/qtsingleapplication.h"
#include <ukuistylehelper/ukuistylehelper.h>
#define PROGRAM_NAME "Revison"

static FILE *fp = NULL;

void msgHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
void log_env_init();

int main(int argc, char *argv[])
{
    log_env_init();
    qInstallMessageHandler(msgHandler);
    if(argc > 2)
    {
        qDebug()<<"输入参数过多";
        return -1;
    }
    QtSingleApplication a(argc, argv);
    QTranslator *translator = new QTranslator();
    translator->load("/usr/share/qt5/translations/qt_zh_CN.qm");
    a.installTranslator(translator);
    if (a.isRunning())
    {
        a.sendMessage(QApplication::arguments().length() > 1 ? QApplication::arguments().at(1) : a.applicationFilePath());
        qDebug() << QObject::tr("kylin-upgrade is already running!");
        return EXIT_SUCCESS;
    }
    dbusconnect *dbus = dbusconnect::getInstance();
    dbus->interface->call("clearMoveAppList");
    QDBusReply<QString> mode = dbus->interface->call("getCurOsVersion");

    if(!mode.isValid())
    {
        qDebug()<<"获取版本接口异常";
        QMessageBox msgbox;
        msgbox.setText("获取版本接口异常，请重启后端尝试");
        msgbox.setStandardButtons(QMessageBox::Ok);
        msgbox.setButtonText(QMessageBox::Ok,"OK");
        msgbox.exec();
        return -1;
    }
    if(argc == 1)
    {
        MainWindow Upgrade_main;
//        kdk::UkuiStyleHelper::self()->removeHeader(&Upgrade_main);
        //    Upgrade_main.show();
//        Upgrade_main.showFullScreen();
//        Upgrade_main.activateWindow();
        Upgrade_main.show();
        return a.exec();
    }
    if(argc == 2)
    {
        if(strcmp(argv[1],"result"))
        {
            qDebug()<<"输入参数错误";
            qDebug()<<argv[0];
            qDebug()<<argv[1];
            return -2;
        }
        else
        {
            /*增加退出机制*/
            QDBusInterface upgrade_interface( "com.kylin.RevisionsManager",
                                             "/com/kylin/RevisionsManager",
                                             "com.kylin.RevisionsManager.interface",
                                             QDBusConnection::systemBus() );
            QDBusMessage modelist = upgrade_interface.call("getState");
            QString result = modelist.arguments().value(0).toString();
            qDebug()<<"result is "<<result;
            if(result!="upsuccess"&&result!="upfailed"&&result!="backsuccess"&&result!="backfailed")
            {
                qDebug()<<"return -1";
                return -1;
            }

            MainWindowResult Upgrade_main_result;
//            Upgrade_main_result.dbusinitial(result);
            qDebug()<<"get upgrade result";
            const char* command1="gsettings set org.mate.background picture-filename \"/usr/share/backgrounds/1-warty-final-ubuntukylin.jpg\"";
            const char* command2="gsettings set org.ukui.screensaver background \"/usr/share/backgrounds/1-warty-final-ubuntukylin.jpg\"";
            system(command1);
            system(command2);
//            Upgrade_main_result.showFullScreen();
//            Upgrade_main_result.activateWindow();
            Upgrade_main_result.show();
            return a.exec();
        }
    }
}


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

void log_env_init()
{
    static int uid = -1;
    static char logPath[255] = {0};
    bool _is_root = false;

    // 初始执行时，设置log文件路径
    if (uid == -1) {
        uid = getuid();

    }
    if (uid == 0) {
        // root user
        sprintf(logPath, "/var/log/RevisionsManager/frontend-upgrade.log", uid, PROGRAM_NAME);
        _is_root = true;
    } else {
        // normal user
        sprintf(logPath, "/run/user/%d/%s.log", uid, PROGRAM_NAME);
    }

    if (access(logPath, F_OK|W_OK) == 0)
    { // log文件存在且可写
        if (fp == NULL)
            fp = fopen(logPath, "a+");
        QFileInfo info(logPath);
        if (info.size() > 1024*1024*200)
        { //200MB
            fclose(fp); fp = NULL;
            QFile fileclean(logPath);
            fileclean.open(QIODevice::WriteOnly | QIODevice::Text | QFile::Truncate);
            fileclean.close();
            if (fp == NULL)
                fp = fopen(logPath, "a+");
        }
    }
    else if (access(logPath, F_OK) != 0)
    { //文件不存在
        if (fp == NULL)
        {
            if (_is_root)
            { //root用户日志文件
                QDir* dir = new QDir();
                if(!dir->exists("/var/log/RevisionsManager/"))
                {
                    dir->mkpath("/var/log/RevisionsManager/");
                }
                fp = fopen(logPath, "a+");
                if (fp == NULL)
                    printf("Can't open logfile!\n");
            }
            else
            {
                fp = fopen(logPath, "a+");
                if (fp == NULL)
                printf("Can't open logfile!\n");
            }
        }
    }
    else
    {
        // log文件不可写，则需要判断是否被打开过，被打开过就需要关闭
        if (fp != NULL)
            fclose(fp);
        fp = NULL;
    }
}
