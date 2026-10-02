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

#include <QQmlEngine>
#include <QQmlContext>
#include <KWindowSystem>
#include <KWindowEffects>
#include <QScreen>
#include <QApplication>
#include <QPainterPath>
#include <window-helper.h>
#include <QQuickItem>
#include "global-settings.h"

#include "popup-notification-window.h"
#include "notification-group-model.h"
#include "notification-model.h"
#include "screen-monitor.h"
#include "popup-notification-model.h"

using namespace UkuiNotification;
PopupNotificationWindow::PopupNotificationWindow(QWindow *parent) : SharedEngineView(parent)
{
    initWindow();
    initNotificationModel();
}

void PopupNotificationWindow::initWindow()
{
    setResizeMode(SharedEngineView::SizeViewToRootObject);
    setFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setColor("transparent");

    m_screenLockState = Sidebar::ScreenMonitor::getInstance()->getScreenLockState();
    m_windowProxy = new UkuiQuick::WindowProxy(this);
    m_windowProxy->setWindowStates(UkuiQuick::WindowState::Activatable, false);
//    m_windowProxy->setWindowType(m_screenLockState ? UkuiQuick::WindowType::ScreenLockNotification : UkuiQuick::WindowType::Notification);
    m_windowProxy->setWindowType(UkuiQuick::WindowType::ScreenLockNotification);
    Q_EMIT screenLockStateChanged();

    // 监听窗口尺寸变化,屏幕变化
    onPrimaryScreenChanged(Sidebar::ScreenMonitor::getInstance()->getPrimaryScreen());

    connect(Sidebar::ScreenMonitor::getInstance(), &Sidebar::ScreenMonitor::primaryScreenChanged, this, [this] {
        onPrimaryScreenChanged(Sidebar::ScreenMonitor::getInstance()->getPrimaryScreen());
    });
    connect(Sidebar::ScreenMonitor::getInstance(), &Sidebar::ScreenMonitor::panelPropertyChanged, this, &PopupNotificationWindow::updateGeometry);
    connect(Sidebar::ScreenMonitor::getInstance(), &Sidebar::ScreenMonitor::layoutDirectionChanged, this, &PopupNotificationWindow::updateGeometry);
    connect(Sidebar::ScreenMonitor::getInstance(), &Sidebar::ScreenMonitor::screenLockStateChanged, this, [this] (bool state) {
        if (m_screenLockState != state) {
//            m_windowProxy->setWindowType(state ? UkuiQuick::WindowType::ScreenLockNotification : UkuiQuick::WindowType::Notification);
            m_screenLockState = state;
            Q_EMIT screenLockStateChanged();
        }
    });

    connect(Sidebar::GlobalSettings::globalInstance(), &Sidebar::GlobalSettings::valueChanged, this, [this] (const QString &key) {
        if (key == UKUI_PANEL_TYPE_KEY           || key == UKUI_SETTINGS_ISLAND_POSITION_KEY ||
            key == UKUI_DATA_ISLAND_POSITION_KEY || key == UKUI_TOPBAR_SIZE_KEY ) {
            updateGeometry();
        }
    });
}

void PopupNotificationWindow::initNotificationModel()
{
    auto popupFilterModel = new PopupNotificationModel(this);
    popupFilterModel->setSourceModel(NotificationModel::instance());

    auto groupModel = new NotificationGroupModel(this);
    groupModel->setSourceModel(popupFilterModel);

    rootContext()->setContextProperty("groupModel", groupModel);
    rootContext()->setContextProperty("sourceModel", NotificationModel::instance());
    rootContext()->setContextProperty("popupNotificationWindow", this);

    connect(groupModel, &QAbstractItemModel::rowsInserted, groupModel, [this, groupModel] {
        setVisible(groupModel->rowCount(QModelIndex()) > 0);
    });
    connect(groupModel, &QAbstractItemModel::rowsRemoved, groupModel, [this, groupModel] {
        setVisible(groupModel->rowCount(QModelIndex()) > 0);
    });
}

void PopupNotificationWindow::loadQML()
{
    engine()->addImportPath("qrc:/qml");
    setSource(QUrl("qrc:/qml/PopupView.qml"));
    if (rootObject()) {
        m_viewWidth = rootObject()->width();
        connect(rootObject(), &QQuickItem::widthChanged, this, &PopupNotificationWindow::onRootWidthChanged);
    }
}

void PopupNotificationWindow::updateGeometry()
{
    if (m_viewWidth <= 0 || !m_screen) {
        return;
    }
    QRect screenRect = m_screen->geometry();
    if (screenRect.isEmpty()) return;

    Sidebar::ScreenMonitor *screenMonitor = Sidebar::ScreenMonitor::getInstance();
    bool isMirrored = screenMonitor->getLayoutDirection() == Qt::LayoutDirection::RightToLeft;

    int panelSize = screenMonitor->getPanelSize();
    int panelPosition = screenMonitor->getPanelPosition();
    int panelType = Sidebar::GlobalSettings::globalInstance()->getValue(UKUI_PANEL_TYPE_KEY).toInt();
    int dataIslandPosition = Sidebar::GlobalSettings::globalInstance()->getValue(UKUI_DATA_ISLAND_POSITION_KEY).toInt();
    int settingsIslandPosition = Sidebar::GlobalSettings::globalInstance()->getValue(UKUI_SETTINGS_ISLAND_POSITION_KEY).toInt();
    int topbarSize = Sidebar::GlobalSettings::globalInstance()->getValue(UKUI_TOPBAR_SIZE_KEY).toInt();

    int margin = 0;
    int maxWindowHeight;

    if (panelType == 1) {
        if (dataIslandPosition == 1 || settingsIslandPosition == 1) {
            m_notificationPoint = QPoint(isMirrored ?
                                             margin : screenRect.width() - m_viewWidth - margin,
                                         margin + topbarSize);
            maxWindowHeight = screenRect.height() - panelSize - topbarSize - margin * 2;
        } else {
            m_notificationPoint = QPoint(isMirrored ?
                                             margin : screenRect.width() - m_viewWidth - margin,
                                         margin);
            maxWindowHeight = screenRect.height() - panelSize - margin * 2;
        }
    } else {
        switch(panelPosition) {
            default:
            case 0: {
                m_notificationPoint = QPoint(isMirrored ?
                                                 margin : screenRect.width() - m_viewWidth - margin,
                                             margin);
                maxWindowHeight = screenRect.height() - panelSize - margin * 2;
                break;
            }
            case 1: {
                m_notificationPoint = QPoint(isMirrored ?
                                                 margin : screenRect.width() - m_viewWidth - margin,
                                             margin + panelSize);
                maxWindowHeight = screenRect.height() - panelSize - margin * 2;
                break;
            }
            case 2: {
                m_notificationPoint = QPoint(isMirrored ?
                                                 margin + panelSize : screenRect.width() - m_viewWidth - margin,
                                             margin);
                maxWindowHeight = screenRect.height() - margin * 2;
                break;
            }
            case 3: {
                m_notificationPoint = QPoint(isMirrored ?
                                                 margin : screenRect.width() - m_viewWidth - margin - panelSize,
                                             margin);
                maxWindowHeight = screenRect.height() - margin * 2;
                break;
            }
        }
    }

    if (maxWindowHeight <= 0) return;
    setMaximumHeight(maxWindowHeight);
    // 多屏状态下，坐标位置确定
    m_notificationPoint.setX(m_notificationPoint.x() + screenRect.x());
    m_notificationPoint.setY(m_notificationPoint.y() + screenRect.y());

    if (m_needDisplace) {
        if (isMirrored) {
            m_notificationPoint.setX(m_notificationPoint.x() + m_sidebarWidth);
        } else {
            m_notificationPoint.setX(m_notificationPoint.x() - m_sidebarWidth);
        }
    }

    m_windowProxy->setPosition(m_notificationPoint);
}

void PopupNotificationWindow::updataWindowRegion(QVariantMap windowRect, int contentY)
{
    QVariantList groupsRect = windowRect.value("regions").toList();
    QRegion windowRegion;
    int nextGroupsY = 0;

    for (int i = 0; i < groupsRect.length(); i ++) {
        int count = groupsRect.at(i).toMap().value("count").toInt();
        int height = groupsRect.at(i).toMap().value("height").toInt();
        int width = groupsRect.at(i).toMap().value("width").toInt();
        int radius = groupsRect.at(i).toMap().value("radius").toInt();
        int spacing = 8;
        int groupsRightBottom = 0;
        QRegion childRegion;
        QPainterPath path;

        path.addRoundedRect(0, 0, width, height, radius, radius);
        QRegion groupsRegion = QRegion(path.toFillPolygon().toPolygon());

        if (count == 1) {
            childRegion = groupsRegion;
            groupsRightBottom = height;

        } else if (count == 2) {
            path.addRoundedRect((0.05 * width) / 2, spacing, width * 0.95, height, radius, radius);
            QRegion folderRegion = QRegion(path.toFillPolygon().toPolygon());
            childRegion = groupsRegion.united(folderRegion);
            groupsRightBottom = height + spacing;

        } else if (count > 2) {
            path.addRoundedRect((0.05 * width) / 2, spacing, width * 0.95, height, radius, radius);
            QRegion folderRegion = QRegion(path.toFillPolygon().toPolygon());
            path.addRoundedRect((0.0975 * width) / 2, spacing * 2, width * 0.9025, height, radius, radius);
            QRegion secondFolderRegion = QRegion(path.toFillPolygon().toPolygon());
            childRegion = groupsRegion.united(folderRegion).united(secondFolderRegion);
            groupsRightBottom = height + spacing * 2;
        }

        if (i > 0) {
            childRegion.translate(0, nextGroupsY);
        }

        nextGroupsY = nextGroupsY + groupsRightBottom + spacing;
        windowRegion = windowRegion.united(childRegion);
    }
    m_windowRegion = windowRegion.translated(0, -contentY);
    enableWindowBlur(m_enable);
}

void PopupNotificationWindow::updataGroupsPosition(int relativeY)
{
    m_windowRegion.translate(QPoint(0, -relativeY));
}

void PopupNotificationWindow::enableWindowBlur(bool enable)
{
    if (m_enable == enable) return;
    m_enable = enable;

    if (m_windowProxy == nullptr) return;
    m_windowProxy->setBlurRegion(m_enable, m_windowRegion);
}

void PopupNotificationWindow::updatePopupWindowGeometry(bool needDisplace, int sidebarWidth)
{
    if (m_needDisplace == needDisplace && m_sidebarWidth == sidebarWidth) {
        return;
    }
    m_needDisplace = needDisplace;
    m_sidebarWidth = sidebarWidth;
    updateGeometry();
}

bool PopupNotificationWindow::event(QEvent *event)
{
    return SharedEngineView::event(event);
}

void PopupNotificationWindow::onPrimaryScreenChanged(QScreen *newScreen)
{
    if (!newScreen) {
        return;
    }

    if (m_screen) {
        m_screen->disconnect(this);
    }

    m_screen = newScreen;
    setScreen(m_screen);
    updateGeometry();

    connect(m_screen, &QScreen::geometryChanged, this, &PopupNotificationWindow::updateGeometry);
}

void PopupNotificationWindow::onRootWidthChanged()
{
    if (rootObject()->width() == m_viewWidth) {
        return;
    }
    m_viewWidth = rootObject()->width();
    updateGeometry();
}

int PopupNotificationWindow::viewWidth() const
{
    return m_viewWidth;
}

int PopupNotificationWindow::itemWidth() const
{
    return m_itemWidth;
}

bool PopupNotificationWindow::screenLockState() const
{
    return m_screenLockState;
}
