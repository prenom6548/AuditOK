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

#include "notification-client.h"
#include "applications-settings.h"
#include "notification-global-settings.h"
#include "screen-monitor.h"

namespace UkuiNotification {

static QList<uint> g_closedIds;
static QList<uint> g_invokedIds;
static QStringList g_invokedActions;
static bool g_registerClientResult = true;
static SingleApplicationSettings g_appSettings;
static NotificationGlobalSettings *g_lastGlobalSettings = nullptr;

NotificationClient::NotificationClient(QObject *parent) : QObject(parent)
{
}

bool NotificationClient::registerClient()
{
    return g_registerClientResult;
}

void NotificationClient::invokeAction(uint id, const QString &action)
{
    g_invokedIds.append(id);
    g_invokedActions.append(action);
}

void NotificationClient::closeNotification(uint id, NotificationCloseReason::CloseReason)
{
    g_closedIds.append(id);
}

void NotificationClient::reset()
{
    g_closedIds.clear();
    g_invokedIds.clear();
    g_invokedActions.clear();
    g_registerClientResult = true;
}

QList<uint> NotificationClient::closedIds()
{
    return g_closedIds;
}

QList<uint> NotificationClient::invokedIds()
{
    return g_invokedIds;
}

QStringList NotificationClient::invokedActions()
{
    return g_invokedActions;
}

void NotificationClient::setRegisterClientResult(bool value)
{
    g_registerClientResult = value;
}

SingleApplicationSettings::SingleApplicationSettings(QObject *parent) : QObject(parent)
{
}

bool SingleApplicationSettings::allowNotify() const
{
    return m_allowNotify;
}

bool SingleApplicationSettings::allowSound() const
{
    return m_allowSound;
}

bool SingleApplicationSettings::showNotificationOnLockScreen() const
{
    return m_showOnLockScreen;
}

bool SingleApplicationSettings::showContentOnLockScreen() const
{
    return m_showContentOnLockScreen;
}

void SingleApplicationSettings::setAllowNotify(bool value)
{
    m_allowNotify = value;
}

void SingleApplicationSettings::setAllowSound(bool value)
{
    m_allowSound = value;
}

void SingleApplicationSettings::setShowNotificationOnLockScreen(bool value)
{
    m_showOnLockScreen = value;
}

void SingleApplicationSettings::setShowContentOnLockScreen(bool value)
{
    m_showContentOnLockScreen = value;
}

ApplicationsSettings *ApplicationsSettings::self()
{
    static ApplicationsSettings settings;
    return &settings;
}

bool ApplicationsSettings::isInAllowList(const QString &) const
{
    return false;
}

SingleApplicationSettings *ApplicationsSettings::creatSettings(const PopupNotification &)
{
    return &g_appSettings;
}

SingleApplicationSettings *ApplicationsSettings::settings()
{
    return &g_appSettings;
}

NotificationGlobalSettings::NotificationGlobalSettings(QObject *parent) : QObject(parent)
{
    g_lastGlobalSettings = this;
}

bool NotificationGlobalSettings::receiveNotificationsFromApps() const
{
    return m_receiveNotificationsFromApps;
}

bool NotificationGlobalSettings::isDND() const
{
    return m_isDnd;
}

bool NotificationGlobalSettings::notifyAlarmWhileDND() const
{
    return m_notifyAlarmWhileDnd;
}

void NotificationGlobalSettings::setReceiveNotificationsFromApps(bool value)
{
    m_receiveNotificationsFromApps = value;
}

void NotificationGlobalSettings::setDND(bool value)
{
    m_isDnd = value;
}

void NotificationGlobalSettings::setNotifyAlarmWhileDND(bool value)
{
    m_notifyAlarmWhileDnd = value;
}

NotificationGlobalSettings *NotificationGlobalSettings::lastInstance()
{
    return g_lastGlobalSettings;
}

} // namespace UkuiNotification
