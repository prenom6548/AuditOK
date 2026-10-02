/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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

#include <ukui-log4qt.h>

#include <QCoreApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QTranslator>
#include <QDebug>

#include "qtsinglecoreapplication.h"
#include "StatusManager/statusmanager.h"
#include "DbusManager/dbusservice.h"

int main(int argc, char *argv[])
{
    QtSingleCoreApplication a("kylin-status-manager", argc, argv);
    if (a.isRunning())
    {
        return EXIT_SUCCESS;
    }

    QTranslator translator;
    translator.load(QString("/usr/share/kylin-status-manager/%1.qm").arg(QLocale::system().name()));
    a.installTranslator(&translator);

    QCommandLineParser parser;
    parser.addHelpOption();  //g加帮助选项 （"-h" 或 "--help"）
    parser.addVersionOption();  // 添加版本选项 ("-v" 或 "--version")

    QCommandLineOption logOption(QStringList() << "l" << "log",
                        QtSingleCoreApplication::translate("main", ("print log to terminal")));

    parser.addOption(logOption);
    parser.process(a);

    initUkuiLog4qt("kylin-status-manager");

    StatusManager statusManagerInstance;
    DBusService dbus {&statusManagerInstance};
    statusManagerInstance.notifyStartupFinished();
    return a.exec();
}
