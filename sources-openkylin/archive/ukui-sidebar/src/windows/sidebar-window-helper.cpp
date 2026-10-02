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

#include "sidebar-window-helper.h"
#include "screen-monitor.h"
//#include "layout-config.h"
#include "hand-gesture-helper.h"
#include "global-settings.h"

// 窗管特效接口
#include <KWindowEffects>
#include <KWindowSystem>

// Qt
#include <QQuickItem>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <private/qtx11extras_p.h>
#else
#include <QX11Info>
#endif
#include <QMouseEvent>
#include <QTouchEvent>

#define HAND_GESTURE_WIDTH 2

namespace Sidebar {

static SidebarWindowHelper *globalWindowHelperInstance = nullptr;

void SidebarWindowDefineModule::defineModules(const char *uri, int versionMajor, int versionMinor)
{
    qRegisterMetaType<Sidebar::SidebarWindowType::Type>("SidebarWindowType::Type");
    qmlRegisterUncreatableType<Sidebar::SidebarWindowType>(uri, versionMajor, versionMinor, "SidebarWindowType", "Enum");

    qmlRegisterRevision<QWindow, 2>(uri, versionMajor, versionMinor);
    qmlRegisterRevision<QQuickWindow, 2>(uri, versionMajor, versionMinor);
}

SidebarWindowHelper *SidebarWindowHelper::instance(QObject *parent)
{
    if (!globalWindowHelperInstance) {
        globalWindowHelperInstance = new SidebarWindowHelper(parent);
    }

    return globalWindowHelperInstance;
}

SidebarWindowHelper::SidebarWindowHelper(QObject *parent) : QObject(parent)
{
    ScreenMonitor *screenMonitor = ScreenMonitor::getInstance();
    connect(screenMonitor, &ScreenMonitor::geometryChanged, this, &SidebarWindowHelper::updateRects);
    connect(screenMonitor, &ScreenMonitor::primaryScreenChanged, this, &SidebarWindowHelper::updateRects);
    connect(screenMonitor, &ScreenMonitor::layoutDirectionChanged, this, &SidebarWindowHelper::updateRects);

    connect(Sidebar::GlobalSettings::globalInstance(), &Sidebar::GlobalSettings::valueChanged, this, [this] (const QString &key) {
        if (key == UKUI_PANEL_POSITION_KEY) {
            m_panelPos = Sidebar::GlobalSettings::globalInstance()->getValue(UKUI_PANEL_POSITION_KEY).toInt();
            updateRects();
        } else if (key == UKUI_PANEL_SIZE_KEY) {
            m_panelSize = Sidebar::GlobalSettings::globalInstance()->getValue(UKUI_PANEL_SIZE_KEY).toInt();
            updateRects();
        }
    });

    m_panelPos = Sidebar::GlobalSettings::globalInstance()->getValue(UKUI_PANEL_POSITION_KEY).toInt();
    m_panelSize = Sidebar::GlobalSettings::globalInstance()->getValue(UKUI_PANEL_SIZE_KEY).toInt();

    updateRects();
}

QRect SidebarWindowHelper::getWindowGeometry(SidebarWindowType::Type type)
{
    switch (type) {
        case SidebarWindowType::NotificationCenter:
            return m_notificationCenterRect;
        case SidebarWindowType::StatusBar:
            return m_statusBarRect;
        case SidebarWindowType::RightHandGesture:
            return m_rightGestureRect;
        default:
            return {};
    }
}

Qt::LayoutDirection SidebarWindowHelper::getLayoutDirection()
{
    return ScreenMonitor::getInstance()->getLayoutDirection();
}

int SidebarWindowHelper::getPanelPos()
{
    return m_panelPos;
}

/**
 * 根据不同的窗口类型与主屏幕参数设置每一个窗口的geometry属性
 */
void SidebarWindowHelper::updateRects()
{
    QRect primaryScreenRect = ScreenMonitor::getInstance()->getGeometry();
    bool isMirrored = ScreenMonitor::getInstance()->getLayoutDirection() == Qt::LayoutDirection::RightToLeft;

    m_notificationCenterRect = primaryScreenRect;

    m_statusBarRect.setRect(primaryScreenRect.x(), primaryScreenRect.y(), primaryScreenRect.width(), HAND_GESTURE_WIDTH);

    QRect rightGestureRect;
    rightGestureRect.setX(isMirrored ?
                              primaryScreenRect.x() : primaryScreenRect.x() + primaryScreenRect.width() - HAND_GESTURE_WIDTH);
    rightGestureRect.setWidth(HAND_GESTURE_WIDTH);

    switch (m_panelPos) {
        default:
        case 0:
            rightGestureRect.setY(primaryScreenRect.y() + HAND_GESTURE_WIDTH);
            rightGestureRect.setHeight(primaryScreenRect.height() - HAND_GESTURE_WIDTH - m_panelSize);
            break;
        case 1:
            rightGestureRect.setY(primaryScreenRect.y() + m_panelSize);
            rightGestureRect.setHeight(primaryScreenRect.height() - m_panelSize);
            break;
        case 2:
        case 3:
            rightGestureRect.setY(primaryScreenRect.y() + HAND_GESTURE_WIDTH);
            rightGestureRect.setHeight(primaryScreenRect.height() - HAND_GESTURE_WIDTH);
            break;
    }

    m_rightGestureRect = rightGestureRect;

    Q_EMIT geometryChanged();
}

} // Sidebar
