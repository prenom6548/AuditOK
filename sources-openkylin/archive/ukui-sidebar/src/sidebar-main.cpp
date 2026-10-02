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

#include "sidebar-main.h"

#include <QQmlContext>
#include <QUrl>
#include <QDebug>
#include <QThread>
#include <QMenu>
#include <QAction>
#include <QSystemTrayIcon>
#include <QCoreApplication>

#include "global-settings.h"
#include "short-cut-manager.h"
#include "screen-monitor.h"
#include "hand-gesture-helper.h"
#include "date-time-utils.h"
#include "window-blur-helper.h"
#include "app-manager.h"
#include "sidebar-window-helper.h"
#include "sidebar-view.h"
#include "notification-window.h"
#include "popup-notification-window.h"
#include "notification-model.h"
#include "tablet-popup-view.h"
#include "status-bar-view.h"
#include "right-hand-gesture-view.h"
#include "event-track.h"
#include "shortcuts-window.h"
#include "sidebar-dbus-service.h"
#include "model/widget-delegate.h"
#include "layout/ukui-column.h"
#include "plugin-state.h"

//注册托盘
#define  TRAY_ICON           ":/icon/kylin-tool-box.svg"
#define  TRAY_NULL_ICON      ":/icon/kylin-tool-box-null.svg"
#define  SETTING_ICON        ":/icon/application-menu.svg"
#define  SHORTCUT_TRAY_ICON  ":/icon/ukui-zs-kernel-version-symbolic.svg"

using namespace Sidebar;

SidebarMain::SidebarMain(QObject *parent) : QObject(parent)
{
    createSystray();
    startSidebar();
}

SidebarMain::~SidebarMain()
{
    //if (m_systemTrayMenu) {
    //    delete m_systemTrayMenu;
    //    m_systemTrayMenu = nullptr;
    //}

    if (m_popupPC) {
        delete m_popupPC;
        m_popupPC = nullptr;
    }
}

void SidebarMain::requestWidgetActive(const QString &id, bool showReturnButton)
{
    if (m_shortcutsWindow) {
        bool same = (id == m_shortcutsWindow->currentId());
        if (m_shortcutsWindow->requestMenuWidget(id, showReturnButton)) {
            requestShortcut(same ? SidebarMain::Active : SidebarMain::Show);
        }
    }
}

void SidebarMain::initPublicObjects()
{
    GlobalSettings::globalInstance();
    ScreenMonitor::getInstance();
    HandGestureHelper::getInstance();
    SidebarWindowHelper::instance();
}

void SidebarMain::loadQML()
{
    QQmlContext *rootContext = UkuiQuick::SharedEngineView::sharedEngine()->rootContext();
    rootContext->setContextProperty("isOpenGLEnv", false);
    rootContext->setContextProperty("dateTimeUtils", new Sidebar::DateTimeUtils(this));
    rootContext->setContextProperty("appManager", AppManager::getInstance());
    rootContext->setContextProperty("screenMonitor", ScreenMonitor::getInstance());

    const bool isLiteMode = GlobalSettings::globalInstance()->getValue(IS_LITE_MODE).toBool();
    rootContext->setContextProperty("isLiteMode", isLiteMode);

    initShortcut();
    // 更新特效开关变量
    rootContext->setContextProperty("isOpenGLEnv", !isLiteMode && (QQuickWindow::sceneGraphBackend() == ""));

    initSidebar();
    initPopupWindow();

    if (!isLiteMode) {
        loadTabletWindows();
        connect(GlobalSettings::globalInstance(), &GlobalSettings::valueChanged, this, [this] (const QString &key) {
            if (key == TABLET_MODE) {
                loadTabletWindows();
            }
        });
    }

    connect(GlobalSettings::globalInstance(), &GlobalSettings::valueChanged, this, [rootContext] (const QString &key) {
        if (key == IS_LITE_MODE) {
            const bool isLiteMode = GlobalSettings::globalInstance()->getValue(key).toBool();
            rootContext->setContextProperty("isLiteMode", isLiteMode);
            rootContext->setContextProperty("isOpenGLEnv", !isLiteMode && (QQuickWindow::sceneGraphBackend() == ""));
        }
    });
    qDebug() << "loadQML isOpenGLEnv:" << (QQuickWindow::sceneGraphBackend() == "") << QQuickWindow::sceneGraphBackend();
}

void SidebarMain::initSidebar()
{
    m_notificationPC = new SidebarView;

    m_notificationPC->rootContext()->setContextProperty("isNotificationCenter", false);
    m_notificationPC->rootContext()->setContextProperty("groupModel", UkuiNotification::NotificationCenterWindow::globalGroupModel());
    m_notificationPC->rootContext()->setContextProperty("sourceModel", UkuiNotification::NotificationModel::instance());
    m_notificationPC->rootContext()->setContextProperty("handGestureHelper", HandGestureHelper::getInstance());
    m_notificationPC->init();

    //屏幕右侧响应左划动作
    RightHandGestureView *handGestureView = new RightHandGestureView;
    handGestureView->init();

    connect(UkuiNotification::NotificationModel::instance(), &UkuiNotification::NotificationModel::unreadMessageCountChanged, [this] {
        bool viewShowed = (m_notificationPC && m_notificationPC->isVisible()) || (m_notificationTablet && m_notificationTablet->isVisible());
        if (!viewShowed) {
            sidebarStateChanged(false);
        }
    });

    connect(m_notificationPC, &SidebarView::visibleChanged, this, [=] (bool arg) {
        UkuiNotification::NotificationModel::instance()->storePopupNotification(arg);
        if (arg) {
            sidebarStateChanged();
        }
    });
}

void SidebarMain::initShortcut()
{
    m_shortcutsWindow = new ShortcutsWindow;
}

void SidebarMain::initNotificationTablet()
{
    m_notificationTablet = new UkuiNotification::NotificationCenterWindow;
    m_notificationTablet->rootContext()->setContextProperty("isNotificationCenter", true);

    m_notificationTablet->init();

    connect(m_notificationTablet, &UkuiNotification::NotificationCenterWindow::visibleChanged, this, [=] (bool arg) {
        if (arg) {
            sidebarStateChanged();
        }
    });

    //状态栏（响应下划动作）
    StatusBarView *statusBarView = new StatusBarView;
    statusBarView->init();
}

void SidebarMain::initPopupWindow()
{
    m_popupPC = new UkuiNotification::PopupNotificationWindow;

    connect(m_notificationPC, &SidebarView::visibleChanged,
            m_popupPC, [=](const bool isSidebarShow) {
                bool needDisplace = isSidebarShow && !m_notificationPC->isWindowFold() && (m_notificationPC->screen() == m_popupPC->screen());
                m_popupPC->updatePopupWindowGeometry(needDisplace, m_notificationPC->contentWidth());
            });

    connect(m_notificationPC, &SidebarView::isWindowFoldChanged,
            m_popupPC, [=]() {
                bool needDisplace = m_notificationPC->isVisible() && !m_notificationPC->isWindowFold() && (m_notificationPC->screen() == m_popupPC->screen());
                m_popupPC->updatePopupWindowGeometry(needDisplace, m_notificationPC->contentWidth());
            });

    m_popupPC->loadQML();
}

void SidebarMain::loadTabletWindows()
{
    bool isTabletMode = GlobalSettings::globalInstance()->getValue(TABLET_MODE).toBool();
    if (isTabletMode) {
        if (!m_notificationTablet) {
            initNotificationTablet();
        }

        if (!m_popupTablet) {
            initTabletPopupWindow();
        }
    } else {
        if (m_popupTablet) {
            m_popupTablet->close();
            //m_popupTablet->deleteLater();
            delete m_popupTablet;
            m_popupTablet = nullptr;
        }

        if (m_notificationTablet) {
            disconnect(m_notificationTablet, nullptr, this, nullptr);
            m_notificationTablet->close();
            //m_notificationTablet->deleteLater();
            delete m_notificationTablet;
            m_notificationTablet = nullptr;
        }
    }
}

void SidebarMain::initTabletPopupWindow()
{
    m_popupTablet = new UkuiNotification::TabletPopupView;
}

void SidebarMain::parseMessage(const QString &msg)
{
    QStringList args = msg.split(" ");
    if (args.isEmpty()) {
        return;
    }

    //show
    if (args.first() == QStringLiteral("show")) {
        if (args.length() > 1) {
            if (args.at(1) == QStringLiteral("control")) {
                requestShortcut(Active);
            } else if (args.at(1) == QStringLiteral("notify")) {
                requestNotificationCenter(Active);
            }
        }

    } else if (args.first() == QStringLiteral("quit")) {
        QCoreApplication::quit();
    }
}

void SidebarMain::registerItems()
{
    const char uri[] = "org.ukui.sidebar.core";
    SidebarWindowDefineModule::defineModules(uri, 1, 0);
    // 公用部分 包括自定义图标item
    qmlRegisterType<Sidebar::SettingMonitor>("org.ukui.sidebar.core", 1, 0, "SettingMonitor");
    //qmlRegisterType<Sidebar::WindowBlurHelper>("org.ukui.sidebar.core", 1, 0, "WindowBlurHelper");
    qmlRegisterUncreatableType<UkuiShortcut::Color>("org.ukui.sidebar.core", 1, 0, "PluginColorRole", "Enum");
    qmlRegisterUncreatableType<Sidebar::EventTrack>("org.ukui.sidebar.core", 1, 0, "EventTrack", "Only attached prop.");

    qmlRegisterUncreatableType<UkuiPositionerAttached>("org.ukui.positioner", 1, 0, "UkuiPositioner", "UkuiPositioner is an attached property");
    qmlRegisterUncreatableType<UkuiBasePositioner>("org.ukui.positioner", 1, 0, "UkuiBasePositioner", "UkuiBasePositioner is an abstract type that is only available as an attached property.");
    qmlRegisterType<UkuiColumn>("org.ukui.positioner", 1, 0, "UkuiColumn");

    // 控制中心: 注册类用于在qml访问枚举变量
    qmlRegisterUncreatableType<UkuiShortcut::PluginMetaType>("org.ukui.sidebar.shortcut.core", 1, 0, "ShortcutPlugin", "Enum");
    qmlRegisterUncreatableType<UkuiShortcut::PluginStateType>("org.ukui.sidebar.shortcut.core", 1, 0, "PluginState", "Enum");
    qmlRegisterType<UkuiShortcut::WidgetDelegate>("org.ukui.sidebar.shortcut.core", 1, 0, "WidgetDelegate");

    // 通知中心专用部分
    qmlRegisterUncreatableType<UkuiNotification::NotificationItem>("org.ukui.notification.core", 1, 0, "NotificationItem", "Enum");

    //注册元类型 用于信号与槽，与qml交互
    qRegisterMetaType<UkuiShortcut::Color::ColorRole>("Color::ColorRole");
    qRegisterMetaType<UkuiShortcut::StatusInfo::MenuItem>("StatusInfo::MenuItem");
    qRegisterMetaType<UkuiShortcut::PluginMetaType::Action>("PluginMetaType::Action");
    qRegisterMetaType<UkuiShortcut::PluginMetaType::PluginType>("PluginMetaType::PluginType");
    qRegisterMetaType<UkuiShortcut::PluginMetaType::SystemMode>("PluginMetaType::SystemMode");
    qRegisterMetaType<UkuiShortcut::PluginStateType::AddState>("PluginStateType::AddState");
}

//注册托盘、创建托盘菜单
void SidebarMain::createSystray()
{
    int intervalTime = 100, registerCount = 0;
    while (!QSystemTrayIcon::isSystemTrayAvailable() && registerCount <= 5) {
        ++registerCount;
        intervalTime *= 2;
        QThread::msleep(intervalTime);
    }

    m_systemTray = new QSystemTrayIcon(this);
    if (nullptr == m_systemTray) {
        qWarning() << "Allocate space trayIcon failed";
        return;
    }

    m_systemTray->setIcon(QIcon::fromTheme("ukui-control-symbolic", QIcon(SHORTCUT_TRAY_ICON)));
    m_systemTray->setToolTip(tr("Shortcuts"));
    m_systemTray->setVisible(true);

    //m_systemTrayMenu = new QMenu;
    //auto open = new QAction(QObject::tr("Open"), m_systemTrayMenu);
    //auto openSetUp = new QAction(QIcon::fromTheme("document-page-setup-symbolic", QIcon(SETTING_ICON)),
    //                             tr("Set up notification center"), m_systemTrayMenu);

    //m_systemTrayMenu->addAction(open);
    //m_systemTrayMenu->addAction(openSetUp);

    //m_systemTray->setContextMenu(m_systemTrayMenu);

    //connect(open, &QAction::triggered, [this] {
    //    requestSidebar(Active);
    //});
    //connect(openSetUp, &QAction::triggered, [] {
    //    AppManager::getInstance()->launchAppWithArguments("/usr/share/applications/ukui-control-center.desktop", {"-m", "Notice"}, "ukui-control-center");
    //});
    connect(m_systemTray, &QSystemTrayIcon::activated, [this] (QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger) {
            if (m_shortcutsWindow->currentId() == "") {
                requestShortcut(SidebarMain::Active);
            } else {
                m_shortcutsWindow->hideMenuWidget();
            }
        }
    });
}

void SidebarMain::startSidebar()
{
    SidebarMain::registerItems();

    initPublicObjects();
    loadQML();

    // 初始化侧边栏dbus接口
    new Sidebar::SidebarDbusService(this);
    sidebarStateChanged();
}

void SidebarMain::updateSystrayIcon(bool clearRedPoint)
{
    if (!m_systemTray) {
        return;
    }

    if (clearRedPoint) {
        m_systemTray->setToolTip(tr("NotificationCenter"));
        m_systemTray->setIcon(QIcon::fromTheme("ukui-tool-symbolic", QIcon(TRAY_ICON)));

    } else {
        m_systemTray->setToolTip(tr("%1 Notifications").arg(UkuiNotification::NotificationCenterWindow::globalGroupModel()->rowCount(QModelIndex())));
        m_systemTray->setIcon(QIcon::fromTheme("ukui-tool-box-null-symbolic", QIcon(TRAY_NULL_ICON)));
    }
}

void SidebarMain::sidebarStateChanged(bool clearMsg)
{
    QVariantMap data;
    if (clearMsg) {
        data.insert("unreadMsg", 0);
    } else {
        data.insert("unreadMsg", UkuiNotification::NotificationModel::instance()->getUnreadMessageCount());
    }

    Q_EMIT stateChange("sidebar", data);
}

void SidebarMain::requestSidebar(SidebarMain::Request request)
{
    if (!m_notificationPC) {
        return;
    }

    switch (request) {
        case Active: {
            m_notificationPC->activeWindow(!m_notificationPC->isVisible());
            break;
        }
        case Show: {
            m_notificationPC->activeWindow(true);
            break;
        }
        case Hide: {
            m_notificationPC->activeWindow(false);
            break;
        }
        case DirectlyHide: {
            m_notificationPC->setWindowVisible(false);
            break;
        }
        default:
            break;
    }
}

void SidebarMain::requestNotificationCenter(SidebarMain::Request request)
{
    if (!m_notificationTablet) {
        return;
    }

    switch (request) {
        case Active: {
            m_notificationTablet->activeNotificationCenter(!m_notificationTablet->isVisible());
            break;
        }
        case Show: {
            m_notificationTablet->activeNotificationCenter(true);
            break;
        }
        case Hide: {
            m_notificationTablet->activeNotificationCenter(false);
            break;
        }
        default:
            break;
    }
}

void SidebarMain::requestShortcut(SidebarMain::Request request)
{
    if (!m_shortcutsWindow) {
        return;
    }

    switch (request) {
        case Active: {
            m_shortcutsWindow->activeShortcutsWindow(!m_shortcutsWindow->isVisible());
            break;
        }
        case Show: {
            m_shortcutsWindow->activeShortcutsWindow(true);
            break;
        }
        case Hide: {
            m_shortcutsWindow->activeShortcutsWindow(false);
            break;
        }
        default:
            break;
    }
}
