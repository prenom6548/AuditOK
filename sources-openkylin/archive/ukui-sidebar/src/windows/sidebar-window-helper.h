/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef UKUI_SIDEBAR_SIDEBAR_WINDOW_HELPER_H
#define UKUI_SIDEBAR_SIDEBAR_WINDOW_HELPER_H

#include <QRect>
#include <QQuickWindow>

#include "window-type.h"

namespace Sidebar {

/**
 * 1.管理不同平台设置geometry的接口
 * 2.为某类型的窗口提供geometry属性
 */
class SidebarWindowHelper : public QObject
{
    Q_OBJECT
public:
    static SidebarWindowHelper *instance(QObject *parent = nullptr);

    QRect getWindowGeometry(SidebarWindowType::Type type);
    Qt::LayoutDirection getLayoutDirection();
    int getPanelPos();

Q_SIGNALS:
    void geometryChanged();

private Q_SLOTS:
    void updateRects();

private:
    explicit SidebarWindowHelper(QObject *parent = nullptr);

private:
    QRect m_statusBarRect;
    QRect m_rightGestureRect;
    QRect m_notificationCenterRect;

    int m_panelSize = 0;
    int m_panelPos = 0;
};

class SidebarWindowDefineModule
{
public:
    static void defineModules(const char *uri, int versionMajor, int versionMinor);
};

} // Sidebar

#endif //UKUI_SIDEBAR_SIDEBAR_WINDOW_HELPER_H
