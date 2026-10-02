/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#include "../src/saneobject.h"
#include "../src/mainwidget.h"
#include <kysdk/applications/singleapplication.h>
#include "../globalsignal.h"
#include <QLibraryInfo>
#include <QCommandLineParser>
#include <QDir>
#include "../titlebar/titlebar.h"
#include "mytest.h"
#include <QTest>
#define TRANS_PATH  "/usr/share/kylin-scanner/translations"


static QString getCurrentUserName()
{
    QString cmd(BashType);
    QStringList arglists;

    arglists << "-c";
    arglists << "whoami";

    QProcess *process = new QProcess();
    process->start(cmd, arglists);
    process->waitForFinished();
    QString userNow = QString::fromLocal8Bit(process->readAllStandardOutput().trimmed());

    qDebug() << "Exist user: " << userNow;

    return userNow;
}


static QString execCmd(QString cmd)
{
    qDebug() << "cmd = " << cmd;
    QProcess process;
    process.start(QString(cmd));
    process.waitForFinished();
    QByteArray result = process.readAllStandardOutput();
    result = result.left(result.length()-1);
    qDebug() << "arch = " << result;
    return result;
}

static QString getAppVersion()
{
    QString versionResult = execCmd(QString("dpkg-parsechangelog -l %1 --show-field Version").arg(ChangelogFilePath));
    return versionResult;
}

static void createDir(QString dirPath)
{
    qDebug() << "path: "<< dirPath;
    QDir configDir(dirPath);
    if (! configDir.exists()) {
        configDir.mkpath(dirPath);
    }
}


static void createScannerDir()
{
    createDir(g_config_signal->m_configPath);
    createDir(g_config_signal->m_scannerPath);
    createDir(g_config_signal->m_scannerPnmImagePath);
    createDir(g_config_signal->m_scannerImagePath);
    createDir(g_config_signal->m_scannerTempPath);
}

static QString getSystemArchitecture()
{
    QString archResult = execCmd("arch");
    return archResult;
}

static void doWrite(QString userNow, int pidNow)
{
    QFile file(UserExistFile);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
        qDebug() << "open this file error!";
        return;
    }

    // Easy to other user to modify `scanner/user.id` file
    file.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner
                        | QFileDevice::ReadUser | QFileDevice::WriteUser
                        | QFileDevice::ReadGroup | QFileDevice::WriteGroup
                        | QFileDevice::ReadOther | QFileDevice::WriteOther);

    QString message = userNow + "," + QString::number(pidNow);
    qDebug() << "doWrite: message= " << message;

    QTextStream text_stream(&file);
    text_stream << message << "\r\n";
    file.flush();
    file.close();
}

int main(int argc, char *argv[])
{
    QString scannerFileName;
    if (argc > 1) {
        scannerFileName = argv[1];
    }

    int exitCode = 0;

#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif

    kdk::QtSingleApplication app(argc, argv);

    createScannerDir();
    getSystemArchitecture();
    QLocale loc = QLocale::system().name();
    app.setApplicationVersion(getAppVersion());
    app.setWindowIcon(QIcon::fromTheme("kylin-scanner"));
    app.setApplicationName(QApplication::tr("Scanner"));

    if (app.isRunning()) {
        qDebug() << "is running";
        app.sendMessage(scannerFileName);
        return 0;
    }

    // Fetch tokens from the environment, if present
    const QByteArray kylinScannerImageDebug = qgetenv("KYLIN_SCANNER_IMAGE_DEBUG");
    //0606本地扫描调试/*
    if (!kylinScannerImageDebug.isEmpty()) {
//    if (kylinScannerImageDebug.isEmpty()) {
        g_config_signal->m_kylinScannerImageDebug = true;
    }

    QCommandLineParser parser;
    parser.setApplicationDescription("An interface-friendly scanner with OCR, Smart rectify and OneClickBeauty.");
    parser.addHelpOption();
    parser.addVersionOption();

    // An option with a value
    QCommandLineOption openFileOption(QStringList() << "f" << "openfile",
                                      QCoreApplication::translate("main", "Open file <filename>"),
                                      QCoreApplication::translate("main", "Filename"));

    // A boolean option with multiple names (-d, --hide)
    QCommandLineOption hideOption(QStringList() << "d" << "hide",
                                  QCoreApplication::translate("main", "Hide scan settings widget"));
    if (g_config_signal->m_kylinScannerImageDebug) {
        parser.addOption(openFileOption);
        parser.addOption(hideOption);
    }

    parser.process(app);

    if (g_config_signal->m_kylinScannerImageDebug) {
        g_config_signal->m_openFileName = parser.value(openFileOption);
        //0606本地扫描调试
        g_config_signal->m_openFileName = "/home/zyz/ocr.jpg";
        bool hide = parser.isSet(hideOption);
        g_config_signal->m_hideScanSettingsWidget = hide;

        qDebug() << "args = " << parser.positionalArguments()
                 << "hide = " << hide
                 << "openFile = " << g_config_signal->m_openFileName;
    }

    if (! app.isRunning()) {
        QString userNow = getCurrentUserName();
        int pidNow = app.applicationPid();

        doWrite(userNow, pidNow);


        Mytest mytest;
        return app.exec();
    }
}
