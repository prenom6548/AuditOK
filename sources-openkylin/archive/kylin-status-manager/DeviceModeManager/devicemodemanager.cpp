/*
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
 */

#include "devicemodemanager.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QDBusReply>
#include <QThread>
#include <QDebug>
#include <unistd.h>
#include <QtConcurrent/qtconcurrentrun.h>

DeviceModeManager::DeviceModeManager(QObject *parent)
    : QObject(parent)
{
}

bool DeviceModeManager::isAutoSwitchEnabled() const
{
    return m_autoSwitchEnabled;
}

bool DeviceModeManager::isTabletMode() const
{
    QString sessionData = getenv("XDG_SESSION_DESKTOP");
    if(!sessionData.compare("kylin-wlcom")) {
        qDebug()<<__FUNCTION__<<__LINE__<<"iswlcom";
        return false;
    }

    return m_tabletMode;
}

bool DeviceModeManager::isTabletModeEnabled() const
{
    QString sessionData = getenv("XDG_SESSION_DESKTOP");
    if(!sessionData.compare("kylin-wlcom")) {
        qDebug()<<__FUNCTION__<<__LINE__<<"iswlcom";
        return false;
    }
    return m_isTabletModeEnabled;
}

void DeviceModeManager::setShowNotificationCallback(ShowNotificationCallback func)
{
    m_showNotification = std::move(func);
}

void DeviceModeManager::setShouldShowNotificationCallback(ShouldShowNotificationCallback func)
{
    m_shouldShowNotification = std::move(func);
}

void DeviceModeManager::setCheckFirstDeviceModeSwitchFunction(CheckFirstDeviceModeSwitchCallback func)
{
    m_checkFirstDeviceModeSwitch = std::move(func);
}

QMap<QString, QVariant> DeviceModeManager::getLiteMode() const
{
    return m_liteModeMap;
}

bool DeviceModeManager::isClassicalMode() const
{
    return m_isClassicalMode;
}

bool DeviceModeManager::isImmersiveMode() const
{
    return m_isImmersiveMode;
}

int DeviceModeManager::getDeviceAllMode()
{
    return 0;
}

bool DeviceModeManager::setAutoSwitchEnabled(bool autoSwitchEnabled)
{
    if (autoSwitchEnabled == m_autoSwitchEnabled) {
        return true;
    }
    m_autoSwitchEnabled = autoSwitchEnabled;
    Q_EMIT autoSwitchEnabledChanged(m_autoSwitchEnabled);
    return true;
}

void DeviceModeManager::requestToSwitchMode(bool tabletMode)
{
    if (m_tabletMode == tabletMode) {
        return;
    }

    if (shouldShowNotification()) {
        showNotification();
        return;
    }

    setTabletMode(tabletMode);
}

void DeviceModeManager::setTabletMode(bool tabletMode)
{
    if (m_tabletMode == tabletMode) {
        return;
    }
    m_tabletMode = tabletMode;
    Q_EMIT tabletModeChanged(tabletMode);

    if (shouldOpenUserGuide()) {
        QtConcurrent::run(this, &DeviceModeManager::openUserGuide);
    }
}

void DeviceModeManager::setTabletModeEnabled(bool enabled)
{
    QString sessionData = getenv("XDG_SESSION_DESKTOP");
    if(!sessionData.compare("kylin-wlcom")) {
        qDebug()<<__FUNCTION__<<__LINE__<<"iswlcom";
        return;
    }

    m_isTabletModeEnabled = enabled;
    Q_EMIT isTabletModeEnabledChanged(m_isTabletModeEnabled);
}

void DeviceModeManager::setClassicalMode(bool classicalMode)
{
    if(m_isClassicalMode == classicalMode) {
        return;
    }
    m_isClassicalMode = classicalMode;
    Q_EMIT isClassicalModeChanged(m_isClassicalMode);
}

void DeviceModeManager::setImmersiveMode(bool ImmersiveMode)
{
    if(m_isImmersiveMode == ImmersiveMode) {
        return;
    }

    m_isImmersiveMode = ImmersiveMode;
    Q_EMIT isImmersiveModeChanged(m_isImmersiveMode);
}

void DeviceModeManager::showNotification() const
{
    if (!m_showNotification) {
        return;
    }

    m_showNotification();
}

bool DeviceModeManager::shouldShowNotification() const
{
    return m_shouldShowNotification && m_shouldShowNotification();
}

bool DeviceModeManager::shouldOpenUserGuide() const
{
    return m_checkFirstDeviceModeSwitch && m_checkFirstDeviceModeSwitch();
}

void DeviceModeManager::openUserGuide()
{
    QString serviceName = "com.kylinUserGuide.hotel" + QString("%1%2").arg("_").arg(QString::number(getuid()));
    QDBusInterface interface(serviceName,
                             "/",
                             "com.guide.hotel",
                             QDBusConnection::sessionBus());
    if (!interface.isValid()) {
        qDebug() << "Failed to call kylinUserGuide DBus interface !!!";
        return;
    }

    QDBusPendingCall async = interface.asyncCall("showGuide", "hot-key");
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(async);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
            this, SLOT(callUserGuideFinishedSlot(QDBusPendingCallWatcher*)));
}

void DeviceModeManager::setLiteMode(QMap<QString, QVariant> liteMode)
{
    m_liteModeMap = liteMode;
}

void DeviceModeManager::callUserGuideFinishedSlot(QDBusPendingCallWatcher *watcher)
{
    if (watcher->isError()) {
        qDebug() << "open UserGuide failed !!!";
    }
    watcher->deleteLater();
}
