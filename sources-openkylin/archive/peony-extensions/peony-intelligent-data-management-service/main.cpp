/*
 * Peony Intelligent Spaces Service
 *
 * Copyright (C) 2025, KylinSoft Co., Ltd.
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
 * Authors: He JinQin <hejinqin@kylinos.cn>
 *
 */

#include <QObject>
#include <QCommandLineParser>
#include <QApplication>
#include <QDebug>
#include "model/modelcontroller.h"
#include <QDBusConnection>
#include <QDBusMetaType>
#include <QDBusError>
#include "model/searchresultmodel.h"
#include "3rd-parties/qtsinglecoreapplication.h"
#include <QTranslator>
#include <ukui-log4qt.h>
#ifdef KY_SDK_KABASE
#include <kysdk/applications/kabase/log.hpp>
#endif
int main(int argc, char *argv[])
{

#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif
#ifdef KY_SDK_KABASE
    qInstallMessageHandler(kdk::kabase::Log::logOutput);
#else
    initUkuiLog4qt("peony-intelligent-data-management-service");
#endif
    QCoreApplication::setApplicationName("peony-intelligent-data-management-service");
    QtSingleCoreApplication app("peony-intelligent-data-management-service", argc, argv);
    if (app.isRunning()) {
        QString message = app.arguments().join(' ');
        qDebug() << "send message" << message;
        if (app.sendMessage(message)) {
            return 0;
        } else {
            qWarning() << "failed to send massage to running program";
        }
    }
    QTranslator idm;
    if (idm.load("/usr/share/peony-intelligent-data-management-service/peony-intelligent-data-management-service_"+QLocale::system().name()))
        app.installTranslator(&idm);
    QCommandLineParser parser;
    parser.addHelpOption();
    QCommandLineOption quitOption(QStringList() << "q" << "quit", QObject::tr("exit"));
    parser.addOption(quitOption);
    parser.process(app);
    if (parser.isSet(quitOption)) {
        app.quit();
        return 0;
    }
    auto mc = ModelController::globalInstance();
    mc->setParent(&app);
    QObject::connect(&app, &QtSingleCoreApplication::messageReceived, &app, [&](const QString & message) {
        QStringList arguments = message.split(" ");
        qDebug() << "message recived" << message << "arguments" << arguments;
        parser.process(arguments);
        if (parser.isSet(quitOption)) {
            app.quit();
        }});
    return app.exec();
}
