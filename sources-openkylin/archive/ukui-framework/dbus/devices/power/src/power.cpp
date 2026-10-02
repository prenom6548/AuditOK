/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "power.h"

#include <QDebug>
#include <QProcess>
#include <QtDBus/QtDBus>

#define UPOWER_SERVICE         "org.freedesktop.UPower"
#define UPOWER_PATH            "/org/freedesktop/UPower"
#define UPOWER_INTERFACE       "org.freedesktop.UPower"

#define UPOWER_DBUS_PROPERTIES "org.freedesktop.DBus.Properties"
#define UPOWER_DEVICE          "org.freedesktop.UPower.Device"

#define POWERMANAGER_SERVICE   "org.ukui.powerManager"
#define POWERMANAGER_PATH      "/PowerManager"
#define POWERMANAGER_INTERFACE "org.ukui.powerManager"

#define GSETTING_POWERMANAGER  "org.ukui.power-manager"
//using namespace  UKUI;
UkuiPowerInterface::UkuiPowerInterface(QObject *parent) : QObject(parent)
{
    if (QGSettings::isSchemaInstalled(GSETTING_POWERMANAGER)) {
        m_powerGSettings = new QGSettings(GSETTING_POWERMANAGER);
        // 连接 GSettings 变化信号，监听外部手动修改
        connect(m_powerGSettings, &QGSettings::changed, this, &UkuiPowerInterface::onGSettingsChanged);
    } else {
        qWarning() << "org.ukui.power-manager schema not install!";
    }

    getBatteryDbusPath();

    // 连接设备插入信号
    QDBusConnection::systemBus().connect(
        UPOWER_SERVICE,
        UPOWER_PATH,
        UPOWER_INTERFACE,
        "DeviceAdded",
        this,
        SLOT(onDeviceAdded(QString))
    );

    // 连接设备拔出信号
    QDBusConnection::systemBus().connect(
        UPOWER_SERVICE,
        UPOWER_PATH,
        UPOWER_INTERFACE,
        "DeviceRemoved",
        this,
        SLOT(onDeviceRemoved(QString))
    );
}

UkuiPowerInterface::~UkuiPowerInterface() = default;

static QVariant GetDBusProperty(const QDBusInterface& iface, const QString &propertyName)
{
    if (!iface.isValid()) {
        qDebug() << "QDBusInterface is not valid"<< iface.lastError().message();
        return QVariant();
    }

    QVariant reply = iface.property(propertyName.toUtf8().constData());
    if (!reply.isValid()) {
        qDebug() << "QDBus call failed for service";
    }

    return reply;
}

int UkuiPowerInterface::getDeviceType(const QString &dBusPath)
{
    qDebug() << "device dBus object path:" << dBusPath;

    QDBusInterface dBusInterface(UPOWER_SERVICE,
                                 dBusPath,
                                 UPOWER_DBUS_PROPERTIES,
                                 QDBusConnection::systemBus());
    QDBusReply<QVariant> reply = dBusInterface.call("Get", UPOWER_DEVICE, "Type");
    if (reply.isValid()) {
        return reply.value().toInt();
    } else {
        qDebug() << "Get device type failed";
        return UP_DEVICE_TYPE_UNKNOWN;
    }
}

QVariant UkuiPowerInterface::getBatteryProperty(const QString &propertyName)
{
    QDBusInterface iface(UPOWER_SERVICE,
                         m_batteryPath,
                         UPOWER_DEVICE,
                         QDBusConnection::systemBus());

    return GetDBusProperty(iface, propertyName);
}

void UkuiPowerInterface::getBatteryDbusPath()
{
    QDBusInterface dBusInterface(UPOWER_SERVICE,
                                 UPOWER_PATH,
                                 UPOWER_INTERFACE,
                                 QDBusConnection::systemBus());

    QDBusReply<QList<QDBusObjectPath>> reply = dBusInterface.call("EnumerateDevices");
    if (dBusInterface.isValid()) {
        for (QDBusObjectPath dBusObjectPath : reply.value()) {
            int upDeviceType = getDeviceType(dBusObjectPath.path());
            qDebug() << "device type:" << upDeviceType;
            if (UP_DEVICE_TYPE_BATTERY == upDeviceType) {
                m_batteryPath = dBusObjectPath.path();
                qDebug() << "battery dBusObjectPath:" << m_batteryPath;
            }
        }
    }
}


int UkuiPowerInterface::GetBatteryState()
{
    QVariant reply = getBatteryProperty("State");
    if (!reply.isValid()) {
        if (calledFromDBus()) {
        sendErrorReply(QDBusError::Failed, "Failed to get Battery State");        
        }
        return -1;
    }

    return reply.toInt();
}

int UkuiPowerInterface::BatteryPercentage()
{
    QVariant reply = getBatteryProperty("Percentage");
    if (!reply.isValid()) {
        if (calledFromDBus()) {
        sendErrorReply(QDBusError::Failed, "Failed to get Battery Percentage"); 
        }
        return -1;
    }

    return reply.toInt();
}

int UkuiPowerInterface::BatteryHealthLevel()
{
    QVariant reply = getBatteryProperty("Capacity");
    if (!reply.isValid()) {
        sendErrorReply(QDBusError::Failed, "Failed to get Battery Health");
        return -1;
    }

    return qRound(reply.toDouble());
}

int UkuiPowerInterface::TimeToFull()
{
    QVariant reply = getBatteryProperty("TimeToFull");
    if (!reply.isValid()) {
        if (calledFromDBus()) {
        sendErrorReply(QDBusError::Failed, "Failed to get Time To Full");
        }
        return -1;
    }

    return reply.toInt();
}

int UkuiPowerInterface::TimeToEmpty()
{
    QVariant reply = getBatteryProperty("TimeToEmpty");
    if (!reply.isValid()) {
        if (calledFromDBus()) {
        sendErrorReply(QDBusError::Failed, "Failed to get Time To Empty");
        }
        return -1;
    }

    return reply.toInt();
}

bool UkuiPowerInterface::IsBatteryPresent()
{
    QVariant reply = getBatteryProperty("IsPresent");

    return reply.isValid() && reply.toBool();
}

int UkuiPowerInterface::GetPowerMode()
{   
    QDBusInterface iface(UPOWER_SERVICE,
                         UPOWER_PATH,
                         UPOWER_INTERFACE,
                         QDBusConnection::systemBus());

    QVariant reply = GetDBusProperty(iface, "OnBattery");

    if (!reply.isValid()) {
        return -1;
    }

    if (!QGSettings::isSchemaInstalled(GSETTING_POWERMANAGER)) {
        qWarning() << "org.ukui.power-manager is not install";
        return -1;
    }

    QGSettings settings(GSETTING_POWERMANAGER);
    QVariant value;

    if (reply.toBool()) {
        value = settings.get("power-policy-battery");
        return value.isValid() ? value.toInt() : -1;
    } else {
        value = settings.get("power-policy-ac");
        return value.isValid() ? value.toInt() : -1;
    }
}

void UkuiPowerInterface::SetPowerMode(uint mode)
{
    if (mode > 2)
    {
        qWarning() << "error param " + QString::number(mode) + " just 0 or 1 or 2 can take effect!";
        return;
    }

    QDBusInterface iface(UPOWER_SERVICE,
                         UPOWER_PATH,
                         UPOWER_INTERFACE,
                         QDBusConnection::systemBus());

    QVariant reply = GetDBusProperty(iface, "OnBattery");

    if (!reply.isValid()) {
        return;
    }

    if (!QGSettings::isSchemaInstalled(GSETTING_POWERMANAGER)) {
        qWarning() << "org.ukui.power-manager is not install!";
        return;
    }

    QGSettings settings(GSETTING_POWERMANAGER);
    settings.set((reply.toBool() ? "power-policy-battery" : "power-policy-ac"), mode);
    return;
}

int UkuiPowerInterface::GetPowerSaveLevel()
{
    QDBusInterface iface(POWERMANAGER_SERVICE,
                         POWERMANAGER_PATH,
                         POWERMANAGER_INTERFACE,
                         QDBusConnection::sessionBus());

    QDBusReply<int> reply = iface.call("getPowersaveLevel");
    if (!reply.isValid()) {
        qDebug() << "Failed to get powersaveLevel";
        if (calledFromDBus()) {
            sendErrorReply(QDBusError::Failed, "Failed to get power save Level");
        }
        return -1; // 返回一个表示错误的值
    }

    return reply.value();
}

bool UkuiPowerInterface::IsLid()
{
    QDBusInterface iface(UPOWER_SERVICE,
                         UPOWER_PATH,
                         UPOWER_INTERFACE,
                         QDBusConnection::systemBus());

    QVariant reply = GetDBusProperty(iface, "LidIsPresent");

    return reply.isValid() && reply.toBool();
}

QString UkuiPowerInterface::GetPowerButtonAction()
{
    if (!m_powerGSettings) {
        qWarning() << "maybe org.ukui.power-manager is not install!";
        return QString();
    }

    return m_powerGSettings->get(KEY_POWER_BUTTON).toString();
}

int UkuiPowerInterface::SetPowerButtonAction(QString action)
{
    if (!m_powerGSettings) {
        qWarning() << "maybe org.ukui.power-manager is not install!";
        return -1;
    }

    if (!POWER_BUTTON_ACTION_VALID_LIST.contains(action)) {
        qWarning() << "SetPowerButtonAction action（" << action << "）不可用";
        return -1;
    }

    m_powerGSettings->set(KEY_POWER_BUTTON, action);
    return 0;
}

QString UkuiPowerInterface::GetLidCloseAction()
{
    if (!m_powerGSettings) {
        qWarning() << "maybe org.ukui.power-manager is not install!";
        return QString();
    }

    return m_powerGSettings->get(KEY_LID_BUTTON_DC).toString();
}

int UkuiPowerInterface::SetLidCloseAction(QString action)
{
    if (!m_powerGSettings) {
        qWarning() << "maybe org.ukui.power-manager is not install!";
        return -1;
    }

    if (!LID_CLOSE_ACTION_VALID_LIST.contains(action)) {
        qWarning() << "SetLidCloseAction action（" << action << "）不可用";
        return -1;
    }

    m_powerGSettings->set(KEY_LID_BUTTON_DC, action);
    return 0;
}

int UkuiPowerInterface::GetScreenTimeoutOnAC()
{
    if (!m_powerGSettings) {
        qWarning() << "maybe org.ukui.power-manager is not install!";
        return -1;
    }

    return m_powerGSettings->get(KEY_SCREEN_TIMEOUT_ON_AC).toInt();
}

void UkuiPowerInterface::SetScreenTimeoutOnAC(int time)
{
    if (!m_powerGSettings) {
        qWarning() << "maybe org.ukui.power-manager is not install!";
        return;
    }

    m_powerGSettings->set(KEY_SCREEN_TIMEOUT_ON_AC, time);
    return;
}

int UkuiPowerInterface::GetScreenTimeoutOnDC()
{
    if (!m_powerGSettings) {
        qWarning() << "maybe org.ukui.power-manager is not install!";
        return -1;
    }

    return m_powerGSettings->get(KEY_SCREEN_TIMEOUT_ON_DC).toInt();
}

void UkuiPowerInterface::SetScreenTimeoutOnDC(int time)
{
    if (!m_powerGSettings) {
        qWarning() << "maybe org.ukui.power-manager is not install!";
        return;
    }

    m_powerGSettings->set(KEY_SCREEN_TIMEOUT_ON_DC, time);
    return;
}

int UkuiPowerInterface::GetSleepComputerTimeOnAC()
{
    if (!m_powerGSettings) {
        qWarning() << "maybe org.ukui.power-manager is not install!";
        return -1;
    }

    return m_powerGSettings->get(KEY_SLEEP_COMPUTER_ON_AC).toInt();
}

void UkuiPowerInterface::SetSleepComputerTimeOnAC(int time)
{
    if (!m_powerGSettings) {
        qWarning() << "maybe org.ukui.power-manager is not install!";
        return;
    }

    m_powerGSettings->set(KEY_SLEEP_COMPUTER_ON_AC, time);
    return;
}

int UkuiPowerInterface::GetSleepComputerTimeOnDC()
{
    if (!m_powerGSettings) {
        qWarning() << "maybe org.ukui.power-manager is not install!";
        return -1;
    }

    return m_powerGSettings->get(KEY_SLEEP_COMPUTER_ON_DC).toInt();
}

void UkuiPowerInterface::SetSleepComputerTimeOnDC(int time)
{
    if (!m_powerGSettings) {
        qWarning() << "maybe org.ukui.power-manager is not install!";
        return;
    }

    m_powerGSettings->set(KEY_SLEEP_COMPUTER_ON_DC, time);
    return;
}

int UkuiPowerInterface::GetACPowerPolicy()
{
    if (!m_powerGSettings) {
        qWarning() << "maybe org.ukui.power-manager is not install!";
        return -1;
    }

    return m_powerGSettings->get(KEY_AC_POWER_POLICY).toInt();
}

void UkuiPowerInterface::SetACPowerPolicy(int policy)
{
    if (!m_powerGSettings) {
        qWarning() << "maybe org.ukui.power-manager is not install!";
        return;
    }

    m_powerGSettings->set(KEY_AC_POWER_POLICY, policy);
    return;
}

int UkuiPowerInterface::GetDCPowerPolicy()
{
    if (!m_powerGSettings) {
        qWarning() << "maybe org.ukui.power-manager is not install!";
        return -1;
    }

    return m_powerGSettings->get(KEY_DC_POWER_POLICY).toInt();
}

void UkuiPowerInterface::SetDCPowerPolicy(int policy)
{
    if (!m_powerGSettings) {
        qWarning() << "maybe org.ukui.power-manager is not install!";
        return;
    }

    m_powerGSettings->set(KEY_DC_POWER_POLICY, policy);
    return;
}

QString UkuiPowerInterface::GetLowBatteryAction()
{
    if (!m_powerGSettings) {
        qWarning() << "maybe org.ukui.power-manager is not install!";
        return QString();
    }

    return m_powerGSettings->get(KEY_LOW_BATTERY_ACTION).toString();
}

void UkuiPowerInterface::SetLowBatteryAction(QString action)
{
    if (!m_powerGSettings) {
        qWarning() << "maybe org.ukui.power-manager is not install!";
        return;
    }

    m_powerGSettings->set(KEY_LOW_BATTERY_ACTION, action);
    return;
}

int UkuiPowerInterface::GetLowBatteryActionThreshold()
{
    if (!m_powerGSettings) {
        qWarning() << "maybe org.ukui.power-manager is not install!";
        return -1;
    }

    return m_powerGSettings->get(KEY_LOW_BATTERY_ACTION_THRESHOLD).toInt();
}

void UkuiPowerInterface::SetLowBatteryActionThreshold(int value)
{
    if (!m_powerGSettings) {
        qWarning() << "maybe org.ukui.power-manager is not install!";
        return;
    }

    m_powerGSettings->set(KEY_LOW_BATTERY_ACTION_THRESHOLD, value);
    return;
}

int UkuiPowerInterface::GetLowBatteryNotifyThreshold()
{
    if (!m_powerGSettings) {
        qWarning() << "maybe org.ukui.power-manager is not install!";
        return -1;
    }

    return m_powerGSettings->get(KEY_LOW_BATTERY_NOTIFY_THRESHOLD).toInt();
}

void UkuiPowerInterface::SetLowBatteryNotifyThreshold(int value)
{
    if (!m_powerGSettings) {
        qWarning() << "maybe org.ukui.power-manager is not install!";
        return;
    }

    m_powerGSettings->set(KEY_LOW_BATTERY_NOTIFY_THRESHOLD, value);
    return;
}

bool UkuiPowerInterface::IsShowLeftTime()
{
    if (!m_powerGSettings) {
        qWarning() << "maybe org.ukui.power-manager is not install!";
        return false;
    }

    QVariant value = m_powerGSettings->get(KEY_DISPLAY_LEFT_TIME);
    return value.isValid() ? value.toBool() : false;  // 默认返回 false
}

void UkuiPowerInterface::SetShowLeftTime(bool enabled)
{
    if (!m_powerGSettings) {
        qWarning() << "maybe org.ukui.power-manager is not install!";
        return;
    }

    m_powerGSettings->set(KEY_DISPLAY_LEFT_TIME, enabled);
    return;
}

void UkuiPowerInterface::onDeviceAdded(const QString &devicePath)
{
    qDebug() << "Device added:" << devicePath;
    getBatteryDbusPath();
}

void UkuiPowerInterface::onDeviceRemoved(const QString &devicePath)
{
    qDebug() << "Device removed:" << devicePath;
    getBatteryDbusPath();
}

void UkuiPowerInterface::onGSettingsChanged(const QString &key)
{
    // 当 GSettings 中的值被外部修改时，触发对应的信号通知
    if (key == KEY_POWER_BUTTON) {
        emit PowerButtonActionChanged();
    } else if (key == KEY_LID_BUTTON_DC) {
        emit LidCloseActionChanged();
    } else if (key == KEY_SCREEN_TIMEOUT_ON_AC) {
        emit ScreenTimeoutOnACChanged();
    } else if (key == KEY_SCREEN_TIMEOUT_ON_DC) {
        emit ScreenTimeoutOnDCChanged();
    } else if (key == KEY_SLEEP_COMPUTER_ON_AC) {
        emit SleepComputerTimeOnACChanged();
    } else if (key == KEY_SLEEP_COMPUTER_ON_DC) {
        emit SleepComputerTimeOnDCChanged();
    } else if (key == KEY_AC_POWER_POLICY) {
        emit ACPowerPolicyChanged();
    } else if (key == KEY_DC_POWER_POLICY) {
        emit DCPowerPolicyChanged();
    } else if (key == KEY_LOW_BATTERY_ACTION) {
        emit LowBatteryActionChanged();
    } else if (key == KEY_LOW_BATTERY_ACTION_THRESHOLD) {
        emit LowBatteryActionThresholdChanged();
    } else if (key == KEY_LOW_BATTERY_NOTIFY_THRESHOLD) {
        emit LowBatteryNotifyThresholdChanged();
    } else if (key == KEY_DISPLAY_LEFT_TIME) {
        emit ShowLeftTimeChanged();
    }
}
