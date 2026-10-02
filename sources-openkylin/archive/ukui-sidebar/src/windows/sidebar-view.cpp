/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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

#include "sidebar-view.h"
#include "sidebar-window-helper.h"
#include "global-settings.h"
#include "window-blur-helper.h"
#include "notification-window.h"
#include "screen-monitor.h"
#include <window-helper.h>

#include <QUrl>
#include <QQmlEngine>
#include <QQmlContext>
#include <QGuiApplication>
#include <QQuickItem>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <private/qtx11extras_p.h>
#else
#include <QX11Info>
#endif
#include <QScreen>

#define UKUI_PANEL_SETTING          "org.ukui.panel.settings"
#define UKUI_PANEL_POSITION_KEY     "panelposition"

// 两种模式下侧边栏按钮区域的宽度
#define PC_SIDEBAR_WIDTH     384
#define TABLET_SIDEBAR_WIDTH 540

using namespace Sidebar;

SidebarView::SidebarView(QWindow *parent) : SharedEngineView(parent)
{
    setColor(Qt::transparent);
    setResizeMode(SharedEngineView::SizeRootObjectToView);
    setFlags(Qt::FramelessWindowHint | Qt::Window);

    m_windowProxy = new UkuiQuick::WindowProxy(this);
    m_windowProxy->setWindowType(UkuiQuick::WindowType::SystemWindow);

    m_windowBlurHelper = new WindowBlurHelper(this);
    m_windowBlurHelper->setWindowProxy(m_windowProxy);
    connect(m_windowBlurHelper, &WindowBlurHelper::radiusChanged, this, [this] {
        // * wm...
        update();
    });

    // 初始化窗口，监听任务栏变化
    m_screen = QGuiApplication::primaryScreen();
    setScreen(m_screen);
    connect(m_screen, &QScreen::geometryChanged, this, &SidebarView::updateWindowGeometry);

    connect(ScreenMonitor::getInstance(), &ScreenMonitor::screenRemoved, this, [&](QScreen *screen) {
        if(screen == m_screen) {
            activeWindow(false);
        }
    }, Qt::UniqueConnection);

    setIsNotificationEmpty(UkuiNotification::NotificationCenterWindow::globalGroupModel()->rowCount(QModelIndex()) == 0);
    connect(UkuiNotification::NotificationCenterWindow::globalGroupModel(), &UkuiNotification::NotificationGroupModel::rowsInserted,
            this, [this] {
        setIsNotificationEmpty(UkuiNotification::NotificationCenterWindow::globalGroupModel()->rowCount(QModelIndex()) == 0);
    });
    connect(UkuiNotification::NotificationCenterWindow::globalGroupModel(), &UkuiNotification::NotificationGroupModel::rowsRemoved,
            this, [this] {
        setIsNotificationEmpty(UkuiNotification::NotificationCenterWindow::globalGroupModel()->rowCount(QModelIndex()) == 0);
    });

    // 2.平板模式切换
    onSystemModeChanged();
    connect(GlobalSettings::globalInstance(), &GlobalSettings::valueChanged, this, [this] (const QString &key) {
        if (key == TABLET_MODE) {
            onSystemModeChanged();
        } else if (key == UKUI_PANEL_POSITION_KEY       || key == UKUI_PANEL_SIZE_KEY ||
                   key == UKUI_PANEL_TYPE_KEY           || key == UKUI_SETTINGS_ISLAND_POSITION_KEY ||
                   key == UKUI_DATA_ISLAND_POSITION_KEY || key == UKUI_TOPBAR_SIZE_KEY ) {
            updateWindowGeometry();
        }
    });

    connect(this, &QQuickView::activeFocusItemChanged, this, [this] {
        if (!activeFocusItem()) {
            activeWindow(false);
        }
    });

    connect(this, &QQuickView::visibleChanged, this, [this] (bool visible) {
        if (visible) {
            if (m_screen != UkuiQuick::WindowProxy::currentScreen()) {
                if (m_screen) {
                    m_screen->disconnect(this);
                }
                m_screen = UkuiQuick::WindowProxy::currentScreen();
                setScreen(m_screen);
                connect(m_screen, &QScreen::geometryChanged, this, &SidebarView::updateWindowGeometry, Qt::UniqueConnection);
                updateWindowGeometry();
            }
        }
    });

    rootContext()->setContextProperty("sidebarWindow", this);
    rootContext()->setContextProperty("windowBlurHelper", m_windowBlurHelper);

    updateWindowGeometry();
}

void SidebarView::init()
{
    // init
    engine()->addImportPath("qrc:/qml");
    setSource(QUrl("qrc:/qml/Sidebar.qml"));
}

void SidebarView::activeWindow(bool active)
{
    if (!rootObject()) {
        return;
    }

    // 获取是否存在执行中的动画
    QVariant isRunning(false);
    QMetaObject::invokeMethod(rootObject(), "isRunning", Qt::DirectConnection, Q_RETURN_ARG(QVariant, isRunning), Q_ARG(QVariant, active));

    if (isRunning.toBool() || (active == isVisible())) {
        return;
    }

    // 初始化动画属性
    QMetaObject::invokeMethod(rootObject(), "updateProperty", Qt::DirectConnection, Q_ARG(QVariant, active));

    if (active) {
        show();
        Q_EMIT requestShowContent();

    } else {
        Q_EMIT requestHideContent();
    }
}

void SidebarView::onSystemModeChanged()
{
    bool isTabletMode = GlobalSettings::globalInstance()->getValue(TABLET_MODE).toBool();
    if (isTabletMode == m_isTabletMode) {
        return;
    }

    m_isTabletMode = isTabletMode;

    // TODO: 平板切换隐藏动画
//    activeWindow(false);
    setWindowVisible(false);

    //LayoutConfig::getInstance()->updateLayout(m_isTabletMode);
    updateWindowGeometry();

    Q_EMIT isTabletModeChanged();
}

bool SidebarView::isTabletMode()
{
    return m_isTabletMode;
}

void SidebarView::updateWindowGeometry()
{
    if (!m_screen || m_screen->geometry().isEmpty()) return;

    QRect screenRect = m_screen->geometry();

    // 8%的屏幕宽度为最小滑动距离
    m_minimumThreshold   = static_cast<qint32>(screenRect.width() * 0.05);
    m_primaryScreenRight = screenRect.right();

    updateGeometryOfWindow();
    updateGeometryOfMask();

    if (m_windowGeometry.isEmpty()) return;
    setGeometry(m_windowGeometry);
    m_windowProxy->setPosition(m_windowGeometry.topLeft());

    setMask(m_maskGeometry);

    m_windowBlurHelper->setRegion(m_padding + m_expandPadding,
                                  m_windowGeometry.height() - m_blurHeight - m_padding - m_expandPadding,
                                  m_windowGeometry.width() - m_padding*2 - m_expandPadding*2,
                                  m_blurHeight, radius());

    if (rootObject()) {
        rootObject()->setSize(m_windowGeometry.size());
    }

    Q_EMIT minimumThresholdChanged();
    Q_EMIT primaryScreenRightChanged();
}

qint32 SidebarView::windowPadding() const
{
    return m_padding;
}

qint32 SidebarView::windowExpandPadding() const
{
    return m_expandPadding;
}

void SidebarView::setWindowExpandPadding(int padding)
{
    if (m_expandPadding == padding) return;
    m_expandPadding = padding;
    Q_EMIT windowExpandPaddingChanged();
    updateWindowGeometry();
    m_contentWidth = width() - m_expandPadding * 2;
}

qint32 SidebarView::minimumThreshold() const
{
    return m_minimumThreshold;
}

qint32 SidebarView::primaryScreenRight() const
{
    return m_primaryScreenRight;
}

qint32 SidebarView::minWindowSize() const
{
    return m_minWindowSize - m_padding*2;
}

void SidebarView::setBlurHeight(int height)
{
    m_windowBlurHelper->setRegion(m_padding + m_expandPadding,
                                  m_windowGeometry.height() - height - m_padding - m_expandPadding,
                                  PC_SIDEBAR_WIDTH,
                                  height, radius());
    m_blurHeight = height;
}

qint32 SidebarView::contentWidth() const
{
    return m_contentWidth;
}

qint32 SidebarView::radius() const
{
    bool isOpenGLEnv = rootContext()->contextProperty("isOpenGLEnv").toBool();
    return isOpenGLEnv ? m_radius : 0;
}

void SidebarView::setRadius(int radius)
{
    if (m_radius != radius) {
        m_radius = radius;
        setBlurHeight(m_blurHeight);
        Q_EMIT radiusChanged();
    }
}

bool SidebarView::isWayland()
{
    return UkuiQuick::WindowProxy::isWayland();
}

void SidebarView::updateGeometryOfWindow()
{
//    QRect screenRect = screen()->availableGeometry();
    QRect screenRect = m_screen->geometry();

    int width = PC_SIDEBAR_WIDTH + m_padding * 2;
    int panelPos = Sidebar::GlobalSettings::globalInstance()->getValue(UKUI_PANEL_POSITION_KEY).toInt();
    int panelSize = Sidebar::GlobalSettings::globalInstance()->getValue(UKUI_PANEL_SIZE_KEY).toInt();
    bool isMirrored = (SidebarWindowHelper::instance()->getLayoutDirection() == Qt::LayoutDirection::RightToLeft);

    int panelType = Sidebar::GlobalSettings::globalInstance()->getValue(UKUI_PANEL_TYPE_KEY).toInt();
    int dataIslandPosition = Sidebar::GlobalSettings::globalInstance()->getValue(UKUI_DATA_ISLAND_POSITION_KEY).toInt();
    int settingsIslandPosition = Sidebar::GlobalSettings::globalInstance()->getValue(UKUI_SETTINGS_ISLAND_POSITION_KEY).toInt();
    int topbarSize = Sidebar::GlobalSettings::globalInstance()->getValue(UKUI_TOPBAR_SIZE_KEY).toInt();

    if (panelType == 1) {
        if (dataIslandPosition == 1 || settingsIslandPosition == 1) {
            m_windowGeometry = isMirrored
                    ? screenRect.adjusted(0, topbarSize, -(screenRect.width() - width), -panelSize)
                    : screenRect.adjusted(screenRect.width() - width, topbarSize, 0, -panelSize);
        } else {
            m_windowGeometry = isMirrored
                    ? screenRect.adjusted(0, 0, -(screenRect.width() - width), -panelSize)
                    : screenRect.adjusted(screenRect.width() - width, 0, 0, -panelSize);
        }
    } else {
        // 任务栏位置 上: 1, 下: 0, 左: 2, 右: 3
        switch(panelPos) {
            default:
            case 0:
                m_windowGeometry = isMirrored
                        ? screenRect.adjusted(0, 0, -(screenRect.width() - width), -panelSize)
                        : screenRect.adjusted(screenRect.width() - width, 0, 0, -panelSize);
                break;
            case 1:
                m_windowGeometry = isMirrored
                        ? screenRect.adjusted(0, panelSize, -(screenRect.width() - width), 0)
                        : screenRect.adjusted(screenRect.width() - width, panelSize, 0, 0);
                break;
            case 2:
                m_windowGeometry = isMirrored
                        ? screenRect.adjusted(panelSize, 0, -(screenRect.width() - width - panelSize), 0)
                        : screenRect.adjusted(screenRect.width() - width, 0, 0, 0);
                break;
            case 3:
                m_windowGeometry = isMirrored
                        ? screenRect.adjusted(0, 0, -(screenRect.width() - width), 0)
                        : screenRect.adjusted(screenRect.width() - width - panelSize, 0, -panelSize, 0);
                break;
        }
    }

    m_windowGeometry.adjust(-m_expandPadding, -m_expandPadding, m_expandPadding, m_expandPadding);
}

void SidebarView::updateGeometryOfMask()
{
    QRect windowGeometry = m_windowGeometry.adjusted(m_expandPadding, m_expandPadding, -m_expandPadding, -m_expandPadding);
    m_maskGeometry = QRect(m_expandPadding,
                           m_isNotificationEmpty ? (windowGeometry.height() - m_minWindowSize + m_expandPadding) : m_expandPadding,
                           windowGeometry.width(),
                           m_isNotificationEmpty ? m_minWindowSize : windowGeometry.height());
}


void SidebarView::focusInEvent(QFocusEvent *event)
{
    QQuickWindow::focusInEvent(event);
}

void SidebarView::focusOutEvent(QFocusEvent *event)
{
    QQuickWindow::focusOutEvent(event);
}

void SidebarView::resizeEvent(QResizeEvent *event)
{
    SharedEngineView::resizeEvent(event);
}

bool SidebarView::event(QEvent *event)
{
    if (event->type() == QEvent::Move) {
        updateWindowGeometry();
        return true;
    }
    return QQuickWindow::event(event);
}

void SidebarView::exposeEvent(QExposeEvent *event)
{
    if (isExposed()) {
        if (QX11Info::isPlatformX11()) {
            requestActivate();
        }
    }
    QQuickWindow::exposeEvent(event);
}

bool SidebarView::contentVisible() const
{
    return m_contentVisible;
}

void SidebarView::setContentVisible(bool visible)
{
    m_contentVisible = visible;
    Q_EMIT contentVisibleChanged();
}

bool SidebarView::isNotificationEmpty() const
{
    return m_isNotificationEmpty;
}

void SidebarView::setIsNotificationEmpty(bool isEmpty)
{
    if (m_isNotificationEmpty == isEmpty) return;
    m_isNotificationEmpty = isEmpty;
    Q_EMIT isNotificationEmptyChanged();
}

bool SidebarView::isWindowFold() const
{
    return m_isWindowFold;
}

void SidebarView::setIsWindowFold(bool isFold)
{
    if (m_isWindowFold == isFold) return;
    m_isWindowFold = isFold;

    updateGeometryOfMask();
    setMask(m_maskGeometry);

    Q_EMIT isWindowFoldChanged();
}

void SidebarView::setWindowVisible(bool visible)
{
    setVisible(visible);
    if (!visible) {
        setContentVisible(visible);
    }
}
