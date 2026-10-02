/*
 *
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
#include "ukui-search-app-data-service.h"
#include "app-db-manager.h"
#include "signal-transformer.h"
#include "app-info-dbus-argument.h"

#include <QDBusConnection>
#include <QDBusMetaType>
#include <QDBusError>
#include <QDebug>

using namespace UkuiSearch;

UkuiSearchAppDataService::UkuiSearchAppDataService(int &argc, char *argv[], const QString &applicationName):
    QtSingleCoreApplication (applicationName, argc, argv)
{
    qDebug() << "ukui search app data service constructor start";
    setApplicationVersion(QString("v%1").arg(VERSION));

    if (!this->isRunning()) {
        qDebug() << "First running, I'm in app-db manager dbus rigister.";

        qRegisterMetaType<ApplicationPropertyMap>("ApplicationPropertyMap");
        qDBusRegisterMetaType<ApplicationPropertyMap>();

        qRegisterMetaType<ApplicationInfoMap>("ApplicationInfoMap");
        qDBusRegisterMetaType<ApplicationInfoMap>();

        AppDBManager::getInstance();

        QDBusConnection sessionBus = QDBusConnection::sessionBus();
        if(!sessionBus.registerObject("/org/ukui/search/appDataBase/dbManager", AppDBManager::getInstance(), QDBusConnection::ExportAdaptors)) {
            qCritical() << "ukui-search-app-db-manager dbus register object failed reason:" << sessionBus.lastError();
        }

        if(!sessionBus.registerObject("/org/ukui/search/appDataBase/signalTransformer", SignalTransformer::getTransformer(), QDBusConnection::ExportAllSignals)) {
            qCritical() << "ukui-search-signal-transformer dbus register object failed reason:" << sessionBus.lastError();
        }

        if (!sessionBus.registerService("com.ukui.search.appdb.service")) {
            qCritical() << "QDbus register service failed reason:" << sessionBus.lastError();
        }

        connect(this, &QtSingleCoreApplication::messageReceived, [ & ](QString msg) {
            this->parseCmd(msg, true);
        });
    }

    //parse cmd
    qDebug() << "parse cmd";
    auto message = this->arguments().join(' ').toUtf8();
    parseCmd(message, !isRunning());
    qDebug() << "ukui search app data service constructor end";
}

void UkuiSearchAppDataService::parseCmd(QString msg, bool isPrimary)
{
    QCommandLineParser parser;

    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption quitOption(QStringList()<<"q"<<"quit", tr("Stop service"));
    parser.addOption(quitOption);

    if (isPrimary) {
        const QStringList args = QString(msg).split(' ');
        parser.process(args);

        if (parser.isSet(quitOption)) {
            qApp->quit();
            return;
        }
    }
    else {
        if (arguments().count() < 2) {
            parser.showHelp();
        }
        parser.process(arguments());
        sendMessage(msg);
    }
}
