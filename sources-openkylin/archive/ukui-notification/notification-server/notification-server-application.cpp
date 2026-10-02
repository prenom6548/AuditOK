/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
#include <QTranslator>
#include "notification-server-application.h"

#include <QVariant>
#include <QLocale>
#include "notification-server-config.h"
#include "server.h"
using namespace NotificationServer;
NotificationServerApplication::NotificationServerApplication(int &argc, char **argv, const QString &applicationName)
    : QtSingleCoreApplication(applicationName, argc, argv)
{
    setApplicationVersion(NOTIFICATION_SERVER_VERSION);
    qApp->setProperty("IS_UKUI_NOTIFICATION_SERVER", QVariant(true));
    if (!this->isRunning()) {
        connect(this, &QtSingleCoreApplication::messageReceived, [=](QString msg) {
            this->parseCmd(msg, true);
        });
        Server::self().init();
        installQTranslator();
    }
    parseCmd(arguments().join(" ").toUtf8(), !isRunning());
}

NotificationServerApplication::~NotificationServerApplication()
{
}

void NotificationServerApplication::parseCmd(QString msg, bool isPrimary)
{
    QCommandLineParser parser;

    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption quitOption(QStringList()<<"q"<<"quit", tr("Quit notification server"));
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

void NotificationServerApplication::installQTranslator()
{
    QTranslator* translator = new QTranslator(this);
    if(!translator->load(QLocale(), QStringLiteral(""), QStringLiteral(""), QStringLiteral("/usr/share/ukui-notification/notification-server/translations/"))) {
        qWarning() << "/usr/share/ukui-notification/notification-server/translations/" + QLocale::system().name() << "load failed";
    }
    qApp->installTranslator(translator);
}
