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
#include "freedesktophelper.h"
#include <QDebug>

std::unique_ptr<FreedesktopHelper> FreedesktopHelper::m_instanceSystem;
std::unique_ptr<FreedesktopHelper> FreedesktopHelper::m_instanceSession;
std::once_flag FreedesktopHelper::m_initFlagSystem;
std::once_flag FreedesktopHelper::m_initFlagSession;

FreedesktopHelper::FreedesktopHelper(bool bSession, QObject *parent)
    : QDBusAbstractInterface(
        FD_DBUS_SERVICE,
        FD_DBUS_PATH,
        FD_DBUS_INTERFACE,
        (bSession ? QDBusConnection::sessionBus() : QDBusConnection::systemBus()),
        parent)
{
    connect(
        this,
        SIGNAL(NameOwnerChanged(QString, QString, QString)),
        this,
        SLOT(onDBusNameOwnerChanged(QString, QString, QString)));
}

FreedesktopHelper* FreedesktopHelper::getInstance(bool bSession, QObject *parent)
{
    if (bSession) {
        std::call_once(m_initFlagSession, [&,bSession, parent]() {
            m_instanceSession.reset(new FreedesktopHelper(bSession, parent));
        });
        return m_instanceSession.get();
    } else {
        std::call_once(m_initFlagSystem, [&,bSession, parent]() {
            m_instanceSystem.reset(new FreedesktopHelper(bSession, parent));
        });
        return m_instanceSystem.get();
    }
}

void FreedesktopHelper::asyncStartService(const QString &serviceName)
{
    QDBusPendingCall pendingCall = asyncCall(QStringLiteral("StartServiceByName"), serviceName,  QVariant::fromValue<uint>(0));
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pendingCall, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, [](QDBusPendingCallWatcher *watcher) {
        QDBusPendingReply<uint> reply = *watcher;
        if (reply.isError()) {
            qWarning() << "Failed to start service:" << reply.error().message();
        } else {
            qDebug() << "Service started, result code:" << reply.value();
        }
        watcher->deleteLater();
    });
}

bool FreedesktopHelper::NameHasOwner(const QString &strService)
{
    QDBusReply<bool> ret = call("NameHasOwner", strService);
    if (ret.isValid() && ret.value()) {
        return true;
    }
    return false;
}

bool FreedesktopHelper::isServiceActivable(const QString &strService)
{
    bool isActivable = false;
    QDBusReply<QStringList> result = call(QStringLiteral("ListActivatableNames"));
    if (!result.isValid()) {
        qWarning() << "ListActivatableNames error:" << result.error().message();
    } else {
        QStringList listNames = result.value();
        if (listNames.contains(strService)) {
            isActivable = true;
        }
    }
    return isActivable;
}

void FreedesktopHelper::onDBusNameOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner)
{
    Q_UNUSED(oldOwner);
    Q_EMIT serviceStatusChanged(name, !newOwner.isEmpty());
}
