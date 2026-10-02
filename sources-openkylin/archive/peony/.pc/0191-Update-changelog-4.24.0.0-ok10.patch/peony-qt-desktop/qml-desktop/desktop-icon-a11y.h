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

#ifndef DESKTOP_ICON_A11Y_H
#define DESKTOP_ICON_A11Y_H

#include <QObject>

class QQuickItem;

class DesktopIconA11y : public QObject
{
    Q_OBJECT

public:
    static DesktopIconA11y *instance();

    Q_INVOKABLE void notifyIconState(QQuickItem *item, bool selected, bool focused);

private:
    explicit DesktopIconA11y(QObject *parent = nullptr);
    void postStateChange(QQuickItem *item, bool selected, bool focused);
};

#endif // DESKTOP_ICON_A11Y_H
