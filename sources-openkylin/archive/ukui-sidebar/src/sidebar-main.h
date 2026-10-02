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

#ifndef UKUI_SIDEBAR_SIDEBAR_MAIN_H
#define UKUI_SIDEBAR_SIDEBAR_MAIN_H

#include <QObject>

class QMenu;
class QAction;
class QSystemTrayIcon;

class ShortcutsWindow;

namespace Sidebar {
class SidebarView;
class SidebarDbusService;
}

namespace UkuiNotification {
    class NotificationCenterWindow;
    class PopupNotificationWindow;
    class TabletPopupView;
}

class SidebarMain : public QObject
{
    Q_OBJECT
public:
    enum Request {
        Active,
        Show,
        Hide,
        DirectlyHide
    };
    explicit SidebarMain(QObject *parent = nullptr);
    ~SidebarMain() override;

    void requestWidgetActive(const QString &id, bool showReturnButton);

public Q_SLOTS:
    void parseMessage(const QString &msg);
    void requestSidebar(SidebarMain::Request request);
    void requestShortcut(SidebarMain::Request request);
    void requestNotificationCenter(SidebarMain::Request request);

Q_SIGNALS:
    void stateChange(const QString &module, const QVariantMap &data);

private Q_SLOTS:
    void loadTabletWindows();

private:
    void startSidebar();

    static void registerItems();

    inline void initPublicObjects();
    void loadQML();
    //系统托盘 设置menu界面、添加动作 和 创建systray实例
    void createSystray();
    void updateSystrayIcon(bool clearRedPoint = true);
    void sidebarStateChanged(bool clearMsg = true);

    void initSidebar();
    void initShortcut();
    void initNotificationTablet();
    void initPopupWindow();
    void initTabletPopupWindow();

private:
    // 侧边栏-控制中心
    ShortcutsWindow *m_shortcutsWindow {nullptr};

    // 侧边栏-PC通知中心
    Sidebar::SidebarView *m_notificationPC {nullptr};
    // 侧边栏-平板模式通知中心
    UkuiNotification::NotificationCenterWindow *m_notificationTablet {nullptr};
    UkuiNotification::PopupNotificationWindow *m_popupPC {nullptr};
    UkuiNotification::TabletPopupView *m_popupTablet {nullptr};

    // 系统托盘
    QSystemTrayIcon* m_systemTray {nullptr};
    //QMenu *m_systemTrayMenu {nullptr};
};

#endif // UKUI_SIDEBAR_SIDEBAR_MAIN_H
