 /* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "mainwindow.h"

#include <QApplication>
#include <QtSingleApplication>
#include <QTranslator>
#include <QObject>
#include <QDebug>
#include <QSharedPointer>
#include <memory>
#include <QCommandLineParser>
#include <QQuickWidget>
#include <QStandardPaths>
#include <QTime>
#include <KWindowEffects>
#include <signal.h>
#include <stdlib.h>
#include <ukuistylehelper/ukuistylehelper.h>
#include <QSessionManager>
#include <QScreen>
#include <QTimer>
#include <QThread>
#include <QProcessEnvironment>

#ifdef KYDEBUG
#ifndef __sw_64__
#include <ukui-log4qt.h>
#endif
#endif

#include "customstyle.h"
#include "interface/ukcccommon.h"
using namespace ukcc;
#include "utils/xatom-helper.h"
#include "ukccsearchserver.h"

const QString KLong = "Loongson";

#define UKCC_SCHEMA "org.ukui.control-center"
#define FONT_SELECT_FILE "fontSelectFile"

MainWindow *mainWindow = nullptr;

void sigExit(int signalNum)
{
    if (mainWindow != nullptr) {
        mainWindow->close();
    }
}

bool isFontFile(QString path)
{
    QStringList list = path.split(".");
    QString fontTypeTTF = "ttf";
    QString fontTypeOTF = "otf";
    QString fontTypeTTC = "ttc";
    QString fontType = list.at(list.size()-1);

    if (fontType.compare(fontTypeTTF, Qt::CaseInsensitive) == 0) {
        return true;
    } else if (fontType.compare(fontTypeOTF, Qt::CaseInsensitive) == 0) {
        return true;
    } else if (fontType.compare(fontTypeTTC, Qt::CaseInsensitive) == 0) {
        return true;
    }
    return false;
}

int main(int argc, char *argv[])
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    qDebug() << "start time = " << QTime::currentTime() << env.keys();
    QTime startTime = QTime::currentTime();
    signal(SIGINT,sigExit);
#ifdef KYDEBUG
#ifndef __sw_64__
    initUkuiLog4qt("ukui-control-center");
#endif
#endif
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    if (UkccCommon::getCpuInfo().startsWith(KLong, Qt::CaseInsensitive)) {
        QQuickWindow::setSceneGraphBackend(QSGRendererInterface::Software);
    }

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif

    QString id = QString("ukui-control-center" + QLatin1String(getenv("DISPLAY")));
    QtSingleApplication a(id, argc, argv);
    a.setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);

    QString fontFileName;
    if (argc > 1) {
        for (int i = 1 ; i < argc; i++) {
            QString fontFile = argv[i];
            if (!isFontFile(fontFile)) {
                qDebug() << Q_FUNC_INFO << "is not font file" << fontFile;
                continue;
            }
            QFileInfo f(fontFile);
            if (f.isFile()) {
                fontFileName = fontFileName + fontFile;
            }
            if (i < (argc - 1)) {
                fontFileName = fontFileName + "/n/n";
            }
        }
    }
    const QByteArray ukccID(UKCC_SCHEMA);
    QGSettings* ukccSettings = new QGSettings(ukccID, QByteArray());
    if (ukccSettings && ukccSettings->keys().contains(FONT_SELECT_FILE)) {
        ukccSettings->set(FONT_SELECT_FILE, fontFileName);
    }
    qDebug() << Q_FUNC_INFO << "click font file" << fontFileName;
    if (a.isRunning()) {
        // 命令行参数
        QCoreApplication::setApplicationName(QObject::tr("ukui-control-center"));
        QCoreApplication::setApplicationVersion("4.20.0.0");
        QCommandLineParser parser;
        QCommandLineOption moduleOption(QStringList() << "m", "display the specified module page", "module");
        QCommandLineOption moduleOption_p(QStringList() << "p", "load the specified plugin", "plugin");
        QCommandLineOption dbusOption(QStringList() << "d" << "dbus", "startup on dbus");

        parser.addHelpOption();
        parser.addVersionOption();
        parser.addOption(dbusOption);
        parser.addOption(moduleOption);
        parser.addOption(moduleOption_p);
        parser.process(a);

        int agr_length = QApplication::arguments().length();
        QString msg = QApplication::arguments().length() > 1 ? QApplication::arguments().at(agr_length-1) : a.applicationFilePath();
        if (agr_length >3 && QApplication::arguments().at(1) == "-m") {
            msg = QApplication::arguments().at(agr_length-2) + "##" + msg;
        }

        // 使用更安全的消息发送方式，避免快速双击时的闪退问题
        bool messageSent = false;
        int retryCount = 0;
        const int maxRetries = 3;
        while (!messageSent && retryCount < maxRetries) {
            try {
                a.sendMessage(msg);
                messageSent = true;
                qDebug() << "ukui-control-center is already running!";
            } catch (...) {
                retryCount++;
                qDebug() << "Failed to send message to running instance, retry" << retryCount;
                if (retryCount < maxRetries) {
                    QThread::msleep(50); // 等待50ms后重试
                }
            }
        }

        if (!messageSent) {
            qDebug() << "Failed to send message after" << maxRetries << "retries";
        }

        return EXIT_SUCCESS;
    } else {
        //控制面板是否被禁用
        if (UkccCommon::isTablet()) {
            QString m_initPath =  QString("%1/%2/%3").arg(QDir::homePath()).arg(".cache/ukui-menu").arg("ukui-menu.ini");
            QSettings settings(m_initPath, QSettings::IniFormat);
            settings.beginGroup("application");
            if (settings.contains("ukui-control-center")){
                bool isRun = settings.value("application/ukui-control-center").toBool();
                if (!isRun) {
                    qDebug() << QObject::tr("ukui-control-center is disabled！");
                    return EXIT_SUCCESS;
                }
            }
            settings.sync();
            settings.endGroup();
        }
        // 加载国际化文件
        QTranslator translator;
        translator.load("/usr/share/ukui-control-center/shell/res/i18n/" + QLocale::system().name());
        a.installTranslator(&translator);

        // sdk翻译加载
        QTranslator trans;
        if(trans.load(":/translations/gui_"+QLocale::system().name()+".qm"))
        {
            a.installTranslator(&trans);
        }

        // 加载 qt 翻译
        QTranslator qttranslator;
        if(!qttranslator.load(QLocale(), "qt", "_", "/usr/share/qt5/translations/")) {
            qDebug() << "Load qt translation file："<< QString("usr/share/qt5/translations/%1.qm").arg(QLocale::system().name()) << " failed!";
        } else {
            a.installTranslator(&qttranslator);
            qDebug()<<"load qt translation file success";
        }
        // 命令行参数
        QCoreApplication::setApplicationName(QObject::tr("ukui-control-center"));
        QCoreApplication::setApplicationVersion("4.20.0.0");

        QCommandLineParser parser;
        QCommandLineOption moduleOption(QStringList() << "m", "display the specified module page", "module");
        QCommandLineOption moduleOption_p(QStringList() << "p", "load the specified plugin", "plugin");
        QCommandLineOption dbusOption(QStringList() << "d" << "dbus", "startup on dbus");

        parser.addHelpOption();
        parser.addVersionOption();
        parser.addOption(dbusOption);
        parser.addOption(moduleOption);
        parser.addOption(moduleOption_p);
        parser.process(a);

        MainWindow w;
        w.startTime = startTime;
        mainWindow = &w;

        kdk::UkuiStyleHelper::self()->removeHeader(&w);

        QDBusConnection connection = QDBusConnection::sessionBus();
        UkccSearchServer search_obj(mainWindow, parser.isSet(dbusOption));
        if (!connection.registerObject("/", &search_obj, QDBusConnection::ExportAllSlots)) {
            qDebug() << "register ukcc search object error:" << connection.lastError().message();
            return EXIT_FAILURE;
        }
        if (!connection.registerService("org.ukui.ukcc.search")) { // 注册服务
            qDebug() << "register ukcc search service error:" << connection.lastError().message();
            return EXIT_FAILURE;
        }

        const QByteArray id("org.ukui.session");
        if (QGSettings::isSchemaInstalled(id)) {
            QGSettings *sessionGesstings = new QGSettings(id, QByteArray());
            if (sessionGesstings->get("restore-session").toBool()) {
                QGuiApplication::setFallbackSessionManagementEnabled(true);
                QGuiApplication::setQuitOnLastWindowClosed(false);
            }
            QObject::connect(sessionGesstings,&QGSettings::changed, [=](QString key){
                qDebug() << "key =" << key;
                if (key == "restoreSession") {
                    QGuiApplication::setFallbackSessionManagementEnabled(sessionGesstings->get("restore-session").toBool());
                    QGuiApplication::setQuitOnLastWindowClosed(!sessionGesstings->get("restore-session").toBool());
                }
            });
        }

        a.setActivationWindow(&w);

        QObject::connect(&a, SIGNAL(messageReceived(const QString&)), &w, SLOT(sltMessageReceived(const QString&)));
        if (!parser.isSet(dbusOption)) {
            w.initPlugins();
        }
        return a.exec();
    }
}
