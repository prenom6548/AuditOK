/*
 * Copyright 2021 KylinSoft Co., Ltd.
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "lowpowerwatcher.h"

LowPowerWatcher::LowPowerWatcher() {}

LowPowerWatcher::~LowPowerWatcher()
{
    delete mContrlTimer;
}

void LowPowerWatcher::initLowPowerWatcher()
{
    mContrlTimer = new QTimer;
    initConnect();
    mPercentageLow = readPercentage();
    if (mSetPercentageLow >= mPercentageLow) {
        mLowPowerState = true;
    } else {
        mLowPowerState = false;
    }
    mNotifyState = false;
}

void LowPowerWatcher::initConnect()
{
    readSettings();
    connect(m_powerManagerGsettings, &QGSettings::changed, this, [=](const QString &key) {
        readSettings();
        if (key == PERCENTAGE_ACTION || key == ACTION_CRITICAL_BATTERY) {
            veryLowBatteryContrlStop();
        }
    });
    QDBusConnection::sessionBus().connect(
        UKUI_UPOWER_SERVICE,
        UKUI_UPOWER_PATH,
        UKUI_UPOWER_INTERFACE,
        "PowerState",
        this,
        SLOT(dealPowerStateMssage(bool)));
    QDBusConnection::sessionBus().connect(
        UKUI_UPOWER_SERVICE,
        UKUI_UPOWER_BATTERY_PATH,
        UKUI_UPOWER_BATTERY_INTERFACE,
        "LowBatteryState",
        this,
        SLOT(dealLowBatteryMssage(bool)));
    QDBusConnection::sessionBus().connect(
        UKUI_UPOWER_SERVICE,
        UKUI_UPOWER_BATTERY_PATH,
        UKUI_UPOWER_BATTERY_INTERFACE,
        "VeryLowBatteryState",
        this,
        SLOT(dealVeryLowBatteryMssage(bool)));
    connect(mContrlTimer, &QTimer::timeout, [this] {
        veryLowBatteryContrl(mActionCriticalBattery);
        mContrlTimer->stop();
    });
}

void LowPowerWatcher::readSettings()
{
    mSetPercentageLow = m_powerManagerGsettings->get(PERCENTAGE_LOW).toInt();
    qDebug() << "mSetPercentageLow:" << mSetPercentageLow;
    mPercentageAction = m_powerManagerGsettings->get(PERCENTAGE_ACTION).toInt();
    qDebug() << "mPercentageAction:" << mPercentageAction;
    mActionCriticalBattery = m_powerManagerGsettings->get(ACTION_CRITICAL_BATTERY).toString();
    qDebug() << "mActionCriticalBattery:" << mActionCriticalBattery;
}

double LowPowerWatcher::readPercentage()
{
    double value;
    QDBusInterface iface(UPOWER_SERVICE, UPOWER_DISPLAY_PATH, FREEDESKTOP_UPOWER, QDBusConnection::systemBus());
    QDBusReply<QVariant> reply = iface.call("Get", "org.freedesktop.UPower.Device", "Percentage");
    if (reply.isValid()) {
        value = reply.value().toDouble();
    } else {
        qDebug() << "Power percentage get error!";
    }
    return value;
}

void LowPowerWatcher::dealLowBatteryMssage(bool state)
{
    mLowPowerState = state;
    qWarning()<<"LowPowerWatcher::dealLowBatteryMssage state is :"<< state;
    if (mLowPowerState) {
        lowBatteryNotify();
    }
    emit percentageLowSignal(mLowPowerState);
}

void LowPowerWatcher::dealVeryLowBatteryMssage(bool state)
{
    qInfo() << "critical battery state:" << state;
    if (true == state) {
        if (false == mNotifyState) {
            veryLowBatteryNotify(mActionCriticalBattery);
        }
    } else {
        if (m_criticalBatteryNotificationId != 0) {
            closeNotify(m_criticalBatteryNotificationId);
            m_criticalBatteryNotificationId = 0;
        }
        veryLowBatteryContrlStop();
    }
}

void LowPowerWatcher::dealPowerStateMssage(bool state)
{
    if (!state) {
        if (m_notificationId != 0) {
            closeNotify(m_notificationId);
            m_notificationId = 0;
        }
    }
}

bool LowPowerWatcher::lowPowerState()
{
    return mLowPowerState;
}

void LowPowerWatcher::lowBatteryNotify()
{
    mPercentageLow = readPercentage();
    if (mPercentageLow > mPercentageAction) {
        QString mType = tr("Low battery notification");
        m_notificationId = notifySend(mType, tr("The system enters a low battery state"),"battery-low");
        qInfo() << "low battery notification id:" << m_notificationId;
    }
}

void LowPowerWatcher::veryLowBatteryNotify(const QString &msg)
{
    mPercentageLow = readPercentage();
    QString mType = tr("Very low battery notification");
    QString sound = "battery-low";

    if (m_criticalBatteryNotificationId == 0) {
        if ("blank" == msg) {
            m_criticalBatteryNotificationId = notifySend(mType,
                QString(tr("Current power is：%1%. The system will turn off the display in one minute")).arg(mPercentageLow),sound);
        } else if ("suspend" == msg) {
            m_criticalBatteryNotificationId = notifySend(mType,
                QString(tr("Current power is：%1%，The system will suspend in one minute")).arg(mPercentageLow),sound);
        } else if ("shutdown" == msg) {
            m_criticalBatteryNotificationId = notifySend(mType,
                QString(tr("Current power is：%1%，The system will shutdown in one minute")).arg(mPercentageLow),sound);
        } else if ("hibernate" == msg) {
            m_criticalBatteryNotificationId = notifySend(mType,
                QString(tr("Current power is：%1%，The system will hibernate in one minute")).arg(mPercentageLow),sound);
        }
        qInfo() << "new critical battery notification id:" << m_criticalBatteryNotificationId;
    } else {
        qInfo() << "old critical battery notification id:" << m_criticalBatteryNotificationId;
    }

    mNotifyState = true;
    mContrlTimer->start(60000);
}

void LowPowerWatcher::veryLowBatteryContrl(const QString &contrl)
{
    if ("blank" == contrl) {
        QProcess process;
        //执行命令
        process.start("ukui-screensaver-command -b lid");
        //等待命令执行结束
        process.waitForFinished();
        turnOffDisplay();
    } else if ("suspend" == contrl) {
        suspend();
    } else if ("shutdown" == contrl) {
        powerOff();
    } else if ("hibernate" == contrl) {
        hibernate();
    }
    mNotifyState = false;
}

void LowPowerWatcher::veryLowBatteryContrlStop()
{
    if (mContrlTimer->isActive()) {
        mContrlTimer->stop();
        mNotifyState = false;
    }
}

quint32 LowPowerWatcher::notifySend(const QString &type, const QString &arg, QString soundName)
{
    QDBusInterface iface(
        "org.freedesktop.Notifications",
        "/org/freedesktop/Notifications",
        "org.freedesktop.Notifications",
        QDBusConnection::sessionBus());
    QList<QVariant> args;
    QStringList argg;
    QMap<QString, QVariant> pear_map;

    pear_map.insert("sound-name", soundName);
    pear_map.insert("x-ukui-popup-timeout", -1);

    args << tr("Power Manager") << ((unsigned int)0) << QString("ukui-power-manager")
         << type //显示的是什么类型的信息//系统升级
         << arg  //显示的具体信息
         << argg << pear_map << (int)0;
    QDBusReply<quint32> reply = iface.callWithArgumentList(QDBus::AutoDetect, "Notify", args);
    if (reply.isValid()) {
        return reply.value();
    }
    return 0;
}

void LowPowerWatcher::closeNotify(quint32 notificationId)
{
    QDBusInterface iface(
        "org.freedesktop.Notifications",
        "/org/freedesktop/Notifications",
        "org.freedesktop.Notifications",
        QDBusConnection::sessionBus());
    iface.call("CloseNotification", notificationId);
    qInfo() << "close notify id:" << notificationId;
}

