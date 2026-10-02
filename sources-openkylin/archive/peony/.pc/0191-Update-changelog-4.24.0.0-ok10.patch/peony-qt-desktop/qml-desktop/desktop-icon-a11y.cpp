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

    QAccessible::State changed;
    changed.selectable = true;
    changed.focusable = true;
    changed.selected = true;
    changed.focused = true;

    QAccessibleStateChangeEvent stateEvent(item, changed);
    QAccessible::updateAccessibility(&stateEvent);

    if (focused || selected) {
        QAccessibleEvent focusEvent(item, QAccessible::Focus);
        QAccessible::updateAccessibility(&focusEvent);
    }
}
