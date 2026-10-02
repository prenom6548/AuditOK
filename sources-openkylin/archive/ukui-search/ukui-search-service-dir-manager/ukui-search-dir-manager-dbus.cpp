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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 */
#include "ukui-search-dir-manager-dbus.h"
#include "dir-watcher.h"
#include <cstdio>
#include <QCommandLineParser>
#include <QDBusConnection>
#include <QDBusError>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QProcess>

namespace {

QString encodeForwardedArguments(const QStringList &args)
{
    return QString::fromUtf8(QJsonDocument(QJsonArray::fromStringList(args))
                                 .toJson(QJsonDocument::Compact));
}

QStringList decodeForwardedArguments(const QString &payload)
{
    const QJsonDocument doc = QJsonDocument::fromJson(payload.toUtf8());
    if (doc.isArray()) {
        QStringList args;
        const QJsonArray array = doc.array();
        args.reserve(array.size());
        for (const QJsonValue &value : array) {
            args.append(value.toString());
        }
        return args;
    }

    return QProcess::splitCommand(payload);
}

} // namespace

UkuiSearchDirManagerDbus::UkuiSearchDirManagerDbus(int &argc, char *argv[], const QString &applicationName)
    : QtSingleCoreApplication (applicationName, argc, argv)
{
    setApplicationVersion(QString("v%1").arg(VERSION));

    if (!this->isRunning()) {
        qDebug() << "I'm in dir manager dbus rigister";
        QDBusConnection sessionBus = QDBusConnection::sessionBus();
        if (!sessionBus.isConnected()) {
            qCritical() << "QDbus session bus is not connected:" << sessionBus.lastError();
            m_initializationSucceeded = false;
            return;
        }
        if(!sessionBus.registerObject("/org/ukui/search/privateDirWatcher", DirWatcher::getDirWatcher(), QDBusConnection::ExportNonScriptableSlots | QDBusConnection::ExportAllSignals)) {
            qCritical() << "ukui-search-fileindex dbus register object failed reason:" << sessionBus.lastError();
            m_initializationSucceeded = false;
            return;
        }
        if(!sessionBus.registerService("com.ukui.search.fileindex.service")) {
            qCritical() << "QDbus register service failed reason:" << sessionBus.lastError();
            m_initializationSucceeded = false;
            return;
        }
        connect(this, &QtSingleCoreApplication::messageReceived, [this](const QString &msg) {
            this->parseCmd(decodeForwardedArguments(msg), true);
        });
    } else {
        qWarning() << "Ukui search dir manager is running!";
    }
    //parse cmd
    qDebug() << "parse cmd";
    parseCmd(arguments(), !isRunning());
}

bool UkuiSearchDirManagerDbus::initializationSucceeded() const
{
    return m_initializationSucceeded;
}

void UkuiSearchDirManagerDbus::parseCmd(const QStringList &args, bool isPrimary)
{
    QCommandLineParser parser;

    parser.addHelpOption();
    parser.addVersionOption();

    const QCommandLineOption quitOption(QStringList() << "q" << "quit",
                                        tr("Quit DBus service"));
    parser.addOption(quitOption);

    if (!parser.parse(args)) {
        fprintf(stderr, "%s\n\n", qPrintable(parser.errorText()));
        parser.showHelp(EXIT_FAILURE);
    }

    const QStringList positionalArgs = parser.positionalArguments();
    if (!positionalArgs.isEmpty()) {
        fprintf(stderr, "%s\n\n", qPrintable(tr("Unknown argument(s): %1").arg(positionalArgs.join(' '))));
        parser.showHelp(EXIT_FAILURE);
    }

    if (parser.isSet("help")) {
        parser.showHelp();
    }
    if (parser.isSet("version")) {
        parser.showVersion();
    }

    if (isPrimary) {
        if (parser.isSet(quitOption)) {
            qApp->quit();
        }
        return;
    }

    if (!sendMessage(encodeForwardedArguments(args))) {
        qWarning() << "Failed to forward command to primary instance";
    }
}
