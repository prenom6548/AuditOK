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
#include "mainwidget.h"
#include <kysdk/applications/singleapplication.h>
#include <QApplication>
#include <QByteArray>
#include <QDir>
#include <QLibraryInfo>
#include <QLocale>
#include <QProcess>
#include <QTranslator>
#include <X11/Xlib.h> // ought to put this file last
#include <QCommandLineParser>
#include <QIODevice>
#include <QStandardPaths>
#include <QMessageBox>
#include <QObject>
#include <QDir>
#include <QTimer>
#include <QFileInfo>
#include <fstream>
#include <stdio.h>
#include <log.hpp>
#include "windowmanage.hpp"
#include "include/common.h"
#include "globalsignal.h"
#include "mainwidget.h"
#include "scandialog.h"
#include "global.h"
#include "../test/mytest.h"

#define TRANS_PATH "/usr/share/kylin-scanner/translations"
using namespace kdk::kabase;

static void createDir(QString dirPath)
{
    qDebug() << "path: " << dirPath;
    QDir configDir(dirPath);
    if (!configDir.exists())
    {
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

static QString execCmd(QString cmd)
{
    qDebug() << "cmd = " << cmd;
    QProcess process;
    process.start(QString(cmd));
    process.waitForFinished();
    QByteArray result = process.readAllStandardOutput();
    result = result.left(result.length() - 1);
    qDebug() << "arch = " << result;
    return result;
}

static QString getAppVersion()
{
    QString versionResult = execCmd(QString("dpkg-parsechangelog -l %1 --show-field Version").arg(ChangelogFilePath));
    return versionResult;
}

int main(int argc, char *argv[])
{
    /* 使用sdk管理日志 */
    qInstallMessageHandler(kdk::kabase::Log::logOutput);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif

    kdk::QtSingleApplication app(argc, argv);
    app.setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
    app.setApplicationVersion(getAppVersion());
    app.setWindowIcon(QIcon::fromTheme("kylin-scanner"));
    app.setApplicationName(QApplication::tr("Scanner"));

    if (app.isRunning())
    {
        qDebug() << "Application is already running, sending activation message";
        // 向已运行的实例发送激活消息
        app.sendMessage(QByteArray("activate"));
        return 0;
    }

    // Fetch tokens from the envir onment, if present
    const QByteArray kylinScannerImageDebug = qgetenv("KYLIN_SCANNER_IMAGE_DEBUG");
    // 0606本地扫描调试/*
    if (!kylinScannerImageDebug.isEmpty())
    {
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
    if (g_config_signal->m_kylinScannerImageDebug)
    {
        parser.addOption(openFileOption);
        parser.addOption(hideOption);
    }

    parser.process(app);

    if (!app.isRunning())
    {
        QString trans_path;
        if (QDir(TRANS_PATH).exists())
        {
            trans_path = TRANS_PATH;
        }
        else
        {
            trans_path = qApp->applicationDirPath();
        }
        QTranslator app_trans;
        QTranslator qt_trans;
        QTranslator trans_SDK;
        QString locale = QLocale::system().name();
        QString qt_trans_path;
        qt_trans_path = QLibraryInfo::location(QLibraryInfo::TranslationsPath); // /usr/share/qt5/translations
        QTranslator driver_aquire_widget_trans;
        if (driver_aquire_widget_trans.load(QLocale::system(), "driver-acquirer-widget", "_", "/usr/share/driver-acquirer-widget/translations"))
        {
            app.installTranslator(&driver_aquire_widget_trans);
        }
        if (!app_trans.load("kylin-scanner_" + locale + ".qm", trans_path))
            qDebug() << "Load translation file：" << "kylin-scanner_" + locale + ".qm from" << trans_path << "failed!";
        else
            app.installTranslator(&app_trans);
        if (!qt_trans.load("qt_" + locale + ".qm", qt_trans_path))
            qDebug() << "Load translation file：" << "qt_" + locale + ".qm from" << qt_trans_path << "failed!";
        else
            app.installTranslator(&qt_trans);
        QString translatorFileName = QLatin1String("qt_");
        translatorFileName += QLocale::system().name();
        QTranslator *translator = new QTranslator();
        if (translator->load(translatorFileName, QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
        {
            app.installTranslator(translator);
        }
        else
        {
            qDebug() << "加载中文失败";
        }
        if (!trans_SDK.load(":/translations/gui_" + locale + ".qm"))
        {
            qDebug() << "Load translations file" << QLocale() << "failed!";
        }
        else
        {
            QApplication::installTranslator(&trans_SDK);
        }

        MainWidget *w = new MainWidget;
        w->show();
        QTimer::singleShot(1, [&w, &app, &locale]()
                           {       
            createScannerDir();
            w->initcontent(); });

        /* 最小化拉起 */
        app.setActivationWindow(w);

        /* wayland最小化拉起 */
        WindowManage::getWindowId(&w->windowId);
        qDebug() << "mainWidgetWindowId: " << w->windowId;
        qDebug() << "platformName: " << QGuiApplication::platformName();
        
        // 处理单实例激活消息（支持 Wayland 和 X11）
        QObject::connect(&app, &kdk::QtSingleApplication::messageReceived, [=]()
                         {
            qDebug() << "收到激活消息，唤起kylin-scanner 窗口";
            qDebug() << "platformName: " << QGuiApplication::platformName();
            
            // 确保窗口可见并激活（适用于所有环境）
            if (!w->isVisible())
            {
                w->show();
            }
            w->raise();
            w->activateWindow();
            
            // Wayland 环境下额外使用 WindowManage::activateWindow
            // 如果 windowId 还未获取到，延迟尝试激活
            if (QGuiApplication::platformName().startsWith(QLatin1String("wayland"), Qt::CaseInsensitive))
            {
                if (w->windowId != 0)
                {
                    qDebug() << "Wayland: 使用 windowId 激活窗口: " << w->windowId;
                    WindowManage::activateWindow(w->windowId);
                }
                else
                {
                    qDebug() << "Wayland: windowId 还未获取到，稍后重试";
                    // 延迟100ms后再次尝试激活，确保 windowId 已获取
                    QTimer::singleShot(100, [=]() {
                        if (w->windowId != 0)
                        {
                            qDebug() << "Wayland: 延迟激活窗口，windowId: " << w->windowId;
                            WindowManage::activateWindow(w->windowId);
                        }
                        // 即使 windowId 仍为 0，标准 Qt 方法也应该能工作
                        w->raise();
                        w->activateWindow();
                    });
                }
            }
        });

        /* 移除标题栏 */
        WindowManage::removeHeader(w);

        qputenv("QT_WAYLAND_SHELL_INTEGRATION", "ukui-shell");

        return app.exec();
    }
}
