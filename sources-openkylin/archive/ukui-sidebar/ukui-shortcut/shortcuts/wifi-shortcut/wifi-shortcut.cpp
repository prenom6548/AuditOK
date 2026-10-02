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

#include "wifi-shortcut.h"
#include "ukui-quick/platform/dbus-connector.h"

#include <QDebug>
#include <QProcess>
#include <QDBusReply>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QApplication>
#include <QTranslator>
#include <QFile>

#define THEME_LOTTIE_PATH "/usr/share/icons/ukui-motion-icons"

using namespace UkuiShortcut;
using namespace UkuiQuick;

WiFiShortcutPlugin::WiFiShortcutPlugin(QObject *parent) : UkuiShortcutPlugin(parent)
{

}

QStringList WiFiShortcutPlugin::translations()
{
    return {QStringLiteral("wifi-shortcut")};
}

Shortcut *WiFiShortcutPlugin::createShortcut()
{
    return new WiFiShortcut;
}

UkuiShortcut::WiFiShortcut::WiFiShortcut(QObject *parent) : Shortcut(parent)
{
    m_currentStatus.setName(tr("Network"));
    m_currentStatus.setIcon(QFile::exists(THEME_LOTTIE_PATH "/" WIRELESS_LOTTIE) ? WIRELESS_LOTTIE : WIRELESS_ICON);
    m_currentStatus.setToolTip(tr("Network"));
    m_currentStatus.setWidgetId("org.ukui.shortcut.network");

    initMetaData();

    DbusConnector* dbusConnector = new DbusConnector(KYLIN_NETWORK_NAME,
                                                     KYLIN_NETWORK_PATH,
                                                     KYLIN_NETWORK_INTERFACE,
                                                     QDBusConnection::sessionBus());
    connect(dbusConnector, &DbusConnector::ready, this, &WiFiShortcut::onDbusConnectorReady);
    dbusConnector->start();
}

WiFiShortcut::~WiFiShortcut()
{
    endQueryThread();
    endNetworkDbus();
}

QString WiFiShortcut::pluginId()
{
    return QLatin1String("WiFi");
}

QMap<PluginMetaType::SystemMode, PluginMetaData> WiFiShortcut::pluginMetaData()
{
    return m_metaData;
}

//事件处理
void WiFiShortcut::active(PluginMetaType::Action action)
{
    switch (action) {
    default:
    case PluginMetaType::Action::Click:
        //qDebug() << "点击事件";
        kyWifiButtonActive();
        break;
    case PluginMetaType::Action::LongClick:
        //qDebug() << "长按事件";
        break;
    case PluginMetaType::Action::MenuRequest:
        //qDebug() << "菜单事件";
        break;
    }
}

const StatusInfo WiFiShortcut::currentStatus()
{
    return m_currentStatus;
}

//判断无线网卡是否可用
bool WiFiShortcut::isEnable()
{
    return m_isEnable;
}

void WiFiShortcut::setAddedState(bool added)
{
    if (m_isAdded == added) return;
    m_isAdded = added;

    if (m_isAdded) {
        initConnect();
        Q_EMIT toGetNetworkDeviceData(m_networkInterface);
    } else {
        endConnect();
        m_currentStatus.setColor(Color::ColorRole::BaseColor);
        m_currentStatus.setStatusName(tr("Closed"));
        Q_EMIT statusChanged(m_currentStatus);
    }
}

void WiFiShortcut::initMetaData()
{
    PluginMetaData pc {true, 0, PluginMetaType::PluginType::MenuButton};
    PluginMetaData tablet {true, 0, PluginMetaType::PluginType::MenuButton};
    m_metaData.insert(PluginMetaType::SystemMode::PC, pc);
    m_metaData.insert(PluginMetaType::SystemMode::Tablet, tablet);
}

//信号连接
void WiFiShortcut::initConnect()
{
    if (!QDBusConnection::sessionBus().connect(KYLIN_NETWORK_NAME,
                                               KYLIN_NETWORK_PATH,
                                               KYLIN_NETWORK_INTERFACE,
                                               "lanActiveConnectionStateChanged", this, SLOT(networkDeviceDataChanged()))) {
        qDebug() << "WiFiShortcut: QDBusConnection::sessionBus() connect lanActiveConnectionStateChanged failed";
    }

    if (!QDBusConnection::sessionBus().connect(KYLIN_NETWORK_NAME,
                                               KYLIN_NETWORK_PATH,
                                               KYLIN_NETWORK_INTERFACE,
                                               "wlanactiveConnectionStateChanged", this, SLOT(networkDeviceDataChanged()))) {
        qDebug() << "WiFiShortcut: QDBusConnection::sessionBus() connect wlanactiveConnectionStateChanged failed";
    }

    if (!QDBusConnection::sessionBus().connect(KYLIN_NETWORK_NAME,
                                               KYLIN_NETWORK_PATH,
                                               KYLIN_NETWORK_INTERFACE,
                                               "wiredMainSwitchBtnChanged", this, SLOT(networkDeviceDataChanged()))) {
        qDebug() << "WiFiShortcut: QDBusConnection::sessionBus() connect wiredMainSwitchBtnChanged failed";
    }

    if (!QDBusConnection::sessionBus().connect(KYLIN_NETWORK_NAME,
                                               KYLIN_NETWORK_PATH,
                                               KYLIN_NETWORK_INTERFACE,
                                               "wirelessSwitchBtnChanged", this, SLOT(networkDeviceDataChanged()))) {
        qDebug() << "WiFiShortcut: QDBusConnection::sessionBus() connect wirelessSwitchBtnChanged failed";
    }
}

void WiFiShortcut::endQueryThread()
{
    if (m_wifiDbusQuery) {
        disconnect(this, &WiFiShortcut::toGetNetworkDeviceData, m_wifiDbusQuery, &WifiDbusQuery::getNetworkDeviceData);
        disconnect(m_wifiDbusQuery, &WifiDbusQuery::networkDeviceData, this, &WiFiShortcut::onNetworkDeviceDataChanged);
    }

    if (m_wifiDbusThread) {
        m_wifiDbusThread->quit();
        m_wifiDbusThread->wait();
        delete m_wifiDbusThread;
        m_wifiDbusThread = nullptr;
    }

    if (m_wifiDbusQuery) {
        delete m_wifiDbusQuery;
        m_wifiDbusQuery = nullptr;
    }
}

void WiFiShortcut::endNetworkDbus()
{
    if (m_networkInterface) {
        delete m_networkInterface;
        m_networkInterface = nullptr;
    }
}

void WiFiShortcut::endConnect()
{
    QDBusConnection::sessionBus().disconnect(KYLIN_NETWORK_NAME,
                                            KYLIN_NETWORK_PATH,
                                            KYLIN_NETWORK_INTERFACE,
                                            "lanActiveConnectionStateChanged", this, SLOT(networkDeviceDataChanged()));

    QDBusConnection::sessionBus().disconnect(KYLIN_NETWORK_NAME,
                                            KYLIN_NETWORK_PATH,
                                            KYLIN_NETWORK_INTERFACE,
                                            "wlanactiveConnectionStateChanged", this, SLOT(networkDeviceDataChanged()));

    QDBusConnection::sessionBus().disconnect(KYLIN_NETWORK_NAME,
                                            KYLIN_NETWORK_PATH,
                                            KYLIN_NETWORK_INTERFACE,
                                            "wiredMainSwitchBtnChanged", this, SLOT(networkDeviceDataChanged()));

    QDBusConnection::sessionBus().disconnect(KYLIN_NETWORK_NAME,
                                            KYLIN_NETWORK_PATH,
                                            KYLIN_NETWORK_INTERFACE,
                                            "wirelessSwitchBtnChanged", this, SLOT(networkDeviceDataChanged()));
}

void WiFiShortcut::onNetworkDeviceDataChanged(QVariantList data)
{
    if (data.length() < 6) return;

    m_isWireEnable = (data.at(0).toInt() == 1);
    m_isWireOpen = m_isAdded ? (data.at(1).toInt() == 1) : false;
    m_wireName = m_isAdded ? data.at(2).toString() : "";

    m_isWirelessEnable = (data.at(3).toInt() == 1);
    m_isWirelessOpen = m_isAdded ? (data.at(4).toInt() == 1) : false;
    m_wirelessName = m_isAdded ? data.at(5).toString() : "";

    updateStatus();
}

void WiFiShortcut::updateStatus()
{
    m_isEnable = true;
    if (m_isWirelessEnable) {
        m_currentStatus.setIcon(QFile::exists(THEME_LOTTIE_PATH "/" WIRELESS_LOTTIE) ? WIRELESS_LOTTIE : WIRELESS_ICON);
        if (m_isWirelessOpen) {
            if (m_wirelessName != "") {
                m_currentStatus.setStatusName(m_wirelessName);
            } else {
                m_currentStatus.setStatusName(tr("Not connected"));
            }
            m_currentStatus.setColor(Color::ColorRole::HighLight);

        } else {
            m_currentStatus.setStatusName(tr("Closed"));
            m_currentStatus.setColor(Color::ColorRole::BaseColor);
        }

    } else if (m_isWireEnable) {
        m_currentStatus.setIcon(QFile::exists(THEME_LOTTIE_PATH "/" WIRED_LOTTIE) ? WIRED_LOTTIE : WIRED_ICON);
        if (m_isWireOpen) {
            if (m_wireName != "") {
                m_currentStatus.setStatusName(m_wireName);
            } else {
                m_currentStatus.setStatusName(tr("Not connected"));
            }
            m_currentStatus.setColor(Color::ColorRole::HighLight);

        } else {
            m_currentStatus.setStatusName(tr("Closed"));
            m_currentStatus.setColor(Color::ColorRole::BaseColor);
        }

    } else {
        m_isEnable = false;
    }
    Q_EMIT statusChanged(m_currentStatus);
    Q_EMIT enableStatusChanged(m_isEnable);
}

void WiFiShortcut::kyWifiButtonActive()
{
    if (m_isWirelessEnable) {
        QDBusMessage message = QDBusMessage::createMethodCall(KYLIN_NETWORK_NAME,
                                                              KYLIN_NETWORK_PATH,
                                                              KYLIN_NETWORK_INTERFACE,
                                                              "setWirelessSwitchEnable");
        message << !m_isWirelessOpen;
        QDBusConnection::sessionBus().call(message);

    } else if (m_isWireEnable) {
        QDBusMessage message = QDBusMessage::createMethodCall(KYLIN_NETWORK_NAME,
                                                              KYLIN_NETWORK_PATH,
                                                              KYLIN_NETWORK_INTERFACE,
                                                              "setWiredSwitchEnable");
        message << !m_isWireOpen;
        QDBusConnection::sessionBus().call(message);
    }
}

void WiFiShortcut::onDbusConnectorReady(QDBusInterface *interface)
{
    if (!interface || !interface->isValid()) return;

    m_networkInterface = interface;
    m_wifiDbusThread = new QThread();
    m_wifiDbusQuery = new WifiDbusQuery();
    m_wifiDbusThread->start();
    m_wifiDbusQuery->moveToThread(m_wifiDbusThread);

    connect(this, &WiFiShortcut::toGetNetworkDeviceData, m_wifiDbusQuery, &WifiDbusQuery::getNetworkDeviceData);
    connect(m_wifiDbusQuery, &WifiDbusQuery::networkDeviceData, this, &WiFiShortcut::onNetworkDeviceDataChanged);

    QDBusConnection::sessionBus().connect(KYLIN_NETWORK_NAME,
                                         KYLIN_NETWORK_PATH,
                                         KYLIN_NETWORK_INTERFACE,
                                         "deviceStatusChanged", this, SLOT(networkDeviceDataChanged()));

    QDBusConnection::sessionBus().connect(KYLIN_NETWORK_NAME,
                                         KYLIN_NETWORK_PATH,
                                         KYLIN_NETWORK_INTERFACE,
                                         "wirelessDeviceStatusChanged", this, SLOT(networkDeviceDataChanged()));

    Q_EMIT toGetNetworkDeviceData(m_networkInterface);
}

//查询网络状态
void WiFiShortcut::networkDeviceDataChanged()
{
    Q_EMIT toGetNetworkDeviceData(m_networkInterface);
}
