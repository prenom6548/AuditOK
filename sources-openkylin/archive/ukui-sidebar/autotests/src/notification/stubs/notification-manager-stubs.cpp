/*
* Copyright (C) 2026, KylinSoft Co., Ltd.
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

#include "notification-helper.h"
#include "app-manager.h"

namespace Notify {

Message::Message(QString id, QString appName, bool defaultActionResult)
    : m_id(std::move(id)), m_appName(std::move(appName)), m_defaultActionResult(defaultActionResult)
{
}

QString Message::getID() const
{
    return m_id;
}

QString Message::getAppName() const
{
    return m_appName;
}

bool Message::executeDefaultAction() const
{
    return m_defaultActionResult;
}

NotificationHelper *NotificationHelper::instance()
{
    static NotificationHelper helper;
    return &helper;
}

NotificationHelper::NotificationHelper(QObject *parent) : QObject(parent)
{
}

void NotificationHelper::emitNewMessage(Message &message)
{
    Q_EMIT newMessage(message);
}

} // namespace Notify

namespace Sidebar {

static QString g_lastDesktopFile;
static QStringList g_lastArgs;
static QString g_lastAppName;

AppManager::AppManager(QObject *parent) : QObject(parent)
{
}

AppManager *AppManager::getInstance(QObject *parent)
{
    Q_UNUSED(parent);
    static AppManager manager(nullptr);
    return &manager;
}

bool AppManager::launchAppWithArguments(const QString &desktopFile, const QStringList &args, const QString &appName)
{
    g_lastDesktopFile = desktopFile;
    g_lastArgs = args;
    g_lastAppName = appName;
    return true;
}

void AppManager::reset()
{
    g_lastDesktopFile.clear();
    g_lastArgs.clear();
    g_lastAppName.clear();
}

QString AppManager::lastDesktopFile()
{
    return g_lastDesktopFile;
}

QStringList AppManager::lastArgs()
{
    return g_lastArgs;
}

QString AppManager::lastAppName()
{
    return g_lastAppName;
}

} // namespace Sidebar
