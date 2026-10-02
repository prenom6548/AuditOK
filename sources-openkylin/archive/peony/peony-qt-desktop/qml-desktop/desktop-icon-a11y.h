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

    /// 任务栏通过 DBus 调起桌面聚焦：复用 windowActivated 的入口逻辑
    /// （QML 侧恢复上次聚焦图标，无记录则选中第一个图标）。
    void requestDesktopFocus();

    /// 桌面图标聚焦状态下按 Tab/Shift+Tab 时调用，把焦点移交给任务栏。
    /// \param focusFirst true=任务栏首端，false=任务栏末端（与任务栏接口 activeFocus 对齐）。
    Q_INVOKABLE void focusPanel(bool focusFirst);

Q_SIGNALS:
    /// 桌面窗口被激活时（如 Win+D 从应用窗口切回桌面），用于 QML 入口播报
    void windowActivated();

private:
    explicit DesktopIconA11y(QObject *parent = nullptr);
    void postStateChange(QQuickItem *item, bool selected, bool focused);
};

#endif // DESKTOP_ICON_A11Y_H
