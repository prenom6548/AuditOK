/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "upgrademain.h"

#include <qlogging.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#ifdef KYDEBUG
#ifndef __sw_64__
#include <ukui-log4qt.h>
#endif
#endif

#define PROGRAM_NAME "kylin-update-frontend"
static FILE *fp = NULL;

void msgHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
void log_env_init();



UpgradeMain::UpgradeMain(QString arg,QWidget *parent)
    : QMainWindow(parent)
{
//    log_env_init();
//    qInstallMessageHandler(msgHandler);
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    if(!CreatConnection())
    {
        qInfo() << "datebase can not open.";
    }

    // 用户手册功能
    mDaemonIpcDbus = new DaemonIpcDbus();

    // 初始化组件
    setWidgetUi();

    //初始化DBus
    QTimer *timer = new QTimer;
    timer->setSingleShot(true);
    connect(timer,&QTimer::timeout,myTabwidget,&TabWid::initDbus);
    timer->start(100);
    qInfo()<<"start to connect dbus";
}

UpgradeMain::~UpgradeMain()
{
//    qInfo() << "quit mainwindow";
}

// 初始化组件
void UpgradeMain::setWidgetUi()
{
    // 整体界面widget
    mainWid = new QWidget(this);

    // 整体界面布局
    mainLayout = new QVBoxLayout();

    // 设置整体界面布局
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);
    myTabwidget = new TabWid(this);
    mainLayout->addWidget(myTabwidget);

    this->mainWid->setLayout(mainLayout);
    // 将mainWid作为整体界面
    this->setCentralWidget(mainWid);
}

// 实现键盘响应
void UpgradeMain::keyPressEvent(QKeyEvent *event)
{
    // F1快捷键打开用户手册
    if (event->key() == Qt::Key_F1) {
        if (!mDaemonIpcDbus->daemonIsNotRunning()){
            // F1快捷键打开用户手册，如kylin-update-manager
            mDaemonIpcDbus->showGuide("kylin-update-manager");
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
        sprintf(logPath, "/var/log/kylin-update-frontend/frontend-upgrade.log", uid, PROGRAM_NAME);
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



