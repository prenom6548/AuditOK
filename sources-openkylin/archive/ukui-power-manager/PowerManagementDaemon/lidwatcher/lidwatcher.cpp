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

#include "lidwatcher.h"

LidWatcher::LidWatcher() {}

LidWatcher::~LidWatcher() {}

void LidWatcher::initLidWatcher()
{
    inhibitSystemdLogin();
    QDBusConnection::systemBus().connect("org.freedesktop.login1",
                                         "/org/freedesktop/login1",
                                         "org.freedesktop.login1.Manager",
                                         "PrepareForSleep",
                                         this,
                                         SLOT(dealWakeUpSignal(bool)));
    QDBusConnection::systemBus().connect(
        UPOWER_SERVICE, UPOWER_PATH, FREEDESKTOP_UPOWER, "PropertiesChanged", this, SLOT(dealLidWatcherMssage(QDBusMessage)));
    m_displayState = true;
    m_lidState = false;
}

void LidWatcher::inhibitSystemdLogin()
{
    QDBusInterface manager(
        QStringLiteral("org.freedesktop.login1"),
        QStringLiteral("/org/freedesktop/login1"),
        QStringLiteral("org.freedesktop.login1.Manager"),
        QDBusConnection::systemBus());
    QDBusReply<QDBusUnixFileDescriptor> reply = manager.call(
        QStringLiteral("Inhibit"),
        QStringLiteral("handle-lid-switch"),
        QStringLiteral("powermanagment"),
        QStringLiteral("ukui-powermanagement blocked LidSwitch!"),
        QStringLiteral("block"));
    if (reply.isValid()) {
        m_logindLock = reply.value();
        qDebug() << "Inhibit got:" << m_logindLock.fileDescriptor();
    } else {
        qDebug() << "Error from inhibit:" << reply.error();
    }
}

void LidWatcher::dealLidWatcherMssage(QDBusMessage msg)
{
    const QDBusArgument &dbusArgs = msg.arguments().at(1).value<QDBusArgument>();
    QMap<QString,QVariant> map;
    dbusArgs >> map;

    if (map.contains("LidIsClosed")) {
        m_lidState = map.value(QString("LidIsClosed")).toBool();
        QString contrl;
        if (getBatteryState()) {
            contrl = m_powerManagerGsettings->get(BUTTON_LID_BATTERY).toString();
            qDebug() << "lid close contrl on battery state：" << contrl;
        } else {
            contrl = m_powerManagerGsettings->get(BUTTON_LID_AC).toString();
            qDebug() << "lid close contrl on ac state：" << contrl;
        }
        if (true == m_lidState) {
            if ("blank" == contrl) {
                QProcess process;
                //执行命令
                process.start("ukui-screensaver-command -b lid");
                //等待命令执行结束
                process.waitForFinished();
                turnOffDisplay();
                turnOffDisplaySignal(true);
                m_displayState = false;
            } else if ("suspend" == contrl) {
                suspend();
            } else if ("shutdown" == contrl) {
                powerOff();
            } else if ("hibernate" == contrl) {
                hibernate();
            }
        } else {
            if ("blank" == contrl && false == m_displayState) {
                turnOnDisplay();
                turnOffDisplaySignal(false);
                m_displayState = true;
            }
        }
    }
}

void LidWatcher::resetLidAction(void)
{
    QString contrl;
    QDBusInterface iface(UPOWER_SERVICE, UPOWER_PATH, FREEDESKTOP_UPOWER, QDBusConnection::systemBus());
    QDBusReply<QVariant> reply = iface.call("Get", "org.freedesktop.UPower", "LidIsClosed");
    if (reply.isValid()) {
        m_lidState = reply.value().toBool();
        qDebug() << "lid state:" << m_lidState;
        if (getBatteryState()) {
            contrl = m_powerManagerGsettings->get(BUTTON_LID_BATTERY).toString();
        } else {
            contrl = m_powerManagerGsettings->get(BUTTON_LID_AC).toString();
        }
        qDebug() << "lid close contrl：" << contrl;
        if (m_lidState) {
            if ("blank" == contrl) {
                QProcess process;
                //执行命令
                process.start("ukui-screensaver-command -b lid");
                //等待命令执行结束
                process.waitForFinished();
                turnOffDisplay();
                turnOffDisplaySignal(true);
                m_displayState = false;
            } else if ("suspend" == contrl) {
                suspend();
            } else if ("shutdown" == contrl) {
                powerOff();
            } else if ("hibernate" == contrl) {
                hibernate();
            }
        } else {
            if ("blank" == contrl && false == m_displayState) {
                turnOnDisplay();
                turnOffDisplaySignal(false);
                m_displayState = true;
            }
        }
    } else {
        qDebug() << "Failed to get lid closed event!";
    }
}

bool LidWatcher::getBatteryState()
{
    QDBusInterface iface(UPOWER_SERVICE, UPOWER_PATH, FREEDESKTOP_UPOWER, QDBusConnection::systemBus());
    QDBusReply<QVariant> reply = iface.call("Get", "org.freedesktop.UPower", "OnBattery");
    if (reply.isValid()) {
        return reply.value().toBool();
    }
    qDebug() << "The battery status obtained by closing the cover is abnormal!";
    return false;
}

void LidWatcher::dealWakeUpSignal(bool isSleepState)
{
    //在合盖后，通过其他唤醒源将机器唤醒时，保证合盖操作
    if (isSleepState) {
        qDebug() << "The computer is going to sleep ";
    } else {
        if (m_lidState) {
            qWarning() << "Sleep is awakened unexpectedly!";
            resetLidAction();
        } else {
            qDebug() << "The computer wakes up ";
        }
    }
}
