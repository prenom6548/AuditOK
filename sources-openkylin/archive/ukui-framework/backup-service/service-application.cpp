/*
* Copyright (C) 2025, KylinSoft Co., Ltd.
 *
 *  This program is free software: you can redistribute it and/or modify
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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 */

#include <QCommandLineParser>
#include <QDebug>
#include <QDateTime>
#include "service-application.h"

#include "backup-service-config.h"
#include "backup-service.h"

using namespace UkuiConfigBackup;
ServiceApplication::ServiceApplication(int &argc, char **argv, const QString &applicationName)
    : QtSingleCoreApplication(applicationName, argc, argv)
{
    qDebug() << "Service Application started at" << QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
    setApplicationName("ukui-config-backup-service");
    setApplicationVersion(BACKUP_SERVICE_VERSION);
    if (!this->isRunning()) {
        connect(this, &QtSingleCoreApplication::messageReceived, [=](QString msg) {
            this->parseCmd(msg, true);
        });
        BackupService::self();
    }
    parseCmd(arguments().join(" ").toUtf8(), !isRunning());
}

void ServiceApplication::parseCmd(QString msg, bool isPrimary)
{
    QCommandLineParser parser;

    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption quitOption(QStringList()<<"q"<<"quit", tr("Quit config backup service"));
    parser.addOption(quitOption);

    if (isPrimary) {
        const QStringList args = QString(msg).split(' ');
        parser.process(args);
        if (parser.isSet(quitOption)) {
            qApp->quit();
        }
    } else {
        if (arguments().count() < 2) {
            parser.showHelp();
        }
        parser.process(arguments());
        sendMessage(msg);
    }
}
