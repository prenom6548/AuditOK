/*
 * Copyright (C) 2025, KylinSoft Co., Ltd.
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
 * Authors: an-yongtai <anyongtai@kylinos.cn>
 *
 */

#ifndef VPNSHORTCUT_H
#define VPNSHORTCUT_H
#include "ukui-shortcut-plugin.h"
#include <QDBusInterface>
#include <QDBusServiceWatcher>

namespace UkuiShortcut {
class VPNShortcutPlugin : public UkuiShortcutPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID UKUI_SHORTCUT_PLUGIN_IFACE_IID FILE "vpn-shortcut.json")
    Q_INTERFACES(UkuiShortcut::UkuiShortcutPlugin)
public:
    explicit VPNShortcutPlugin(QObject *parent = nullptr);
    QString pluginId() override { return QStringLiteral("VPN"); };
    QStringList translations() override;
    Shortcut *createShortcut() override;
};

class VPNShortcut : public Shortcut
{
    Q_OBJECT
public:
    explicit VPNShortcut(QObject *parent = nullptr);
    ~VPNShortcut() override;
    QString pluginId() override;
    QMap<PluginMetaType::SystemMode, PluginMetaData> pluginMetaData() override;
    void active(PluginMetaType::Action action) override;
    const StatusInfo currentStatus() override;
    bool isEnable() override;
    void setAddedState(bool added) override;

private Q_SLOTS:
    void onServiceRegistered();
    void onServiceUnregistered();

private:
    void trigger();
    bool initVpnManagerDbusInterface();
    void initVpnServiceWatcher();
    inline void initMetaData();

private:
    bool m_isEnable = false;
    bool m_isAdded = false;
    StatusInfo m_currentStatusInfo;
    QMap<PluginMetaType::SystemMode, PluginMetaData> m_metaData;
    QDBusInterface* m_vpnManagerDbusInterface = nullptr;
    QDBusServiceWatcher *m_vpnServiceWatcher = nullptr;
};
}

#endif // VPNSHORTCUT_H
