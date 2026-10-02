/*
* Copyright (C) 2026, KylinSoft Co., Ltd.
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

#ifndef AUTOTESTS_SRC_STUB_SIDEBAR_MAIN_DEPS_H
#define AUTOTESTS_SRC_STUB_SIDEBAR_MAIN_DEPS_H

#include <QQuickItem>
#include <QSystemTrayIcon>

#include "../windows/stubs/shared-engine-view.h"
#include "../windows/stubs/window-helper.h"
#include "../windows/stubs/global-settings.h"
#include "../windows/stubs/screen-monitor.h"
#include "../windows/stubs/hand-gesture-helper.h"
#include "../windows/stubs/notification-model.h"
#include "../windows/stubs/notification-group-model.h"
#include "../windows/stubs/popup-notification-model.h"
#include "../windows/stubs/sidebar-window-helper.h"
#include "../windows/stubs/event-track.h"
#include "../windows/stubs/date-time-utils.h"
#include "../windows/stubs/notification-window.h"
#include "../windows/stubs/short-cut-manager.h"

#define UKUI_SIDEBAR_GLOBAL_SETTINGS_H
#define UKUI_SIDEBAR_SCREEN_MONITOR_H
#define HANDGESTUREHELPER_H
#define UKUI_SIDEBAR_APP_MANAGER_H
#define UKUI_SIDEBAR_SIDEBAR_WINDOW_HELPER_H
#define UKUI_SIDEBAR_EVENT_TRACK_H
#define UKUI_SIDEBAR_NOTIFICATION_MODEL_H
#define UKUI_SIDEBAR_SIDEBAR_VIEW_H
#define UKUI_SIDEBAR_NOTIFICATION_WINDOW_H
#define UKUI_SIDEBAR_POPUP_NOTIFICATION_WINDOW_H
#define UKUI_SIDEBAR_TABLET_POPUP_VIEW_H
#define UKUI_SIDEBAR_SHORTCUTS_WINDOW_H
#define UKUI_SIDEBAR_STATUS_BAR_VIEW_H
#define UKUI_SIDEBAR_RIGHT_HAND_GESTURE_VIEW_H
#define UKUISHOUTCUT_H
#define WIDGET_DELEGATE_H
#define UKUI_FLOW_H
#define PLUGINSTATE_H
#define UKUI_SIDEBAR_SIDEBAR_DBUS_SERVICE_H
#define UKUISHOUTCUT_H

namespace Sidebar {

class AppManager : public QObject
{
    Q_OBJECT
public:
    static AppManager *getInstance(QObject *parent = nullptr)
    {
        static AppManager manager(parent);
        return &manager;
    }

private:
    explicit AppManager(QObject *parent = nullptr) : QObject(parent) {}
};

class SidebarDbusService : public QObject
{
    Q_OBJECT
public:
    explicit SidebarDbusService(QObject *parent = nullptr) : QObject(parent) {}
};

class SidebarView : public UkuiQuick::SharedEngineView
{
    Q_OBJECT
public:
    explicit SidebarView(QWindow *parent = nullptr) : UkuiQuick::SharedEngineView(parent) {}

    void init() {}
    void activeWindow(bool active)
    {
        m_lastActive = active;
        setVisible(active);
    }
    void setWindowVisible(bool visible)
    {
        m_lastWindowVisible = visible;
        setVisible(visible);
    }
    void setVisible(bool visible)
    {
        if (m_visible == visible) {
            return;
        }
        m_visible = visible;
        Q_EMIT visibleChanged(visible);
    }
    bool isVisible() const
    {
        return m_visible;
    }
    bool isWindowFold() const
    {
        return m_isWindowFold;
    }
    void setIsWindowFold(bool fold)
    {
        m_isWindowFold = fold;
        Q_EMIT isWindowFoldChanged();
    }
    int contentWidth() const
    {
        return m_contentWidth;
    }
    void setContentWidth(int width)
    {
        m_contentWidth = width;
    }
    bool lastActive() const
    {
        return m_lastActive;
    }
    bool lastWindowVisible() const
    {
        return m_lastWindowVisible;
    }

Q_SIGNALS:
    void isWindowFoldChanged();

private:
    bool m_isWindowFold = false;
    int m_contentWidth = 0;
    bool m_lastActive = false;
    bool m_lastWindowVisible = false;
    bool m_visible = false;
};

class SettingMonitor : public QObject
{
    Q_OBJECT
public:
    explicit SettingMonitor(QObject *parent = nullptr) : QObject(parent) {}
};

} // namespace Sidebar

class ShortcutsWindow : public UkuiQuick::SharedEngineView
{
    Q_OBJECT
public:
    explicit ShortcutsWindow(QWindow *parent = nullptr) : UkuiQuick::SharedEngineView(parent) {}

    QString currentId() const
    {
        return m_currentId;
    }

    void setCurrentId(const QString &id)
    {
        m_currentId = id;
    }

    bool requestMenuWidget(QString widgetId = "", bool showReturnButton = false)
    {
        Q_UNUSED(showReturnButton)
        m_requestedWidgetId = widgetId;
        return m_requestMenuWidgetResult;
    }

    void activeShortcutsWindow(bool active = true)
    {
        m_lastActive = active;
        setVisible(active);
    }

    void setVisible(bool visible)
    {
        m_visible = visible;
    }

    bool isVisible() const
    {
        return m_visible;
    }

    void hideMenuWidget()
    {
        ++m_hideMenuWidgetCalls;
    }

    void setRequestMenuWidgetResult(bool result)
    {
        m_requestMenuWidgetResult = result;
    }

    QString requestedWidgetId() const
    {
        return m_requestedWidgetId;
    }

    bool lastActive() const
    {
        return m_lastActive;
    }

    int hideMenuWidgetCalls() const
    {
        return m_hideMenuWidgetCalls;
    }

private:
    QString m_currentId;
    QString m_requestedWidgetId;
    bool m_requestMenuWidgetResult = false;
    bool m_lastActive = false;
    int m_hideMenuWidgetCalls = 0;
    bool m_visible = false;
};

class StatusBarView : public UkuiQuick::SharedEngineView
{
    Q_OBJECT
public:
    explicit StatusBarView(QWindow *parent = nullptr) : UkuiQuick::SharedEngineView(parent) {}
    void init() { m_inited = true; }
    bool inited() const { return m_inited; }
private:
    bool m_inited = false;
};

namespace Sidebar {
class RightHandGestureView : public UkuiQuick::SharedEngineView
{
    Q_OBJECT
public:
    explicit RightHandGestureView(QWindow *parent = nullptr) : UkuiQuick::SharedEngineView(parent) {}
    void init() { m_inited = true; }
    bool inited() const { return m_inited; }
private:
    bool m_inited = false;
};
}

namespace UkuiNotification {

class PopupNotificationWindow : public UkuiQuick::SharedEngineView
{
    Q_OBJECT
public:
    explicit PopupNotificationWindow(QWindow *parent = nullptr) : UkuiQuick::SharedEngineView(parent) {}

    void loadQML()
    {
        m_loadQmlCalls++;
    }

    void updatePopupWindowGeometry(bool needDisplace, int sidebarWidth)
    {
        m_lastNeedDisplace = needDisplace;
        m_lastSidebarWidth = sidebarWidth;
    }

    int loadQmlCalls() const
    {
        return m_loadQmlCalls;
    }

    bool lastNeedDisplace() const
    {
        return m_lastNeedDisplace;
    }

    int lastSidebarWidth() const
    {
        return m_lastSidebarWidth;
    }

private:
    int m_loadQmlCalls = 0;
    bool m_lastNeedDisplace = false;
    int m_lastSidebarWidth = 0;
};

class TabletPopupView : public UkuiQuick::SharedEngineView
{
    Q_OBJECT
public:
    explicit TabletPopupView(QWindow *parent = nullptr) : UkuiQuick::SharedEngineView(parent) {}
};

} // namespace UkuiNotification

namespace UkuiShortcut {

class WidgetDelegate : public QQuickItem
{
    Q_OBJECT
public:
    explicit WidgetDelegate(QQuickItem *parent = nullptr) : QQuickItem(parent) {}
};

} // namespace UkuiShortcut

class UkuiPositionerAttached : public QObject
{
    Q_OBJECT
public:
    explicit UkuiPositionerAttached(QObject *parent = nullptr) : QObject(parent) {}
};

class UkuiBasePositioner : public QQuickItem
{
    Q_OBJECT
public:
    explicit UkuiBasePositioner(QQuickItem *parent = nullptr) : QQuickItem(parent) {}
};

class UkuiColumn : public QQuickItem
{
    Q_OBJECT
public:
    explicit UkuiColumn(QQuickItem *parent = nullptr) : QQuickItem(parent) {}
};

class TestSystemTrayIcon : public QSystemTrayIcon
{
    Q_OBJECT
public:
    explicit TestSystemTrayIcon(QObject *parent = nullptr) : QSystemTrayIcon(parent) {}

    static bool isSystemTrayAvailable()
    {
        return true;
    }
};

#define QSystemTrayIcon TestSystemTrayIcon

#endif
