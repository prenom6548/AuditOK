/*
* Copyright (C) Copyright 2021 KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
* 02110-1301, USA.
**/

#include "ukuismserver.h"
#include "ukuismserverdebug.h"

#include <QApplication>
#include <QDBusConnection>
#include <QCommandLineParser>
#include <QStandardPaths>
#include <QFile>
#include <QDateTime>
#include <QDir>
#include <QTranslator>

extern UKUISMServer*& getGlobalServer(const QString &wm = "");

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString logPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
        + "/ukui-session/ukuismserver.log";
    //判断log文件是否存在
    if (!QFile::exists(logPath)) {
        QString logDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/ukui-session";
        //不存在时，创建ukui-session文件夹
        QDir dir(logDir);
        if (!dir.exists(logDir)) {
            if (!dir.mkdir(logDir)) {
                return;
            }
        }
        //创建log文件
        QFile file(logPath);
        if (!file.open(QIODevice::WriteOnly)) {
            return;
        }
        file.close();
    }
    if (!QFile::exists(logPath)) {
        return;
    }

    QByteArray localMsg = msg.toLocal8Bit();
    QDateTime  dateTime = QDateTime::currentDateTime();
    QByteArray time = QString("[%1] ").arg(dateTime.toString("MM-dd hh:mm:ss.zzz")).toLocal8Bit();
    QString    logMsg;
    switch (type) {
    case QtDebugMsg:
        logMsg = QString("%1 Debug: %2 (%3:%4, %5)\n")
                     .arg(time.constData())
                     .arg(localMsg.constData())
                     .arg(context.file)
                     .arg(context.line)
                     .arg(context.function);
        break;
    case QtInfoMsg:
        logMsg = QString("%1 Info: %2 (%3:%4, %5)\n")
                     .arg(time.constData())
                     .arg(localMsg.constData())
                     .arg(context.file)
                     .arg(context.line)
                     .arg(context.function);
        break;
    case QtWarningMsg:
        logMsg = QString("%1 Warning: %2 (%3:%4, %5)\n")
                     .arg(time.constData())
                     .arg(localMsg.constData())
                     .arg(context.file)
                     .arg(context.line)
                     .arg(context.function);
        break;
    case QtCriticalMsg:
        logMsg = QString("%1 Critical: %2 (%3:%4, %5)\n")
                     .arg(time.constData())
                     .arg(localMsg.constData())
                     .arg(context.file)
                     .arg(context.line)
                     .arg(context.function);
        break;
    case QtFatalMsg:
        logMsg = QString("%1 Fatal: %2 (%3:%4, %5)\n")
                     .arg(time.constData())
                     .arg(localMsg.constData())
                     .arg(context.file)
                     .arg(context.line)
                     .arg(context.function);
        break;
    }

    //clear file content when it is too large
    QFile file(logPath);
    qint64 fileSize = file.size();
    if (fileSize >= 1024 * 1024 * 10) {
        file.open(QFile::WriteOnly | QFile::Truncate);
        file.flush();
        file.close();
    }

    QFile logFile(logPath);
    logFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&logFile);
    ts << logMsg << endl;
    logFile.flush();
    logFile.close();
}

void IoErrorHandler(IceConn iceConn)
{
    getGlobalServer()->ioError(iceConn);
}

void openDubug()
{
    UKUISMSERVER().setFilterRules(QLatin1Literal("org.ukui.ukuismserver=true"));
    qInstallMessageHandler(myMessageOutput);
    qCDebug(UKUISMSERVER) << "===================================================    UKUISMServer start.    ===================================================";
}

int main(int argc, char **argv)
{
    openDubug();
    QApplication a(argc, argv);
    a.setQuitOnLastWindowClosed(false);

    QCommandLineParser parser;//用于解析命令行参数，当程序以命令行形式执行时，可以解析命令行参数
    parser.setApplicationDescription("ukuismserver");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption restoreOption(QStringList() << QStringLiteral("r") << QStringLiteral("restore"),
                                     "Restores the saved user session if available");
    parser.addOption(restoreOption);

    QCommandLineOption wmOption(QStringList() << QStringLiteral("w") << QStringLiteral("windowmanager"),
                                "Starts <wm> in case no other window manager is \nparticipating in the session. Default is 'kwin'",
                                "wm");
    parser.addOption(wmOption);

    QCommandLineOption nolocalOption(QStringLiteral("nolocal"),
                                     "Also allow remote connections");
    parser.addOption(nolocalOption);

    parser.process(a);//从a获取命令行参数， 也就是argc和argv

    // Load ts files
    const QString locale = QLocale::system().name();
    QTranslator   translator;
    qDebug() << "local: " << locale;
    qDebug() << "path: " << QStringLiteral(UKUI_TRANSLATIONS_DIR) + QStringLiteral("/ukui-session-manager");
    if (translator.load(locale, QStringLiteral(UKUI_TRANSLATIONS_DIR) + QStringLiteral("/ukui-session-manager"))) {
        a.installTranslator(&translator);
    } else {
        qDebug() << "Load translations file failed!";
    }

    QString wm = parser.value(wmOption);//应该是为了获取传入的窗管的值 为空

    IceSetIOErrorHandler(IoErrorHandler);
    getGlobalServer(wm)->restoreWM(QStringLiteral("saved at previous logout"));

    qCDebug(UKUISMSERVER) << "ukuismserver register service";
    QDBusConnection::sessionBus().registerService(QStringLiteral("org.ukui.ukuismserver"));

    return a.exec();
}
