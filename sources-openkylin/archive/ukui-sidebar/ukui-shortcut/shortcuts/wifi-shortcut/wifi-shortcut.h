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

#ifndef WIFISHORTCUT_H
#define WIFISHORTCUT_H

#define WIRELESS_ICON                  "network-wireless-24-symbolic"
#define WIRELESS_LOTTIE                "network-wireless-signal-excellent-24-symbolic-motion.lottie"
#define WIRED_ICON                     "network-wired-24-symbolic"
#define WIRED_LOTTIE                   "network-wired-24-symbolic-motion.lottie"

#define KYLIN_NETWORK_NAME             "com.kylin.network"
#define KYLIN_NETWORK_PATH             "/com/kylin/network"
#define KYLIN_NETWORK_INTERFACE        "com.kylin.network"

#include "ukui-shortcut-plugin.h"
#include "wifi-dbus-query.h"

#include <QObject>
#include <QProcess>
#include <QDBusInterface>
#include <QDBusReply>
#include <QThread>

namespace UkuiShortcut {

class WiFiShortcutPlugin : public UkuiShortcutPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID UKUI_SHORTCUT_PLUGIN_IFACE_IID FILE "wifi-shortcut.json")
    Q_INTERFACES(UkuiShortcut::UkuiShortcutPlugin)
public:
    explicit WiFiShortcutPlugin(QObject *parent = nullptr);

    QString pluginId() override { return QStringLiteral("WiFi"); };
    QStringList translations() override;
    Shortcut *createShortcut() override;
};

class WiFiShortcut : public Shortcut
{
    Q_OBJECT
public:
    explicit WiFiShortcut(QObject *parent = nullptr);
    ~WiFiShortcut() override;
    QString pluginId() override;
    QMap<PluginMetaType::SystemMode, PluginMetaData> pluginMetaData() override;
    void active(PluginMetaType::Action action) override;
    const StatusInfo currentStatus() override;
    bool isEnable() override;
    void setAddedState(bool added) override;

private:
    inline void initMetaData();

    void initConnect();
    void endQueryThread();
    void endNetworkDbus();
    void endConnect();

    void onNetworkDeviceDataChanged(QVariantList data);
    void updateStatus();

    void kyWifiButtonActive();

private Q_SLOTS:
    void onDbusConnectorReady(QDBusInterface* interface);
    void networkDeviceDataChanged();

private:
    QMap<PluginMetaType::SystemMode, PluginMetaData> m_metaData;
    StatusInfo      m_currentStatus;
    bool            m_isEnable = false;
    bool            m_isAdded = false;

    QThread        *m_wifiDbusThread = nullptr;
    WifiDbusQuery  *m_wifiDbusQuery = nullptr;
    QDBusInterface *m_networkInterface = nullptr;

    bool            m_isWireEnable = false;
    bool            m_isWireOpen = false;
    QString         m_wireName = "";
    bool            m_isWirelessEnable = false;
    bool            m_isWirelessOpen = false;
    QString         m_wirelessName = "";

Q_SIGNALS:
    void toGetNetworkDeviceData(QDBusInterface *networkInterface);
};


}
#endif // NIGHTMODESHORTCUT_H
