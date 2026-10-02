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
#include "systemd1serviceinterface.h"

#include <QVariant>
#include <QDBusInterface>

Systemd1ServiceInterface::Systemd1ServiceInterface(const QString &service,
                                                   const QString &path,
                                                   const QDBusConnection &connection,
                                                   QObject *parent)
    : QObject(parent)
{
    qDebug() << "Creating Systemd1ServiceInterface for service:" << service << "path:" << path;

    if (service.isEmpty() || path.isEmpty() || !connection.isConnected()) {
        qWarning() << "Invalid parameters for Systemd1ServiceInterface constructor"
                   << "service:" << service << "path:" << path
                   << "connection connected:" << connection.isConnected();
        m_dbusPropIFService = nullptr;
        return;
    }

    m_dbusPropIFService = new DBusPropertiesInterface(service, path, connection, this);
    if (!m_dbusPropIFService) {
        qWarning() << "Failed to create DBusPropertiesInterface";
    }
}

Systemd1ServiceInterface::~Systemd1ServiceInterface()
{
    if (m_dbusPropIFService != nullptr) {
        delete m_dbusPropIFService;
        m_dbusPropIFService = nullptr;
    }
}

// 获取服务主进程id
QPair<KError, quint32> Systemd1ServiceInterface::getMainPID() const
{
    KError ke;
    quint32 pid = 0;

    if (!m_dbusPropIFService) {
        ke.setCode(KError::kErrorCommon);
        ke.setErrorMessage("DBusPropertiesInterface not initialized");
        return {ke, pid};
    }

    // 获取MainID属性
    auto reply = m_dbusPropIFService->Get(SYSTEMD1_SERVICE_NAME, "MainPID");
    ke = reply.first;
    if (ke)
        return {ke, pid};
    QDBusMessage msg = reply.second;

    if (msg.type() == QDBusMessage::ErrorMessage) {
        return {ke, pid};
    } else {
        Q_ASSERT(msg.type() == QDBusMessage::ReplyMessage);
        QDBusVariant v = qvariant_cast<QDBusVariant>(msg.arguments()[0]);
        if (v.variant().type() == QVariant::UInt) {
            pid = qvariant_cast<quint32>(v.variant());
        }
    }

    return {ke, pid};
}

// 获取服务的id
QPair<KError, QString> Systemd1ServiceInterface::getId() const
{
    KError ke;
    QString id = "";

    if (!m_dbusPropIFService) {
        ke.setCode(KError::kErrorCommon);
        ke.setErrorMessage("DBusPropertiesInterface not initialized");
        return {ke, id};
    }

    // 获取Id属性
    auto reply = m_dbusPropIFService->Get(SYSTEMD1_UNIT_NAME, "Id");
    ke = reply.first;
    if (ke)
        return {ke, id};
    QDBusMessage msg = reply.second;

    if (msg.type() == QDBusMessage::ErrorMessage) {
        return {ke, id};
    } else {
        Q_ASSERT(msg.type() == QDBusMessage::ReplyMessage);
        QDBusVariant v = qvariant_cast<QDBusVariant>(msg.arguments()[0]);
        if (v.variant().type() == QVariant::String) {
            id = qvariant_cast<QString>(v.variant());
        }
    }

    return {ke, id};
}

// 获取服务的加载状态
QPair<KError, QString> Systemd1ServiceInterface::getLoadState() const
{
    KError ke;
    QString state = "";

    if (!m_dbusPropIFService) {
        ke.setCode(KError::kErrorCommon);
        ke.setErrorMessage("DBusPropertiesInterface not initialized");
        return {ke, state};
    }

    // 获取LoadState属性
    auto reply = m_dbusPropIFService->Get(SYSTEMD1_UNIT_NAME, "LoadState");
    ke = reply.first;
    if (ke)
        return {ke, state};
    QDBusMessage msg = reply.second;

    if (msg.type() == QDBusMessage::ErrorMessage) {
        return {ke, state};
    } else {
        Q_ASSERT(msg.type() == QDBusMessage::ReplyMessage);
        QDBusVariant v = qvariant_cast<QDBusVariant>(msg.arguments()[0]);
        if (v.variant().type() == QVariant::String) {
            state = qvariant_cast<QString>(v.variant());
        }
    }

    return {ke, state};
}

// 获取服务的活动状态
QPair<KError, QString> Systemd1ServiceInterface::getActiveState() const
{
    KError ke;
    QString state = "";

    if (!m_dbusPropIFService) {
        ke.setCode(KError::kErrorCommon);
        ke.setErrorMessage("DBusPropertiesInterface not initialized");
        return {ke, state};
    }

    // 获取ActiveState属性
    auto reply = m_dbusPropIFService->Get(SYSTEMD1_UNIT_NAME, "ActiveState");
    ke = reply.first;
    if (ke)
        return {ke, state};
    QDBusMessage msg = reply.second;

    if (msg.type() == QDBusMessage::ErrorMessage) {
        return {ke, state};
    } else {
        Q_ASSERT(msg.type() == QDBusMessage::ReplyMessage);
        QDBusVariant v = qvariant_cast<QDBusVariant>(msg.arguments()[0]);
        if (v.variant().type() == QVariant::String) {
            state = qvariant_cast<QString>(v.variant());
        }
    }

    return {ke, state};
}

// 获取服务的子状态
QPair<KError, QString> Systemd1ServiceInterface::getSubState() const
{
    KError ke;
    QString sub = "";

    if (!m_dbusPropIFService) {
        ke.setCode(KError::kErrorCommon);
        ke.setErrorMessage("DBusPropertiesInterface not initialized");
        return {ke, sub};
    }

    // 获取SubState属性
    auto reply = m_dbusPropIFService->Get(SYSTEMD1_UNIT_NAME, "SubState");
    ke = reply.first;
    if (ke)
        return {ke, sub};
    QDBusMessage msg = reply.second;

    if (msg.type() == QDBusMessage::ErrorMessage) {
        return {ke, sub};
    } else {
        Q_ASSERT(msg.type() == QDBusMessage::ReplyMessage);
        QDBusVariant v = qvariant_cast<QDBusVariant>(msg.arguments()[0]);
        if (v.variant().type() == QVariant::String) {
            sub = qvariant_cast<QString>(v.variant());
        }
    }

    return {ke, sub};
}

// 获取服务的描述
QPair<KError, QString> Systemd1ServiceInterface::getDescription() const
{
    KError ke;
    QString desc = "";

    if (!m_dbusPropIFService) {
        ke.setCode(KError::kErrorCommon);
        ke.setErrorMessage("DBusPropertiesInterface not initialized");
        return {ke, desc};
    }

    // 获取Description属性
    auto reply = m_dbusPropIFService->Get(SYSTEMD1_UNIT_NAME, "Description");
    ke = reply.first;
    if (ke)
        return {ke, desc};
    QDBusMessage msg = reply.second;

    if (msg.type() == QDBusMessage::ErrorMessage) {
        return {ke, desc};
    } else {
        Q_ASSERT(msg.type() == QDBusMessage::ReplyMessage);
        QDBusVariant v = qvariant_cast<QDBusVariant>(msg.arguments()[0]);
        if (v.variant().type() == QVariant::String) {
            desc = qvariant_cast<QString>(v.variant());
        }
    }

    return {ke, desc};
}

// 获取服务是否可重加载属性
QPair<KError, bool> Systemd1ServiceInterface::canReload() const
{
    KError ke;
    bool re = false;

    if (!m_dbusPropIFService) {
        ke.setCode(KError::kErrorCommon);
        ke.setErrorMessage("DBusPropertiesInterface not initialized");
        return {ke, re};
    }

    // 获取CanReload属性
    auto reply = m_dbusPropIFService->Get(SYSTEMD1_UNIT_NAME, "CanReload");
    ke = reply.first;
    if (ke)
        return {ke, re};
    QDBusMessage msg = reply.second;

    if (msg.type() == QDBusMessage::ErrorMessage) {
        return {ke, re};
    } else {
        Q_ASSERT(msg.type() == QDBusMessage::ReplyMessage);
        QDBusVariant v = qvariant_cast<QDBusVariant>(msg.arguments()[0]);
        if (v.variant().type() == QVariant::Bool) {
            re = qvariant_cast<bool>(v.variant());
        }
    }

    return {ke, re};
}

// 获取服务是否可启动属性
QPair<KError, bool> Systemd1ServiceInterface::canStart() const
{
    KError ke;
    bool re  = false;

    if (!m_dbusPropIFService) {
        ke.setCode(KError::kErrorCommon);
        ke.setErrorMessage("DBusPropertiesInterface not initialized");
        return {ke, re};
    }

    // 获取CanStart属性
    auto reply = m_dbusPropIFService->Get(SYSTEMD1_UNIT_NAME, "CanStart");
    ke = reply.first;
    if (ke)
        return {ke, re};
    QDBusMessage msg = reply.second;

    if (msg.type() == QDBusMessage::ErrorMessage) {
        return {ke, re};
    } else {
        Q_ASSERT(msg.type() == QDBusMessage::ReplyMessage);
        QDBusVariant v = qvariant_cast<QDBusVariant>(msg.arguments()[0]);
        if (v.variant().type() == QVariant::Bool) {
            re = qvariant_cast<bool>(v.variant());
        }
    }

    return {ke, re};
}

// 获取服务是否可停止属性
QPair<KError, bool> Systemd1ServiceInterface::canStop() const
{
    KError ke;
    bool re = false;

    if (!m_dbusPropIFService) {
        ke.setCode(KError::kErrorCommon);
        ke.setErrorMessage("DBusPropertiesInterface not initialized");
        return {ke, re};
    }

    // 获取CanStop属性
    auto reply = m_dbusPropIFService->Get(SYSTEMD1_UNIT_NAME, "CanStop");
    ke = reply.first;
    if (ke)
        return {ke, re};
    QDBusMessage msg = reply.second;

    if (msg.type() == QDBusMessage::ErrorMessage) {
        return {ke, re};
    } else {
        Q_ASSERT(msg.type() == QDBusMessage::ReplyMessage);
        QDBusVariant v = qvariant_cast<QDBusVariant>(msg.arguments()[0]);
        if (v.variant().type() == QVariant::Bool) {
            re = qvariant_cast<bool>(v.variant());
        }
    }

    return {ke, re};
}
