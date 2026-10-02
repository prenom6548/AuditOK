/*
 * Copyright (C) 2024 Tianjin KYLIN Information Technology Co., Ltd.
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
#include "usdblockshortcut.h"
#include <QDBusConnection>
#include <QDBusError>
#include <QDBusMetaType>
#include <QDebug>

USDBlockShortCut::USDBlockShortCut(QObject *parent) : QObject(parent)
{
    QString displayNum = QString(qgetenv("DISPLAY")).replace(":", "").replace(".", "_");
    m_strDbusPath = QString("%1%2").arg(QString("org.ukui.settingsDaemon.shortcut.ukui-polkit")).arg(displayNum);
    m_isRegistService = QDBusConnection::sessionBus().registerService(m_strDbusPath);
    if (!m_isRegistService) {
        qInfo() << "registerService " << m_strDbusPath << " failed!!";
    } else {
        m_isRegistObject = QDBusConnection::sessionBus().registerObject(
            "/org/ukui/settingsDaemon/shortcut",
            "org.ukui.settingsDaemon.shortcut",
            this,
            QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllProperties);
        if (!m_isRegistObject) {
            qInfo() << "registerObject /org/ukui/settingsDaemon/shortcut failed!!";
        }
    }
}

USDBlockShortCut::~USDBlockShortCut()
{
    if (m_isRegistObject) {
        QDBusConnection::sessionBus().unregisterObject("/org/ukui/settingsDaemon/shortcut");
        m_isRegistObject = false;
    }
    if (m_isRegistService) {
        QDBusConnection::sessionBus().unregisterService(m_strDbusPath);
        m_isRegistService = false;
    }
}

QStringList USDBlockShortCut::blockShortcuts()
{
    QStringList listShortCut = { "WINDOWSWITCH_KEY",      "WLCOM_SWITCH_WORKSPACE", "WLCOM_WINDOW_ACTION",
                                 "WLCOM_MAXIMIZED_VIEWS", "WLCOM_CTRL_VIEWS",       "KDS_KEY" };
    return listShortCut;
}
