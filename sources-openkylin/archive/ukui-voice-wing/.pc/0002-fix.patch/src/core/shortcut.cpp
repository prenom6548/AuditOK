/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2026 KylinSoft Co., Ltd.
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

#include "shortcut.h"

#include <QDebug>
#include <QDateTime>
#include <KF5/KGlobalAccel/KGlobalAccel>
#include <QDBusConnectionInterface>
#include "configmonitor.h"

#define DBUS_NAME           "org.kde.kglobalaccel"
#define DBUS_PATH           "/component/ukui_voice_wing"
#define DBUS_INTERFACE      "org.kde.kglobalaccel.Component"

static const QKeySequence keySeqFromString(QString keys)
{
    keys.replace("<", "");
    keys.replace(">", "+");
    keys.replace("Win", "Meta", Qt::CaseInsensitive);
    keys.replace("Super", "Meta", Qt::CaseInsensitive);
    keys.replace("Start", "Meta", Qt::CaseInsensitive);
    keys.replace("PrtSc", "Print", Qt::CaseInsensitive);
    return QKeySequence::fromString(keys);
}

Shortcut *Shortcut::instance()
{
    static Shortcut shortcut;
    return &shortcut;
}

Shortcut::Shortcut(QObject *parent) : QObject(parent)
{
    initConfig();
}

void Shortcut::initConfig()
{
    // 获取 ConfigMonitor 单例
    m_config = ConfigMonitor::instance();

    if (m_config->enabled()) {
        registerShortcut();
    }

    // 订阅 ConfigMonitor 信号
    connect(m_config, &ConfigMonitor::enabledChanged, this, [this](){
        onStateChanged();
    });
    connect(m_config, &ConfigMonitor::shortcutChanged, this, [this](){
        onShortcutChanged();
    });
}

void Shortcut::onStateChanged()
{
    m_config->enabled() ? registerShortcut() : unregisterShortcut();
}

void Shortcut::onShortcutChanged()
{
    if (!m_config->enabled()) {
        return;
    }
    unregisterShortcut();
    registerShortcut();
}

void Shortcut::registerShortcut()
{
    if (m_action != nullptr) {
        return;
    }

    m_action = new QAction(this);
    m_action->setObjectName("ukui-voice-wing");
    m_action->setText("ukui-voice-wing");
    m_action->setProperty("componentName", QStringLiteral("ukui-voice-wing"));

    QKeySequence keySeq = keySeqFromString(m_config->shortcut());
    KGlobalAccel::self()->setGlobalShortcut(m_action, keySeq);
    connectDbus();
}

void Shortcut::unregisterShortcut()
{
    if (m_action == nullptr) {
        return;
    }

    disconnectDbus();
    KGlobalAccel::self()->removeAllShortcuts(m_action);
    m_action->deleteLater();
    m_action = nullptr;
}

void Shortcut::connectDbus()
{
    QDBusConnection::sessionBus().connect(DBUS_NAME, DBUS_PATH, DBUS_INTERFACE,
                                          "globalShortcutPressed", this, SLOT(onGlobalPressed()));

    QDBusConnection::sessionBus().connect(DBUS_NAME, DBUS_PATH, DBUS_INTERFACE,
                                          "globalShortcutRepeated", this, SLOT(onGlobalRepeated()));

    QDBusConnection::sessionBus().connect(DBUS_NAME, DBUS_PATH, DBUS_INTERFACE,
                                          "globalShortcutReleased", this, SLOT(onGlobalReleased()));
}

void Shortcut::disconnectDbus()
{
    QDBusConnection::sessionBus().disconnect(DBUS_NAME, DBUS_PATH, DBUS_INTERFACE,
                                            "globalShortcutPressed", this, SLOT(onGlobalPressed()));

    QDBusConnection::sessionBus().disconnect(DBUS_NAME, DBUS_PATH, DBUS_INTERFACE,
                                            "globalShortcutRepeated", this, SLOT(onGlobalRepeated()));

    QDBusConnection::sessionBus().disconnect(DBUS_NAME, DBUS_PATH, DBUS_INTERFACE,
                                             "globalShortcutReleased", this, SLOT(onGlobalReleased()));
}

void Shortcut::onGlobalPressed()
{
    m_longPressTriggered = false;
}

void Shortcut::onGlobalRepeated()
{
    if (!m_longPressTriggered) {
        m_longPressTriggered = true;
        Q_EMIT shortcutLongPressed();
    }
}

void Shortcut::onGlobalReleased()
{
    if (!m_longPressTriggered) {
        Q_EMIT shortcutClicked();
        return;
    }
    Q_EMIT shortcutReleased();
    m_longPressTriggered = false;
}
