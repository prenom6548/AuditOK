/*
 * Copyright 2021 KylinSoft Co., Ltd.
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "powermanagementdaemon.h"
#include "ukui-power-test-tool/uptttestconfigpage.h"

#include <QApplication>
#include <QCommandLineParser>

#include <ukui-log4qt.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setApplicationName(QObject::tr("ukui-powermanagement"));
    QCoreApplication::setApplicationVersion("0.1");

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption runPowerTestToolOption("run-ptt",
                  QCoreApplication::translate("main", "run power test tool"));
    parser.addOption(runPowerTestToolOption);

    parser.process(a);
    UpttTestConfigPage upttTestConfigPage;
    PowerManagementDaemon PowerManagementDaemon;
    QTranslator translator;

    if (true == parser.isSet(runPowerTestToolOption)) {
        upttTestConfigPage.initUpttTestConfigPage();
        upttTestConfigPage.show();
    } else {
        initUkuiLog4qt("ukui-powermanagement");
        if (translator.load(QLocale(), "ukui-power-manager-daemon", "_", QM_FILES_INSTALL_PATH)) {
            a.installTranslator(&translator);
        } else {
            qDebug() << "load ukui-power-manager-tray qm file error";
        }
        PowerManagementDaemon.initPowerManagementDaemon();
    }
    return a.exec();
}
