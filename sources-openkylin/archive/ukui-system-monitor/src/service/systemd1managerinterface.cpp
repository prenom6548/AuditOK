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
#include "systemd1managerinterface.h"

Systemd1ManagerInterface::Systemd1ManagerInterface(const QString &service,
                                                   const QString &path,
                                                   const QDBusConnection &connection,
                                                   QObject *parent)
    : QDBusAbstractInterface(service, path, SYSTEMD1_MANAGER_NAME, connection, parent)
{
    setTimeout(10000);
}

Systemd1ManagerInterface::~Systemd1ManagerInterface()
{}

// 获取服务文件列表
QPair<KError, ServiceFileInfoList> Systemd1ManagerInterface::ListServiceFiles()
{
    // 已安装的服务文件列表
    ServiceFileInfoList list;
    QList<QVariant> args;
    KError ke;

    // 调用dbus接口方法：ListUnitFiles
    QDBusMessage reply = callWithArgumentList(QDBus::BlockWithGui, "ListUnitFiles", args);

    if (reply.type() == QDBusMessage::ErrorMessage) {
        ke.setCode(KError::kErrorDBus);
        ke.setSubCode(reply.type());
        ke.setErrorName(reply.errorName());
        ke.setErrorMessage(reply.errorMessage());
    } else {
        Q_ASSERT(reply.type() == QDBusMessage::ReplyMessage);
        if (reply.signature() == "a(ss)") {
            qvariant_cast<QDBusArgument>(reply.arguments()[0]) >> list;
        }
    }

    return {ke, list};
}

// 获取服务列表
QPair<KError, ServiceInfoList> Systemd1ManagerInterface::ListServices()
{
    ServiceInfoList list;
    QList<QVariant> args;
    KError ke;

    // 调用dbus接口方法：ListUnits
    QDBusMessage reply = callWithArgumentList(QDBus::BlockWithGui, "ListUnits", args);
    if (reply.type() == QDBusMessage::ErrorMessage) {
        ke.setCode(KError::kErrorDBus);
        ke.setSubCode(reply.type());
        ke.setErrorName(reply.errorName());
        ke.setErrorMessage(reply.errorMessage());
    } else {
        Q_ASSERT(reply.type() == QDBusMessage::ReplyMessage);
        if (reply.signature() == "a(ssssssouso)") {
            qvariant_cast<QDBusArgument>(reply.arguments()[0]) >> list;
        }
    }

    return {ke, list};
}

// 获取服务dbus对象路径
QPair<KError, QDBusObjectPath> Systemd1ManagerInterface::GetService(const QString &path)
{
    KError ke {};
    QDBusObjectPath o;
    QList<QVariant> args;
    args << path;

    // 调用dbus接口方法：GetUnit
    QDBusMessage reply = callWithArgumentList(QDBus::Block, "GetUnit", args);
    if (reply.type() == QDBusMessage::ErrorMessage) {
        ke.setCode(KError::kErrorDBus);
        ke.setSubCode(reply.type());
        ke.setErrorName(reply.errorName());
        ke.setErrorMessage(reply.errorMessage());
    } else {
        Q_ASSERT(reply.type() == QDBusMessage::ReplyMessage);
        if (reply.signature() == "o") {
            o = qvariant_cast<QDBusObjectPath>(reply.arguments()[0]);
        }
    }

    return {ke, o};
}

// 启动服务
QPair<KError, QDBusObjectPath> Systemd1ManagerInterface::StartService(const QString &name, const QString &mode)
{
    KError ke;
    QDBusObjectPath o;

    // 调用dbus接口方法: StartUnit
    QDBusMessage msg = QDBusMessage::createMethodCall("org.freedesktop.systemd1",
                                                       "/org/freedesktop/systemd1",
                                                       "org.freedesktop.systemd1.Manager",
                                                       "StartUnit");
    msg.setInteractiveAuthorizationAllowed(true);
    msg << name << mode;
    QDBusMessage reply = QDBusConnection::systemBus().call(msg);
    if (reply.type() == QDBusMessage::ErrorMessage) {
        ke.setCode(KError::kErrorDBus);
        ke.setSubCode(reply.type());
        ke.setErrorName(reply.errorName());
        ke.setErrorMessage(reply.errorMessage());
    } else {
        Q_ASSERT(reply.type() == QDBusMessage::ReplyMessage);
        if (reply.signature() == "o") {
            o = qvariant_cast<QDBusObjectPath>(reply.arguments()[0]);
        }
    }

    return {ke, o};
}

// 停止服务
QPair<KError, QDBusObjectPath> Systemd1ManagerInterface::StopService(const QString &name, const QString &mode)
{
    KError ke;
    QDBusObjectPath o;

    // 调用dbus接口方法: StopUnit
    QDBusMessage msg = QDBusMessage::createMethodCall("org.freedesktop.systemd1",
                                                       "/org/freedesktop/systemd1",
                                                       "org.freedesktop.systemd1.Manager",
                                                       "StopUnit");

    msg.setInteractiveAuthorizationAllowed(true);
    msg << name << mode;
    QDBusMessage reply = QDBusConnection::systemBus().call(msg);
    if (reply.type() == QDBusMessage::ErrorMessage) {
        ke.setCode(KError::kErrorDBus);
        ke.setSubCode(reply.type());
        ke.setErrorName(reply.errorName());
        ke.setErrorMessage(reply.errorMessage());
    } else {
        Q_ASSERT(reply.type() == QDBusMessage::ReplyMessage);
        if (reply.signature() == "o") {
            o = qvariant_cast<QDBusObjectPath>(reply.arguments()[0]);
        }
    }

    return {ke, o};
}

// 停止服务
QPair<KError, QDBusObjectPath> Systemd1ManagerInterface::RestartService(const QString &name,
                                                        const QString &mode)
{
    KError ke;
    QDBusObjectPath o;

    // 调用dbus接口方法: RestartUnit
    QDBusMessage msg = QDBusMessage::createMethodCall("org.freedesktop.systemd1",
                                                       "/org/freedesktop/systemd1",
                                                       "org.freedesktop.systemd1.Manager",
                                                       "RestartUnit");

    msg.setInteractiveAuthorizationAllowed(true);
    msg << name << mode;
    QDBusMessage reply = QDBusConnection::systemBus().call(msg);
    if (reply.type() == QDBusMessage::ErrorMessage) {
        ke.setCode(KError::kErrorDBus);
        ke.setSubCode(reply.type());
        ke.setErrorName(reply.errorName());
        ke.setErrorMessage(reply.errorMessage());
    } else {
        Q_ASSERT(reply.type() == QDBusMessage::ReplyMessage);
        if (reply.signature() == "o") {
            o = qvariant_cast<QDBusObjectPath>(reply.arguments()[0]);
        }
    }

    return {ke, o};
}

// 获取服务文件状态
QPair<KError, QString> Systemd1ManagerInterface::GetServiceFileState(const QString &unit)
{
    KError ke;
    QString state;
    QList<QVariant> args;
    args.clear();

    args << unit;

    // 调用dbus接口方法: GetUnitFileState
    QDBusMessage reply = callWithArgumentList(QDBus::Block, "GetUnitFileState", args);
    if (reply.type() == QDBusMessage::ErrorMessage) {
        ke.setCode(KError::kErrorDBus);
        ke.setSubCode(reply.type());
        ke.setErrorName(reply.errorName());
        ke.setErrorMessage(reply.errorMessage());
    } else {
        Q_ASSERT(reply.type() == QDBusMessage::ReplyMessage);
        if (reply.signature() == "s") {
            state = qvariant_cast<QString>(reply.arguments()[0]);
        }
    }

    return {ke, state};
}
