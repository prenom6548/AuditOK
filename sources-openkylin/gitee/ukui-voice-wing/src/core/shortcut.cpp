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

#include <QDateTime>
#include <QTimer>
#include <KF5/KGlobalAccel/KGlobalAccel>
#include <QDBusConnectionInterface>
#include "configmonitor.h"
#include "logger.h"

#define DBUS_NAME           "org.kde.kglobalaccel"
#define DBUS_PATH           "/component/ukui_voice_wing"
#define DBUS_INTERFACE      "org.kde.kglobalaccel.Component"

constexpr char kComponentName[] = "ukui-voice-wing";
constexpr int kShortcutRetryIntervalMs = 200;
constexpr int kShortcutMaxAttempts = 5;

static const QKeySequence keySeqFromString(QString keys)
{
    keys.replace("<", "");
    keys.replace(">", "+");
    keys.replace("Win", "Meta", Qt::CaseInsensitive);
    keys.replace("Super", "Meta", Qt::CaseInsensitive);
    keys.replace("Start", "Meta", Qt::CaseInsensitive);
    keys.replace("PrtSc", "Print", Qt::CaseInsensitive);

    return QKeySequence(keys, QKeySequence::PortableText);
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
    // 键值变化：先卸载旧注册，再按新键值重新注册
    VW_LOG(Logger::Level::Info) << "Shortcut key changed, re-registering";
    unregisterShortcut();
    registerShortcut();
}

void Shortcut::registerShortcut()
{
    if (m_action != nullptr) {
        return;
    }

    // 尝试注册快捷键,如果失败则重试,最多重试5次,每次间隔200ms
    tryRegisterShortcut(0);
}

void Shortcut::tryRegisterShortcut(int attempt)
{
    if (m_action != nullptr || !m_config->enabled()) {
        return;
    }

    const QKeySequence keySeq = keySeqFromString(m_config->shortcut());
    if (keySeq.isEmpty()) {
        VW_LOG(Logger::Level::Error) << "Shortcut sequence is empty, cancel registration:"
                                     << m_config->shortcut();
        return;
    }

    const QString component = QString::fromLatin1(kComponentName);
    const QString keyText = keySeq.toString(QKeySequence::NativeText);

    // 传入自身 component，避免本组件历史注册被误判为占用
    if (KGlobalAccel::isGlobalShortcutAvailable(keySeq, component)) {
        doRegisterShortcut(keySeq);
        return;
    }

    // 不可用时查询占用者，判断是否被其他组件占用
    auto occupants = KGlobalAccel::globalShortcutsByKey(keySeq);
    if (occupants.isEmpty()) {
        occupants = KGlobalAccel::getGlobalShortcutsByKey(keySeq);
    }

    bool blockedByOthers = false;
    for (const auto &info : occupants) {
        VW_LOG(Logger::Level::Warning)
            << "Shortcut" << keyText
            << "occupied by" << info.uniqueName()
            << "component" << info.componentUniqueName();
        if (info.componentUniqueName() != component) {
            blockedByOthers = true;
        }
    }

    // 无占用者（接口误报）或仅本组件占用 → 直接注册
    if (!blockedByOthers) {
        VW_LOG(Logger::Level::Info) << "Shortcut" << keyText
                                    << "not blocked by others, continue registration";
        doRegisterShortcut(keySeq);
        return;
    }

    if (attempt + 1 >= kShortcutMaxAttempts) {
        VW_LOG(Logger::Level::Error) << "Global shortcut" << keyText
                                     << "unavailable after" << kShortcutMaxAttempts
                                     << "attempts, cancel registration";
        return;
    }

    VW_LOG(Logger::Level::Info) << "Global shortcut" << keyText
                                << "unavailable, retry" << (attempt + 1)
                                << "/" << kShortcutMaxAttempts
                                << "in" << kShortcutRetryIntervalMs << "ms";

    if (!m_retryTimer) {
        m_retryTimer = new QTimer(this);
        m_retryTimer->setSingleShot(true);
    }
    m_retryTimer->disconnect();
    connect(m_retryTimer, &QTimer::timeout, this, [this, attempt]() {
        tryRegisterShortcut(attempt + 1);
    });
    m_retryTimer->start(kShortcutRetryIntervalMs);
}

void Shortcut::cancelPendingRetry()
{
    if (m_retryTimer && m_retryTimer->isActive()) {
        m_retryTimer->stop();
    }
}

void Shortcut::doRegisterShortcut(const QKeySequence &keySeq)
{
    if (m_action != nullptr) {
        return;
    }

    m_action = new QAction(this);
    m_action->setObjectName(QString::fromLatin1(kComponentName));
    m_action->setText(QString::fromLatin1(kComponentName));
    m_action->setProperty("componentName", QString::fromLatin1(kComponentName));

    // NoAutoloading：键值变化时必须写入新快捷键，避免 Autoloading 恢复旧值
    const QList<QKeySequence> shortcuts{keySeq};
    KGlobalAccel::self()->setShortcut(m_action, shortcuts, KGlobalAccel::NoAutoloading);
    KGlobalAccel::self()->setDefaultShortcut(m_action, shortcuts, KGlobalAccel::NoAutoloading);
    connectDbus();
    VW_LOG(Logger::Level::Debug) << "Global shortcut registered:"
                                 << keySeq.toString(QKeySequence::NativeText);
}

void Shortcut::unregisterShortcut()
{
    // 取消进行中的延时重试
    cancelPendingRetry();

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
