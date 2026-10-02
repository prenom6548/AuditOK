/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2025 KylinSoft Co., Ltd.
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

#include <QVariant>
#include <QDebug>
#include "ukui-wayland-settings-manager.h"
#include "clib-syslog.h"
/*
 * 目前优先同步光标主题与大小。
*/
ukuiWaylandSettingsManager::ukuiWaylandSettingsManager(QObject *parent) :
    AbstractManager(parent)
{
    m_gnomeSettings = new QGSettings("org.gnome.desktop.interface");
    m_mouseSettings = new QGSettings("org.ukui.peripherals-mouse");

    if (false == m_mouseSettings->isOK() || false == m_gnomeSettings->isOK()) {
        return;
    }

    for (QString &key: m_syncMouseSettings) {
        m_gnomeSettings->set(key,m_mouseSettings->get(key));
    }
}

bool ukuiWaylandSettingsManager::start()
{
    if (m_mouseSettings->isOK()) {
        QObject::connect(m_mouseSettings, SIGNAL(changed(QString)), this, SLOT(doGSettingsChanged(QString)));
    }
    sendSessionDbus();
    return true;
}

void ukuiWaylandSettingsManager::stop()
{
    if (m_gnomeSettings) {
        m_gnomeSettings->deleteLater();
        m_gnomeSettings = nullptr;
    }

    if (m_mouseSettings) {
        m_mouseSettings->deleteLater();
        m_mouseSettings = nullptr;
    }
}

//光标大小需要结合缩放吗？？需要测试。
void ukuiWaylandSettingsManager::doGSettingsChanged(const QString &key)
{
    if (m_gnomeSettings == nullptr) {
        return;
    }

    if (m_gnomeSettings->keys().contains(key)) {
        USD_LOG(LOG_DEBUG,"sync %s %s",key.toLatin1().data(),  m_mouseSettings->get(key).toString().toLatin1().data());
        m_gnomeSettings->set(key, m_mouseSettings->get(key));
    }
}
