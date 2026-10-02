/*
 * Peony-Qt Desktop
 *
 * Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include "desktop-icon-a11y.h"

#include <QAccessible>
#include <QAccessibleEvent>
#include <QDBusInterface>
#include <QQuickItem>
#include <QTimer>

DesktopIconA11y *DesktopIconA11y::instance()
{
    static DesktopIconA11y s_instance;
    return &s_instance;
}

DesktopIconA11y::DesktopIconA11y(QObject *parent)
    : QObject(parent)
{
}

void DesktopIconA11y::requestDesktopFocus()
{
    // 复用 QML 入口聚焦逻辑：恢复上次聚焦图标，无记录则选中第一个图标。
    Q_EMIT windowActivated();
}

void DesktopIconA11y::focusPanel(bool focusFirst)
{
    // 桌面图标聚焦状态下按 Tab/Shift+Tab：把焦点交给任务栏。
    // 任务栏接口：org.ukui.panel /panel，方法 activeFocus(bool)，true 首端、false 末端。
    QDBusInterface panelIface(QStringLiteral("org.ukui.panel"),
                              QStringLiteral("/panel"),
                              QStringLiteral("org.ukui.panel"),
                              QDBusConnection::sessionBus());
    if (panelIface.isValid()) {
        panelIface.call(QStringLiteral("activeFocus"), focusFirst);
    }
}

void DesktopIconA11y::notifyIconState(QQuickItem *item, bool selected, bool focused)
{
    if (!item) {
        return;
    }

    // Defer until QML has applied Accessible.selected / focus bindings.
    QTimer::singleShot(0, this, [this, item, selected, focused]() {
        postStateChange(item, selected, focused);
    });
}

void DesktopIconA11y::postStateChange(QQuickItem *item, bool selected, bool focused)
{
    if (!item) {
        return;
    }

    QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(item);
    if (!iface) {
        return;
    }

    // 只声明 selected 状态变化：QML 侧 Accessible.selected 绑定到 selected 属性，
    // 朗读器 re-query 能拿到真值（选中/取消选中都成立）。
    // 不声明 focused：委托侧 Accessible.focus 未绑定（避免与 contentArea 抢焦点），re-query 永远 false，
    // 声明它变化会误导朗读器；焦点改由下方 Focus 事件驱动。
    // selectable/focusable 是静态属性、不变化，重复声明只会引发无效 re-query，去掉。
    QAccessible::State changed;
    changed.selected = true;

    QAccessibleStateChangeEvent stateEvent(item, changed);
    QAccessible::updateAccessibility(&stateEvent);

    if (focused || selected) {
        QAccessibleEvent focusEvent(item, QAccessible::Focus);
        QAccessible::updateAccessibility(&focusEvent);
    }
}
