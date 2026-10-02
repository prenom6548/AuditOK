/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "kglobalacceldbusproxy.h"
#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusPendingCallWatcher>
#include <QDBusReply>
#include <QDebug>

KGlobalAccelDbusProxy::KGlobalAccelDbusProxy(QObject *parent)
    : QObject(parent) {
    // 初始化KGlobal的DBus接口
    m_kglobalAccelInterface = std::make_unique<QDBusInterface>(
        m_serviceName, m_kglobalAccelServicePath,
        m_kglobalAccelServiceInterface, QDBusConnection::sessionBus(), this);

    // 初始化系统快捷键DBus接口
    m_systemInterface = std::make_unique<QDBusInterface>(
        m_serviceName, m_systemServicePath, m_systemServiceInterface,
        QDBusConnection::sessionBus(), this);

    // 初始化窗口快捷键DBus接口
    m_windowInterface = std::make_unique<QDBusInterface>(
        m_serviceName, m_windowServicePath, m_windowServiceInterface,
        QDBusConnection::sessionBus(), this);

    if (!m_kglobalAccelInterface->isValid()) {
        qWarning() << "System interface is not valid:"
                   << m_kglobalAccelInterface->lastError().message();
    }

    if (!m_systemInterface->isValid()) {
        qWarning() << "System interface is not valid:"
                   << m_systemInterface->lastError().message();
    }

    if (!m_windowInterface->isValid()) {
        qWarning() << "Window interface is not valid:"
                   << m_windowInterface->lastError().message();
    }
}

void KGlobalAccelDbusProxy::blockGlobalShortcuts() {
    if (m_kglobalAccelInterface == nullptr ||
        !m_kglobalAccelInterface->isValid()) {
        qWarning() << "m_kglobalAccelInterface is not available";
        return;
    }
    m_kglobalAccelInterface->call(m_blockGlobalShortcutsMethod, true);
    return;
}

void KGlobalAccelDbusProxy::unblockGlobalShortcuts() {
    if (m_kglobalAccelInterface == nullptr ||
        !m_kglobalAccelInterface->isValid()) {
        qWarning() << "m_kglobalAccelInterface is not available";
        return;
    }
    m_kglobalAccelInterface->call(m_blockGlobalShortcutsMethod, false);
    return;
}

QDBusPendingReply<KGlobalAccelShortcutInfoList>
KGlobalAccelDbusProxy::getSystemShortcuts() const {
    if (m_systemInterface == nullptr || !m_systemInterface->isValid()) {
        qWarning() << "System interface is not available";
        return QDBusPendingReply<KGlobalAccelShortcutInfoList>();
    }

    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(m_systemComponentName);

    QDBusPendingReply<KGlobalAccelShortcutInfoList> reply =
        m_systemInterface->asyncCallWithArgumentList(m_allShortcutInfosMethod,
                                                     argumentList);

    return reply;
}

QDBusPendingReply<KGlobalAccelShortcutInfoList>
KGlobalAccelDbusProxy::getWindowShortcuts() const {
    if (m_windowInterface == nullptr || !m_windowInterface->isValid()) {
        qWarning() << "m_windowInterface is not available";
        return QDBusPendingReply<KGlobalAccelShortcutInfoList>();
    }

    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(m_windowComponentName);

    QDBusPendingReply<KGlobalAccelShortcutInfoList> reply =
        m_windowInterface->asyncCallWithArgumentList(m_allShortcutInfosMethod,
                                                     argumentList);

    return reply;
}
