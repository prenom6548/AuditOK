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
 *
 */

#include "app-manager.h"

#include <QDebug>
#include <QProcess>
#include <QDBusReply>
#include <QDBusInterface>
#include <QDBusPendingCallWatcher>

#define KYLIN_APP_MANAGER_NAME         "com.kylin.ProcessManager"
#define KYLIN_APP_MANAGER_PATH         "/com/kylin/ProcessManager/AppLauncher"
#define KYLIN_APP_MANAGER_INTERFACE    "com.kylin.ProcessManager.AppLauncher"

namespace Sidebar {

static AppManager *globalInstance = nullptr;

AppManager *AppManager::getInstance(QObject *parent)
{
    if (!globalInstance) {
        globalInstance = new AppManager(parent);
    }

    return globalInstance;
}

void AppManager::launchApp(const QString &desktopFile, const QString &applicationName)
{
    QDBusMessage message = QDBusMessage::createMethodCall(KYLIN_APP_MANAGER_NAME,
                                                          KYLIN_APP_MANAGER_PATH,
                                                          KYLIN_APP_MANAGER_INTERFACE,
                                                          "LaunchApp");
    message << desktopFile;

    auto watcher = new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(message), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [applicationName] (QDBusPendingCallWatcher *self) {
        if (self) {
            if (self->isError()) {
                qWarning() << self->error().message();
                QProcess::startDetached(applicationName, QStringList(), "");
            }
            self->deleteLater();
        }
    });
}

void AppManager::launchAppWithArguments(const QString &desktopFile, const QStringList &args, const QString &applicationName)
{
    QDBusMessage message = QDBusMessage::createMethodCall(KYLIN_APP_MANAGER_NAME,
                                                          KYLIN_APP_MANAGER_PATH,
                                                          KYLIN_APP_MANAGER_INTERFACE,
                                                          "LaunchAppWithArguments");
    message << desktopFile << args;

    auto watcher = new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(message), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [applicationName, args] (QDBusPendingCallWatcher *self) {
        if (self) {
            if (self->isError()) {
                qWarning() << self->error().message();
                QProcess::startDetached(applicationName, args, "");
            }
            self->deleteLater();
        }
    });
}

AppManager::AppManager(QObject *parent) : QObject(parent)
{

}

AppManager::~AppManager()
= default;

} // Sidebar
