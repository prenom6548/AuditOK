/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 */

#include <QCommandLineParser>
#include <QDebug>
#include <utility>
#include "status-notifier-watcher-application.h"

#include <QVariant>

#include "ukui-sni-watcher-config.h"
#include "watcher.h"

StatusNotifierWatcherApplication::StatusNotifierWatcherApplication(int &argc, char **argv,
                                                                   const QString &applicationName)
        : QtSingleCoreApplication(applicationName, argc, argv)
{
    setApplicationVersion(UKUI_SNI_WATCHER_VERSION);
    qApp->setProperty("IS_UKUI_NOTIFICATION_SERVER", QVariant(true));
    if (!this->isRunning()) {
        connect(this, &QtSingleCoreApplication::messageReceived, [=](QString msg) {
            this->parseCmd(std::move(msg), true);
        });
        Watcher::self();
    }
    parseCmd(arguments().join(" ").toUtf8(), !isRunning());
}

void StatusNotifierWatcherApplication::parseCmd(const QString& msg, bool isPrimary)
{
    QCommandLineParser parser;

    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption quitOption(QStringList()<<"q"<<"quit", tr("Quit ukui-sni-watcher"));
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

StatusNotifierWatcherApplication::~StatusNotifierWatcherApplication()
{
}
