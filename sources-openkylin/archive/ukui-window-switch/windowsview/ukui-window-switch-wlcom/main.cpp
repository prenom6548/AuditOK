/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <QCoreApplication>
#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>
#include "qtsingleapplication.h"
#include "multitaskviewmanagerbywlcom.h"
#include "log-utils.h"

int main(int argc, char *argv[])
{
#ifndef DISABLE_LOG_FILE
    LogUtils::initLogFile("ukui-window-switch-wlcom");
    qInstallMessageHandler(LogUtils::messageOutput);
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
#endif
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif

    QtSingleApplication a(argc, argv);
    QCommandLineParser parser;
    QCommandLineOption showWorkSpaceOption("show-workspace","show multitaskView");
    parser.addOption(showWorkSpaceOption);
    parser.process(a);
    if (a.isRunning())
    {
        if (parser.isSet(showWorkSpaceOption)) {
            a.sendMessage(QApplication::arguments().length() > 1 ? QApplication::arguments().at(1) : a.applicationFilePath());
        }
        return EXIT_SUCCESS;
    }

    MultitaskViewManagerByWlcom m;
    if (parser.isSet(showWorkSpaceOption)) {
        m.open();
    }
    QObject::connect(&a, &QtSingleApplication::messageReceived, [&m](){
        m.open();
    });
    return QtSingleApplication::exec();
}
