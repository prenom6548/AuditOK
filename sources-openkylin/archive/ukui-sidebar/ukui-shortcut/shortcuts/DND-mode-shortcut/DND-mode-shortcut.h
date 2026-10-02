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

#ifndef DNDMODESHORTCUT_H
#define DNDMODESHORTCUT_H

#include <QDBusInterface>
#include <QDBusServiceWatcher>
#include <QObject>

#include "ukui-shortcut-plugin.h"

namespace UkuiQuick {
class DbusConnector;
}

namespace UkuiShortcut {

class DNDModeShortcutPlugin : public UkuiShortcutPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID UKUI_SHORTCUT_PLUGIN_IFACE_IID FILE "DND-mode-shortcut.json")
    Q_INTERFACES(UkuiShortcut::UkuiShortcutPlugin)
public:
    explicit DNDModeShortcutPlugin(QObject *parent = nullptr);

    QString pluginId() override { return QStringLiteral("DNDModeShortcut"); };
    QStringList translations() override;
    Shortcut *createShortcut() override;
};

class DNDModeShortcut : public Shortcut
{
    Q_OBJECT
public:
    explicit DNDModeShortcut(QObject *parent = nullptr);
    ~DNDModeShortcut() override;

    QString pluginId() override { return QStringLiteral("DNDModeShortcut"); }
    QMap<PluginMetaType::SystemMode, PluginMetaData> pluginMetaData() override;
    void active(PluginMetaType::Action action) override;
    const StatusInfo currentStatus() override;
    bool isEnable() override;
    void setAddedState(bool added) override;

private Q_SLOTS:
    void onDbusConnectorReady(QDBusInterface *interface);
    void onServiceRegistered();
    void onServiceUnregistered();
    void onDNDChanged(bool enabled);

private:
    inline void initMetaData();
    void initDbusConnector();
    void initServiceWatcher();
    void trigger();
    void setEnable(bool enabled);
    void setDNDState(bool enabled);
    void requestDNDState();
    void connectDNDChangedSignal();
    void disconnectDNDChangedSignal();
    void clearDNDInterface();
    void clearDbusConnector();
    bool hasValidDNDInterface() const;
    void updateIconButtonColor();

private:
    QDBusInterface *m_dndInterface = nullptr;
    UkuiQuick::DbusConnector *m_dbusConnector = nullptr;
    QDBusServiceWatcher *m_serviceWatcher = nullptr;
    bool m_isConnecting = false;
    bool m_dndChangedSignalConnected = false;
    bool m_isEnable = false;
    bool m_isAdded = false;
    bool m_isDND = false;
    StatusInfo m_currentStatus;
    QMap<PluginMetaType::SystemMode, PluginMetaData> m_metaData;
};

}

#endif // DNDMODESHORTCUT_H
