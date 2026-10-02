/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
#include "media-key-cancel.h"
#include <QDBusConnection>
#include <QDBusReply>

#define KGLOBALACCEL_SCHEMA "org.kde.kglobalaccel"
#define KGLOBALACCEL_PATH   "/kglobalaccel"
#define KGLOBALACCEL_INTERFACE "org.kde.KGlobalAccel"

MediaKeyCancel::MediaKeyCancel(QObject *parent) : QObject(parent)
{
    if (!m_kglobalaccel) {
        m_kglobalaccel = new QDBusInterface(KGLOBALACCEL_SCHEMA, KGLOBALACCEL_PATH,
                                            KGLOBALACCEL_INTERFACE, QDBusConnection::sessionBus(), this);
    }
}

const QString MediaKeyCancel::getComponentPath()
{
    if (m_kglobalaccel->isValid()) {
        QDBusReply<QDBusObjectPath> replay = m_kglobalaccel->call("getComponent", m_component);
        if (replay.isValid()) {
            return replay.value().path();
        }
    }
    return QString();
}

void MediaKeyCancel::unRegisterShortcut(const QString &actionId)
{
    if (m_kglobalaccel->isValid()) {
        m_kglobalaccel->call("unregister", m_component, actionId);
    }
}

void MediaKeyCancel::unRegisterAll(const QString& component)
{
    m_component = component;
    const QStringList& actionIds = getActionIds();
    for (const QString &actionId : actionIds) {
        unRegisterShortcut(actionId);
    }
}

const QStringList MediaKeyCancel::getActionIds()
{
    const QString& componetPath = getComponentPath();
    if (!m_component.isEmpty() && !componetPath.isEmpty()) {
        QDBusInterface compoent(KGLOBALACCEL_SCHEMA, getComponentPath(), "org.kde.kglobalaccel.Component", QDBusConnection::sessionBus());
        if (compoent.isValid()) {
            QDBusReply<QStringList> actions = compoent.call("shortcutNames");
            if (actions.isValid()) {
                return actions.value();
            }
        }
    }
    return QStringList();
}
