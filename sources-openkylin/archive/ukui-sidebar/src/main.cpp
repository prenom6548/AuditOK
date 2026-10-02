/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 *
 */

#include <QDebug>
#include <QTranslator>
#include <QCommandLineParser>

#include "qtsingleapplication.h"
#include "sidebar-main.h"
#include "log-utils.h"

QString parseArgs(const QStringList& args)
{
    if (args.length() < 2) {
        return {};
    }

    QCommandLineOption state({"S", "state"}, QObject::tr("Show the current state of the sidebar."));
    QCommandLineOption show({"s", "show"}, QObject::tr("There are two options, 'notify' and 'control'."), "option");
    QCommandLineOption quit({"q", "quit"}, QObject::tr("Quit sidebar."));

    QCommandLineParser parser;
    parser.addOption(state);
    parser.addOption(show);
    parser.addOption(quit);

    bool pd = parser.parse(args);
    if (pd) {
        if (parser.isSet(state)) {
            //state
            return QStringLiteral("state");

        } else if (parser.isSet(show)) {
            //show
            return QString("show %1").arg(parser.value(show));

        } else if (parser.isSet(quit)) {
            return QStringLiteral("quit");
        }
    } else {
        qDebug() << "Parser Error:" << parser.errorText();
    }

    QCommandLineOption help = parser.addHelpOption();
    QCommandLineOption version = parser.addVersionOption();
    if (parser.isSet(version)) {
        parser.showVersion();
    } else {
        if (!parser.unknownOptionNames().isEmpty()) {
            qDebug() << "Unknown options:" << parser.unknownOptionNames();
        }
        parser.showHelp();
    }
}

int main(int argc, char *argv[])
{
#ifndef UKUI_SIDEBAR_LOG_FILE_DISABLE
    LogUtils::initLogFile("ukui-sidebar");
    qInstallMessageHandler(LogUtils::messageOutput);
#endif

//    qputenv("QT_SCALE_FACTOR", "1.0");
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
//    QGuiApplication::setAttribute(Qt::AA_DisableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif

    QGuiApplication::setApplicationName(QObject::tr("ukui-sidebar"));
    QGuiApplication::setApplicationVersion(VERSION);

    QString sessionType(qgetenv("XDG_SESSION_TYPE"));
    QString displayEnv = (sessionType == QStringLiteral("wayland")) ? QStringLiteral("WAYLAND_DISPLAY") : QStringLiteral("DISPLAY");
    QString display(qgetenv(displayEnv.toUtf8().data()));
    QString appid = QString("ukui-sidebar-qml-%1").arg(display);
    qDebug() << "ukui-sidebar launch with:" << sessionType << "display:" << display << "appid:" << appid;
    if(sessionType == QStringLiteral("wayland")) {
        qputenv("QT_WAYLAND_DISABLE_FIXED_POSITIONS", "true");
        qputenv("QT_WAYLAND_SHELL_INTEGRATION", "ukui-shell");
    }

    QtSingleApplication app(appid, argc, argv);
    QTranslator translator;
    if (translator.load(QString(TRANSLATION_FILE_DIR) + "/ukui-sidebar_" + QLocale::system().name())) {
        QtSingleApplication::installTranslator(&translator);
    } else {
        qWarning() << "Load translations file" << QLocale::system().name() << "failed!";
    }

    QString message = parseArgs(QtSingleApplication::arguments());
    if (app.isRunning()) {
        //已经有实例在运行
        app.sendMessage(message);
        return 0;
    }

    SidebarMain sidebar;
    sidebar.parseMessage(message);
    QObject::connect(&app, &QtSingleApplication::messageReceived, &sidebar, &SidebarMain::parseMessage);

    return QtSingleApplication::exec();
}
