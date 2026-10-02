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
#include "bluetooth-shortcut.h"

#include <QDebug>
#include <QDBusReply>
#include <QDBusError>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QMutexLocker>
#include <QTranslator>
#include <QApplication>
#include <QFile>

#define BLUETOOTH_ACTIVE_SYMBOLIC   "bluetooth-24-symbolic"
#define BLUETOOTH_LOTTIE            "bluetooth-24-symbolic-motion.lottie"
#define THEME_LOTTIE_PATH           "/usr/share/icons/ukui-motion-icons"

using namespace UkuiShortcut;
BluetoothShortcut::BluetoothShortcut()
{
    m_statusInfo.setIcon(QFile::exists(THEME_LOTTIE_PATH "/" BLUETOOTH_LOTTIE) ? BLUETOOTH_LOTTIE : BLUETOOTH_ACTIVE_SYMBOLIC);
    m_statusInfo.setColor(Color::Danger);
    m_statusInfo.setName(tr("Bluetooth"));
    m_statusInfo.setToolTip(tr("Bluetooth"));
    m_statusInfo.setWidgetId("org.ukui.shortcut.bluetooth");

    initMetaData();

    m_connector = new BluetoothConnector(this);
    connect(m_connector, &BluetoothConnector::ready, this, &BluetoothShortcut::init);
    connect(m_connector, &BluetoothConnector::failed, this, &BluetoothShortcut::connectionFailed);
    m_connector->start();
}

void BluetoothShortcut::active(PluginMetaType::Action action)
{
    if (action == PluginMetaType::Action::Click) {
        triggerBluetoothPower();
        return;
    }

    QMutexLocker locker(&m_mutex);
    if(!m_bluetoothIface) {
        return;
    }
    QDBusReply<bool> reply;
    switch (action) {
    case PluginMetaType::Action::MenuRequest:
        reply = m_bluetoothIface->call("showTrayWidgetUI");
        if(!reply.isValid()) {
            qWarning() << "BluetoothShortcut call " << "showTrayWidgetUI" << "failed";
        }
        break;
    default:
        break;
    }
}

void BluetoothShortcut::triggerBluetoothPower()
{
    if (!m_bluetoothIface) {
        return;
    }

    if (m_powerCallPending) {
        return;
    }
    bool targetPower = !m_defaultAdapterPower;
    m_powerCallPending = true;

    const QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if (!sessionBus.isConnected()) {
        qWarning() << "BluetoothShortcut: session bus is not connected";
        m_powerCallPending = false;
        callLegacySetBluetoothPower(targetPower);
        return;
    }

    QDBusMessage msg = QDBusMessage::createMethodCall(BLUETOOTH_SERVICE_NAME,
                                                      BLUETOOTH_SERVICE_PATH,
                                                      BLUETOOTH_SERVICE_INTERFACE,
                                                      QStringLiteral("postBluetoothPower"));
    msg << targetPower;
    const QDBusPendingCall pendingCall = sessionBus.asyncCall(msg);

    auto *watcher = new QDBusPendingCallWatcher(pendingCall, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            [this, targetPower](QDBusPendingCallWatcher *w) {
        QDBusPendingReply<> reply = *w;
        bool needFallback = false;
        if (reply.isError()) {
            const QDBusError error = reply.error();
            qWarning() << "BluetoothShortcut call" << "postBluetoothPower" << "failed" << error;
            switch (error.type()) {
            case QDBusError::ServiceUnknown:
            case QDBusError::UnknownInterface:
            case QDBusError::UnknownObject:
            case QDBusError::UnknownMethod:
                needFallback = true;
                break;
            default:
                break;
            }
        }
        m_powerCallPending = false;
        if (needFallback) {
            callLegacySetBluetoothPower(targetPower);
        }
        w->deleteLater();
    });
}

void BluetoothShortcut::callLegacySetBluetoothPower(bool power)
{
    if (!m_bluetoothIface || !m_bluetoothIface->isValid()) {
        qWarning() << "BluetoothShortcut: legacy bluetooth dbus interface invalid";
        return;
    }
    QMap<QString, QVariant> param;
    param["Powered"] = power;
    QDBusReply<bool> reply = m_bluetoothIface->call("setDefaultAdapterAttr", param);
    if (!reply.isValid()) {
        qWarning() << "BluetoothShortcut call " << "setDefaultAdapterAttr" << "Powered" << "failed";
    }
}

const StatusInfo BluetoothShortcut::currentStatus()
{
    QMutexLocker locker(&m_mutex);
    return m_statusInfo;
}

bool BluetoothShortcut::isEnable()
{
    return m_isEnable;
}

void BluetoothShortcut::setAddedState(bool added)
{
    m_isAdded = added;
    if (m_isAdded) {
        onAdapterChanged();
        initConnect();
    } else {
        endConnect();
        m_statusInfo.setColor(Color::ColorRole::BaseColor);
        m_statusInfo.setStatusName(tr("Closed"));
        Q_EMIT statusChanged(m_statusInfo);
    }
}

void BluetoothShortcut::init(QDBusInterface *interface)
{
    QMutexLocker locker(&m_mutex);
    m_bluetoothIface = interface;
    if (m_bluetoothIface) {
        m_bluetoothIface->setParent(this);
    }

    m_statusInfo.setIcon(QFile::exists(THEME_LOTTIE_PATH "/" BLUETOOTH_LOTTIE) ? BLUETOOTH_LOTTIE : BLUETOOTH_ACTIVE_SYMBOLIC);

    onAdapterChanged();
    initConnect();
}

void BluetoothShortcut::connectionFailed()
{
    m_isEnable = false;
}

void BluetoothShortcut::onAdapterChanged()
{
    if(!m_bluetoothIface) {
        return;
    }

    //是否有可用的适配器
    QDBusReply<QStringList> replylist = m_bluetoothIface->call("getAllAdapterAddress");
    QStringList adapterList;
    if (replylist.isValid()) {
        adapterList = replylist.value();
        if (!adapterList.isEmpty()) {
            m_isEnable = true;
        } else {
            m_isEnable = false;
        }
    }

    //适配器是否有开启的
    for (const auto &id : adapterList) {
        QDBusReply<QVariantMap> reply = m_bluetoothIface->call("getAdapterAttr", id, "Powered");
        if (reply.isValid()) {
            QVariantMap result = reply.value();
            if (result.value("Powered").toBool()) {
                m_statusInfo.setColor(Color::ColorRole::HighLight);
                m_defaultAdapterPower = true;
                onDeviceChanged();
                break;
            }
        }
        m_statusInfo.setColor(Color::ColorRole::BaseColor);
        m_statusInfo.setStatusName(tr("Closed"));
        m_defaultAdapterPower = false;
    }
    Q_EMIT enableStatusChanged(m_isEnable);
    Q_EMIT statusChanged(m_statusInfo);
}

void BluetoothShortcut::onDeviceChanged()
{
    if(!m_bluetoothIface) {
        return;
    }

    //遍历已经配对的设备，查找正处于连接状态的设备
    QDBusReply<QStringList> replylist = m_bluetoothIface->call("getDefaultAdapterPairedDev");
    if (replylist.isValid()) {
        QStringList addressList = replylist.value();
        QStringList connectedDevices;
        for(const QString &address : addressList) {
            QDBusReply<QVariantMap> reply = m_bluetoothIface->call("getDevAttr", address);
            if (reply.isValid()) {
                QVariantMap result = reply.value();
                if (result.value("Connected").toBool()) {
                    connectedDevices.append(result.value("Name").toString());
                }
            }
        }
        if(connectedDevices.isEmpty()) {
            if (m_defaultAdapterPower) {
                m_statusInfo.setStatusName(tr("No Connection"));
            } else {
                m_statusInfo.setStatusName(tr("Closed"));
            }
        } else {
            m_statusInfo.setStatusName(connectedDevices.join(","));
        }
    }
    Q_EMIT statusChanged(m_statusInfo);
}

QMap<PluginMetaType::SystemMode, PluginMetaData> BluetoothShortcut::pluginMetaData()
{
    return m_metaData;
}

void BluetoothShortcut::initMetaData()
{
    PluginMetaData pc {true, 1, PluginMetaType::PluginType::MenuButton};
    PluginMetaData tablet {true, 1, PluginMetaType::PluginType::MenuButton};
    m_metaData.insert(PluginMetaType::SystemMode::PC, pc);
    m_metaData.insert(PluginMetaType::SystemMode::Tablet, tablet);
}

void BluetoothShortcut::initConnect()
{
    connectSignal("adapterAddSignal", SLOT(onAdapterChanged()));
    connectSignal("adapterAttrChanged", SLOT(onAdapterChanged()));
    connectSignal("adapterRemoveSignal", SLOT(onAdapterChanged()));
    connectSignal("deviceAddSignal", SLOT(onDeviceChanged()));
    connectSignal("deviceAttrChanged", SLOT(onDeviceChanged()));
    connectSignal("deviceRemoveSignal", SLOT(onDeviceChanged()));
}

void BluetoothShortcut::endConnect()
{
    disconnectSignal("adapterAddSignal", SLOT(onAdapterChanged()));
    disconnectSignal("adapterAttrChanged", SLOT(onAdapterChanged()));
    disconnectSignal("adapterRemoveSignal", SLOT(onAdapterChanged()));
    disconnectSignal("deviceAddSignal", SLOT(onDeviceChanged()));
    disconnectSignal("deviceAttrChanged", SLOT(onDeviceChanged()));
    disconnectSignal("deviceRemoveSignal", SLOT(onDeviceChanged()));
}

void BluetoothShortcut::connectSignal(const QString &signal, const char *slot)
{
    if(!QDBusConnection::systemBus().connect(BLUETOOTH_SERVICE_NAME,
                                             BLUETOOTH_SERVICE_PATH,
                                             BLUETOOTH_SERVICE_INTERFACE,
                                             signal,
                                             this,
                                             slot)) {
        qWarning() << "BluetoothShortcut connect " << signal << "failed";
    }
}

void BluetoothShortcut::disconnectSignal(const QString &signal, const char *slot)
{

    if(!QDBusConnection::systemBus().disconnect(BLUETOOTH_SERVICE_NAME,
                                                BLUETOOTH_SERVICE_PATH,
                                                BLUETOOTH_SERVICE_INTERFACE,
                                                signal,
                                                this,
                                                slot)) {
        qWarning() << "BluetoothShortcut disconnect " << signal << "failed";
    }
}

BluetoothShortcutPlugin::BluetoothShortcutPlugin(QObject *parent) : UkuiShortcutPlugin(parent)
{

}

QStringList BluetoothShortcutPlugin::translations()
{
    return {QStringLiteral("bluetooth-shortcut")};
}

Shortcut *BluetoothShortcutPlugin::createShortcut()
{
    return new BluetoothShortcut;
}
