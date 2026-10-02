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

#include "dbusservice.h"
#include "StatusManager/statusmanager.h"
#include "interface_adaptor.h"


DBusService::DBusService(StatusManager *manager)
    : QObject(manager)
    , m_statusManager(manager)
    , m_serviceName("com.kylin.statusmanager.interface")
{
    startService();
    connectStatusManagerSignal();
    connectSignals();
}

DBusService::~DBusService()
{
    stopService();
}

void DBusService::startService()
{
    m_adaptor = new InterfaceAdaptor(this);

    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject(QStringLiteral("/"), this);
    dbus.registerService(m_serviceName);
}

void DBusService::stopService()
{
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.unregisterObject(QStringLiteral("/"));
    dbus.unregisterService(m_serviceName);
}

void DBusService::connectSignals()
{
    connectSignal(SIGNAL(autoRotateEnabledChange(bool)),
                  SIGNAL(auto_rotation_change_signal(bool)),
                  SIGNAL(autoRotateEnabledChangeSignal(bool)));

    connectSignal(SIGNAL(keyboardAvailableChanged(bool)),
                  SIGNAL(inputmethod_change_signal(bool)),
                  SIGNAL(builtinKeyboardAvailableChangedSignal(bool)));

    connectSignal(SIGNAL(tabletModeChanged(bool)),
                  SIGNAL(mode_change_signal(bool)),
                  SIGNAL(modeChangeSignal(bool)));

    connectSignal(SIGNAL(autoSwitchEnabledChanged(bool)),
                  SIGNAL(modemonitor_change_signal(bool)),
                  SIGNAL(autoSwitchEnabledChangedSignal(bool)));

    connectSignal(SIGNAL(orientationChanged(QString)),
                  SIGNAL(rotations_change_signal(QString)),
                  SIGNAL(orientationChangedSignal(QString)));

    connect(this, SIGNAL(isTabletModeEnabledChanged(bool)),
            m_adaptor,
            SIGNAL(isTabletModeEnabledChanged(bool)));
}

void DBusService::connectSignal(const char* fromSignal,
                                const char* toOldSignal,
                                const char* toNewSignal)
{
    connect(this, fromSignal, m_adaptor, toOldSignal);
    connect(this, fromSignal, m_adaptor, toNewSignal);
}

void DBusService::connectStatusManagerSignal()
{
    connect(m_statusManager, &StatusManager::autoRotateEnabledChange,
            this, &DBusService::autoRotateEnabledChange);
    connect(m_statusManager, &StatusManager::keyboardAvailableChanged,
            this, &DBusService::keyboardAvailableChanged);
    connect(m_statusManager, &StatusManager::tabletModeChanged,
            this, &DBusService::tabletModeChanged);
    connect(m_statusManager, &StatusManager::autoSwitchEnabledChanged,
            this, &DBusService::autoSwitchEnabledChanged);
    connect(m_statusManager, &StatusManager::orientationChanged,
            this, &DBusService::orientationChanged);
    connect(m_statusManager, &StatusManager::isTabletModeEnabledChanged,
            this, &DBusService::isTabletModeEnabledChanged);
    connect(m_statusManager, &StatusManager::isClassicalModeChanged,
            this, &DBusService::isClassicalModeChanged);
    connect(m_statusManager, &StatusManager::isImmersiveModeChanged,
            this, &DBusService::isImmersiveModeChanged);
}

bool DBusService::isAutoRotateEnabled() const
{
    return m_statusManager->isAutoRotateEnabled();
}

QString DBusService::getDeviceOrientation()
{
    return m_statusManager->getDeviceOrientation();
}

bool DBusService::isTabletMode() const
{
    return m_statusManager->isTabletMode();
}

bool DBusService::getCurrentTabletMode() const
{
    return isTabletMode();
}

bool DBusService::isBuiltinKeyboardAvailable() const
{
    return m_statusManager->isKeyboardAvailable();
}

bool DBusService::isAutoSwitchEnabled() const
{
    return m_statusManager->isAutoSwitchEnabled();
}

bool DBusService::isSupportedAutoRotation() const
{
    return m_statusManager->isOrientationSensorAvailable();
}

bool DBusService::setAutoRotateEnabled(bool autoRotateEnable, const QString &who, const QString &why)
{
    qDebug() << __FILE__ << __func__ << who << why << autoRotateEnable;
    return m_statusManager->setAutoRotateEnabled(autoRotateEnable, true);
}

bool DBusService::setAutoSwitchEnabled(bool autoSwitchEnable, const QString &who, const QString &why)
{
    qDebug() << __FILE__ << __func__ << who << why << autoSwitchEnable;
    return m_statusManager->setAutoSwitchEnabled(autoSwitchEnable);
}

bool DBusService::setDeviceOrientation(const QString &orientation, const QString &who, const QString &why)
{
    qDebug() << __FILE__ << __func__ << who << why << orientation;
    return m_statusManager->setDeviceOrientationPermanent(orientation);
}

bool DBusService::setTabletMode(bool tabletMode, const QString &who, const QString &why)
{
    qDebug() << __FILE__ << __func__ << who << why << tabletMode;
    return m_statusManager->setTabletMode(tabletMode);
}

bool DBusService::isTabletModeEnabled() const
{
    return m_statusManager->isTabletModeEnabled();
}

void DBusService::setTabletModeEnabled(bool enabled)
{
    return m_statusManager->setTabletModeEnabled(enabled);
}

bool DBusService::get_auto_rotation() const
{
    return isAutoRotateEnabled();
}

QString DBusService::get_current_rotation()
{
    return getDeviceOrientation();
}

bool DBusService::get_current_tabletmode() const
{
    return getCurrentTabletMode();
}

int DBusService::get_inputmethod_mode() const
{
    return int(isBuiltinKeyboardAvailable());
}

bool DBusService::get_modemonitor() const
{
    return isAutoSwitchEnabled();
}

bool DBusService::is_supported_autorotation() const
{
    return isSupportedAutoRotation();
}

bool DBusService::set_auto_rotation(bool autoRotation, const QString &who, const QString &why)
{
    qDebug() << __FILE__ << __func__ << who << why << autoRotation;
    return setAutoRotateEnabled(autoRotation, who, why);
}

bool DBusService::set_modemonitor(bool modeMonitor, const QString &who, const QString &why)
{
    qDebug() << __FILE__ << __func__ << who << why << modeMonitor;
    return setAutoSwitchEnabled(modeMonitor, who, why);
}

bool DBusService::set_rotation(const QString &rotation, const QString &who, const QString &why)
{
    qDebug() << __FILE__ << __func__ << who << why << rotation;
    return setDeviceOrientation(rotation, who, why);
}

bool DBusService::set_tabletmode(bool tabletMode, const QString &who, const QString &why)
{
    qDebug() << __FILE__ << __func__ << who << why << tabletMode;
    return setTabletMode(tabletMode, who, why);
}

void DBusService::setClassicalMode(bool classicalMode, const QString &why)
{
    qDebug()<<__func__<<why<<classicalMode;
    m_statusManager->setClassicalMode(classicalMode);
}

bool DBusService::isClassicalMode()
{
    return m_statusManager->isClassicalMode();
}

bool DBusService::isImmersiveMode()
{
    if (isTabletMode()) {
        sendErrorReply(QDBusError::Failed,"don't get immersive mode in tablet.");
        return false;
    }

    return m_statusManager->isImmersiveMode();
}

void DBusService::setImmersiveMode(bool immersiveMode, const QString &why)
{
    if (isTabletMode()) {
        sendErrorReply(QDBusError::Failed,"don't set immersive mode in tablet.");
        return;
    }

    return m_statusManager->setImmersiveMode(immersiveMode);
}

QString DBusService::OSAnimation()
{
    return m_statusManager->getLiteMode()["animation"].toString();
}

QString DBusService::OSFunction()
{
    return m_statusManager->getLiteMode()["function"].toString();
}
