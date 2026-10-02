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
 * Authors: youdiansaodongxi <guojiaqi@kylinos.cn>
 *
 */

#include "DND-mode-shortcut.h"
#include "dbus-connector.h"

#include <QDBusConnection>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDebug>
#include <QIcon>
#include <QFile>

#define NOTIFICATION_SERVICE                "org.freedesktop.Notifications"
#define NOTIFICATION_PATH                   "/org/freedesktop/Notifications"
#define NOTIFICATION_SETTINGS_INTERFACE     "org.ukui.NotificationSettings"
#define IS_DND_METHOD                       "IsDND"
#define SET_DND_METHOD                      "SetDND"
#define DND_CHANGED_SIGNAL                  "DNDChanged"
#define DND_ICON                            "ukui-quiet-24-symbolic"
#define DND_LOTTIE                          "ukui-quiet-24-symbolic-motion.lottie"
#define THEME_LOTTIE_PATH                   "/usr/share/icons/ukui-motion-icons"

using namespace UkuiShortcut;
using namespace UkuiQuick;

DNDModeShortcut::DNDModeShortcut(QObject *parent) : Shortcut(parent)
{
    m_currentStatus.setName(tr("DND Mode"));
    m_currentStatus.setIcon(QFile::exists(THEME_LOTTIE_PATH "/" DND_LOTTIE)
                            ? DND_LOTTIE
                            : (QIcon::fromTheme(DND_ICON).isNull()
                               ? ":/DNDModeShortcut/ukui-quiet.svg" : DND_ICON));
    m_currentStatus.setToolTip(tr("DND Mode"));
    m_currentStatus.setColor(Color::ColorRole::BaseColor);

    initMetaData();
    initServiceWatcher();
    initDbusConnector();
}

DNDModeShortcut::~DNDModeShortcut()
{
    disconnectDNDChangedSignal();
    clearDbusConnector();
    clearDNDInterface();
}

void DNDModeShortcut::initDbusConnector()
{
    if (m_isConnecting || m_dbusConnector) {
        return;
    }

    m_isConnecting = true;
    m_dbusConnector = new DbusConnector(NOTIFICATION_SERVICE,
                                        NOTIFICATION_PATH,
                                        NOTIFICATION_SETTINGS_INTERFACE,
                                        QDBusConnection::sessionBus(),
                                        this);
    connect(m_dbusConnector, &DbusConnector::ready, this, &DNDModeShortcut::onDbusConnectorReady);
    m_dbusConnector->start();
}

void DNDModeShortcut::initServiceWatcher()
{
    m_serviceWatcher = new QDBusServiceWatcher(NOTIFICATION_SERVICE,
                                               QDBusConnection::sessionBus(),
                                               QDBusServiceWatcher::WatchForRegistration |
                                               QDBusServiceWatcher::WatchForUnregistration,
                                               this);
    connect(m_serviceWatcher, &QDBusServiceWatcher::serviceRegistered, this, &DNDModeShortcut::onServiceRegistered);
    connect(m_serviceWatcher, &QDBusServiceWatcher::serviceUnregistered, this, &DNDModeShortcut::onServiceUnregistered);
}

void DNDModeShortcut::onDbusConnectorReady(QDBusInterface *interface)
{
    clearDbusConnector();
    if (!interface || !interface->isValid()) {
        qWarning() << "DNDModeShortcut:" << "dbus interface invalid";
        if (interface != m_dndInterface) {
            delete interface;
        }
        clearDNDInterface();
        setEnable(false);
        return;
    }

    if (interface != m_dndInterface) {
        clearDNDInterface();
    }
    m_dndInterface = interface;
    setEnable(true);

    if (m_isAdded) {
        connectDNDChangedSignal();
        requestDNDState();
    }
}

void DNDModeShortcut::requestDNDState()
{
    if (!hasValidDNDInterface()) {
        setEnable(false);
        return;
    }

    auto watcher = new QDBusPendingCallWatcher(m_dndInterface->asyncCall(IS_DND_METHOD), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *watcher) {
        QDBusPendingReply<bool> reply = *watcher;
        if (reply.isError()) {
            qWarning() << "DNDModeShortcut:" << "call" << IS_DND_METHOD << "failed" << reply.error().message();
        } else {
            setDNDState(reply.value());
        }
        watcher->deleteLater();
    });
}

void DNDModeShortcut::connectDNDChangedSignal()
{
    if (m_dndChangedSignalConnected) {
        return;
    }

    m_dndChangedSignalConnected = QDBusConnection::sessionBus().connect(NOTIFICATION_SERVICE,
                                                                        NOTIFICATION_PATH,
                                                                        NOTIFICATION_SETTINGS_INTERFACE,
                                                                        DND_CHANGED_SIGNAL,
                                                                        this,
                                                                        SLOT(onDNDChanged(bool)));
    if (!m_dndChangedSignalConnected) {
        qWarning() << "DNDModeShortcut:" << "connect" << DND_CHANGED_SIGNAL << "failed";
    }
}

void DNDModeShortcut::disconnectDNDChangedSignal()
{
    if (!m_dndChangedSignalConnected) {
        return;
    }

    QDBusConnection::sessionBus().disconnect(NOTIFICATION_SERVICE,
                                             NOTIFICATION_PATH,
                                             NOTIFICATION_SETTINGS_INTERFACE,
                                             DND_CHANGED_SIGNAL,
                                             this,
                                             SLOT(onDNDChanged(bool)));
    m_dndChangedSignalConnected = false;
}

void DNDModeShortcut::onServiceRegistered()
{
    if (!hasValidDNDInterface()) {
        initDbusConnector();
    }
}

void DNDModeShortcut::onServiceUnregistered()
{
    disconnectDNDChangedSignal();
    clearDNDInterface();
    setEnable(false);
    setDNDState(false);
}

void DNDModeShortcut::clearDNDInterface()
{
    delete m_dndInterface;
    m_dndInterface = nullptr;
}

void DNDModeShortcut::clearDbusConnector()
{
    if (!m_dbusConnector) {
        return;
    }

    auto connector = m_dbusConnector;
    m_dbusConnector = nullptr;
    m_isConnecting = false;
    disconnect(connector, &DbusConnector::ready, this, &DNDModeShortcut::onDbusConnectorReady);
    connector->deleteLater();
}

bool DNDModeShortcut::hasValidDNDInterface() const
{
    return m_dndInterface && m_dndInterface->isValid();
}

void DNDModeShortcut::onDNDChanged(bool enabled)
{
    setDNDState(enabled);
}

bool DNDModeShortcut::isEnable()
{
    return m_isEnable;
}

void DNDModeShortcut::setAddedState(bool added)
{
    if (m_isAdded == added) {
        return;
    }

    m_isAdded = added;
    if (m_isAdded) {
        if (!hasValidDNDInterface()) {
            initDbusConnector();
        } else {
            connectDNDChangedSignal();
            requestDNDState();
        }
    } else {
        disconnectDNDChangedSignal();
    }
    updateIconButtonColor();
}

void DNDModeShortcut::setEnable(bool enabled)
{
    if (m_isEnable == enabled) {
        return;
    }

    m_isEnable = enabled;
    Q_EMIT enableStatusChanged(m_isEnable);
}

void DNDModeShortcut::setDNDState(bool enabled)
{
    if (m_isDND == enabled) {
        return;
    }

    m_isDND = enabled;
    updateIconButtonColor();
}

void DNDModeShortcut::updateIconButtonColor()
{
    Color::ColorRole color = (m_isDND && m_isAdded)
            ? Color::ColorRole::HighLight
            : Color::ColorRole::BaseColor;

    if (m_currentStatus.getColor() == color) {
        return;
    }

    m_currentStatus.setColor(color);
    Q_EMIT statusChanged(m_currentStatus);
}

void DNDModeShortcut::active(PluginMetaType::Action action)
{
    if (action == PluginMetaType::Action::Click) {
        trigger();
    }
}

const StatusInfo DNDModeShortcut::currentStatus()
{
    return m_currentStatus;
}

QMap<PluginMetaType::SystemMode, PluginMetaData> DNDModeShortcut::pluginMetaData()
{
    return m_metaData;
}

void DNDModeShortcut::initMetaData()
{
    PluginMetaData pc {true, 7, PluginMetaType::PluginType::Icon};
    PluginMetaData tablet {true, 7, PluginMetaType::PluginType::Icon};
    m_metaData.insert(PluginMetaType::SystemMode::PC, pc);
    m_metaData.insert(PluginMetaType::SystemMode::Tablet, tablet);
}

void DNDModeShortcut::trigger()
{
    if (!m_isEnable || !m_isAdded || !hasValidDNDInterface()) {
        return;
    }

    const bool targetState = !m_isDND;
    m_dndInterface->asyncCall(SET_DND_METHOD, targetState);
    setDNDState(targetState);
}

DNDModeShortcutPlugin::DNDModeShortcutPlugin(QObject *parent) : UkuiShortcutPlugin(parent)
{

}

QStringList DNDModeShortcutPlugin::translations()
{
    return {QStringLiteral("DND-mode-shortcut")};
}

Shortcut *DNDModeShortcutPlugin::createShortcut()
{
    return new DNDModeShortcut;
}
