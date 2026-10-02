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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */
#ifndef BLUETOOTHSHORTCUT_H
#define BLUETOOTHSHORTCUT_H
#include "ukui-shortcut-plugin.h"
#include "bluetooth-connector.h"
#include <QMutex>

namespace UkuiShortcut {

class BluetoothShortcutPlugin : public UkuiShortcutPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID UKUI_SHORTCUT_PLUGIN_IFACE_IID FILE "bluetooth-shortcut.json")
    Q_INTERFACES(UkuiShortcut::UkuiShortcutPlugin)
public:
    explicit BluetoothShortcutPlugin(QObject *parent = nullptr);

    QString pluginId() override { return QStringLiteral("BluetoothShortcut"); };
    QStringList translations() override;
    Shortcut *createShortcut() override;
};

class BluetoothShortcut: public Shortcut
{
    Q_OBJECT
public:
    BluetoothShortcut();

    QString pluginId() override {return QStringLiteral("BluetoothShortcut");}
    QMap<PluginMetaType::SystemMode, PluginMetaData> pluginMetaData() override;
    void active(PluginMetaType::Action action) override;
    const StatusInfo currentStatus() override;
    bool isEnable() override;
    void setAddedState(bool added) override;

private Q_SLOTS:
    void init(QDBusInterface *interface);
    void connectionFailed();

    void onAdapterChanged();
    void onDeviceChanged();

private:
    inline void initMetaData();

    void initConnect();
    void endConnect();
    void connectSignal(const QString &signal, const char *slot);
    void disconnectSignal(const QString &signal, const char *slot);
    void triggerBluetoothPower();
    void callLegacySetBluetoothPower(bool power);

    QMutex m_mutex;
    BluetoothConnector *m_connector = nullptr;
    QDBusInterface *m_bluetoothIface = nullptr;
    bool m_isEnable = false;
    bool m_isAdded = false;
    bool m_defaultAdapterPower = false;
    bool m_powerCallPending = false;
    StatusInfo m_statusInfo;
    QMap<PluginMetaType::SystemMode, PluginMetaData> m_metaData;
};
}

#endif // BLUETOOTHSHORTCUT_H
