/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#include "login1helper.h"
#include <QCoreApplication>
#include <QDebug>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusConnectionInterface>
#include <QDBusMessage>

const static QString login1Service = QStringLiteral("org.freedesktop.login1");
const static QString login1Path = QStringLiteral("/org/freedesktop/login1");
const static QString propertiesInterface = QStringLiteral("org.freedesktop.DBus.Properties");
const static QString login1ManagerInterface = QStringLiteral("org.freedesktop.login1.Manager");
const static QString login1SessionInterface = QStringLiteral("org.freedesktop.login1.Session");

Login1Helper::Login1Helper(QObject *parent) : QObject(parent)
{
    m_login1Interface = new QDBusInterface(login1Service, login1Path, login1ManagerInterface, QDBusConnection::systemBus(), this);
    // 监听manager的睡眠、唤醒信号
    connect(m_login1Interface, SIGNAL(PrepareForSleep(bool)), this, SLOT(onPrepareForSleep(bool)));

    QDBusReply<QDBusObjectPath> sessionPath = m_login1Interface->call("GetSession", "auto");
    if (!sessionPath.isValid()) {
        qWarning() << "Get session error:" << sessionPath.error();
    } else {
        m_strSessionPath = sessionPath.value().path();
        QDBusConnection::systemBus().connect(
            login1Service,
            m_strSessionPath,
            login1SessionInterface,
            QStringLiteral("Lock"),
            this,
            SIGNAL(requestLock()));
        QDBusConnection::systemBus().connect(
            login1Service,
            m_strSessionPath,
            login1SessionInterface,
            QStringLiteral("Unlock"),
            this,
            SIGNAL(requestUnlock()));

        // 获取会话激活状态
        QDBusInterface iface(login1Service, m_strSessionPath, propertiesInterface, QDBusConnection::systemBus());
        QDBusReply<QVariant> reply = iface.call("Get", login1SessionInterface, "Active");
        if (reply.isValid()) {
            m_isSessionActive = reply.value().toBool();
            qDebug() << "Session is active:" << m_isSessionActive;
        } else {
            qDebug() << "Failed to get session active state!";
        }

        // 监听属性变化
        QDBusConnection::systemBus().connect(
            login1Service,
            m_strSessionPath,
            propertiesInterface,
            "PropertiesChanged",
            this,
            SLOT(onSessionPropChanged(QString, QVariantMap, QStringList)));

        // 监听程序阻塞关机/睡眠
        QDBusConnection::systemBus().connect(
            login1Service,
            login1Path,
            propertiesInterface,
            "PropertiesChanged",
            this,
            SLOT(onHibitedWatcherMessage(void)));
    }

    m_isCanHibernate = getCanPowerManager("CanHibernate");
    m_isCanSuspend = getCanPowerManager("CanSuspend");
    m_isCanReboot = getCanPowerManager("CanReboot");
    m_isCanPowerOff = getCanPowerManager("CanPowerOff");
}

Login1Helper::~Login1Helper() {}

void Login1Helper::onSessionPropChanged(QString strInterface, QVariantMap mapVar, QStringList listValue)
{
    if (login1SessionInterface == strInterface) {
        qDebug() << "onSessionPropChanged:" << strInterface << mapVar << listValue;
        QVariantMap::iterator itVar = mapVar.begin();
        for (; itVar != mapVar.end(); itVar++) {
            if (itVar.key() == "Active") {
                QVariant varValue = itVar.value();
                m_isSessionActive = varValue.toBool();
                if (!m_isSleeping) {
                    if (!m_isSessionActive) {
                        Q_EMIT requestLock();
                    }
                    Q_EMIT sessionActiveChanged(m_isSessionActive);
                }
                break;
            }
        }
    }
}

bool Login1Helper::isSessionActive()
{
    return m_isSessionActive;
}

void Login1Helper::onPrepareForSleep(bool isSleep)
{
    m_isSleeping = isSleep;
    Q_EMIT PrepareForSleep(isSleep);
}

void Login1Helper::onHibitedWatcherMessage()
{
    QDBusInterface iface(login1Service, login1Path, propertiesInterface, QDBusConnection::systemBus());
    QDBusReply<QVariant> reply = iface.call("Get", "org.freedesktop.login1.Manager", "BlockInhibited");
    if (reply.isValid()) {
        QString blockInhibited = reply.value().toString();
        Q_EMIT blockInhibitedChanged(blockInhibited);
    } else {
        qDebug() << "Failed to get lid closed event!";
    }
}

void Login1Helper::setPowerManager(const QString &powerManagerfunc)
{
    if (m_login1Interface) {
        m_login1Interface->call(powerManagerfunc, true);
    }
}

bool Login1Helper::getCanPowerManager(const QString &powerManagerfunc)
{
    qDebug() << __LINE__ << __FUNCTION__ << "===========";
    if (m_login1Interface) {
        bool canFlag = true;
        QDBusReply<QString> stateReplySuspend = m_login1Interface->call(powerManagerfunc);
        if (stateReplySuspend.isValid() && stateReplySuspend.value() == "yes") {
            canFlag = true;
        } else {
            canFlag = false;
        }
        return canFlag;
    } else {
        return false;
    }
}

bool Login1Helper::isCanHibernate()
{
    return m_isCanHibernate;
}

bool Login1Helper::isCanSuspend()
{
    return m_isCanSuspend;
}

bool Login1Helper::isCanReboot()
{
    return m_isCanReboot;
}

bool Login1Helper::isCanPowerOff()
{
    return m_isCanPowerOff;
}

bool Login1Helper::canAction(Action action)
{
    switch (action) {
        case PowerHibernate:
            return m_isCanHibernate;
        case PowerSuspend:
            return m_isCanSuspend;
        case PowerMonitorOff:
            return m_isCanLockScreen;
        case PowerLogout:
            return m_isCanLogout;
        case PowerReboot:
            return m_isCanReboot;
        case PowerShutdown:
            return m_isCanPowerOff;
        default:
            return false;
    }
}

void Login1Helper::Unlock()
{
    if (!m_strSessionPath.isEmpty()) {
        QDBusInterface iface(login1Service, m_strSessionPath, login1SessionInterface, QDBusConnection::systemBus());
        iface.call("Unlock");
        qDebug() << "login1 unlock session:" << m_strSessionPath;
    }
}
