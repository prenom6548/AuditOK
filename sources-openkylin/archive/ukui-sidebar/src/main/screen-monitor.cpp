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

//
// Created by hxf on 22-8-4.
//

#include "screen-monitor.h"
#include <mutex>

#include <QVariant>
#include <QApplication>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>

#define SCREENSAVER_SERVICE_NAME         "org.ukui.ScreenSaver"
#define SCREENSAVER_SERVICE_PATH         "/"
#define SCREENSAVER_SERVICE_INTERFACE    "org.ukui.ScreenSaver"

using namespace Sidebar;
static std::once_flag flag;
static ScreenMonitor *global_intance = nullptr;
ScreenMonitor *ScreenMonitor::getInstance()
{
    std::call_once(flag, [ & ] {
       global_intance = new ScreenMonitor();
   });
    return global_intance;
}

int ScreenMonitor::getPanelPosition()
{
    return m_panelProperty.position;
}

int ScreenMonitor::getPanelSize()
{
    return m_panelProperty.size;
}

bool ScreenMonitor::getScreenLockState()
{
    return m_screenLockState;
}

bool ScreenMonitor::getSessionToolsState()
{
    return m_sessionToolsState;
}

ScreenMonitor::ScreenMonitor(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<QScreen*>("QScreen*");

    initPanelMonitor();
    primaryScreenChangedSlot(QApplication::primaryScreen());
    m_direction = QApplication::layoutDirection();
    initScreenSaver();

    connect(qApp, &QApplication::screenAdded, this, &ScreenMonitor::screenAddedSlot);
    connect(qApp, &QApplication::screenRemoved, this, &ScreenMonitor::screenRemovedSlot);
    connect(qApp, &QApplication::primaryScreenChanged, this, &ScreenMonitor::primaryScreenChangedSlot);
    connect(qApp, &QApplication::layoutDirectionChanged, this, &ScreenMonitor::layoutDirectionChangedSlot);
}

void ScreenMonitor::initPanelMonitor()
{
    const QByteArray id(UKUI_PANEL_SETTING);
    if (QGSettings::isSchemaInstalled(id)) {
        m_panelSetting = new QGSettings(id, QByteArray(), this);

        QStringList keys = m_panelSetting->keys();
        if (keys.contains(UKUI_PANEL_POSITION_KEY)) {
            m_panelProperty.position = m_panelSetting->get(UKUI_PANEL_POSITION_KEY).toInt();
        }
        if (keys.contains(UKUI_PANEL_SIZE_KEY)) {
            m_panelProperty.size = m_panelSetting->get(UKUI_PANEL_SIZE_KEY).toInt();
        }

        connect(m_panelSetting, &QGSettings::changed, this, &ScreenMonitor::panelPropertyChangedSlot);
    }
}

void ScreenMonitor::initScreenSaver()
{
    QDBusInterface interface(SCREENSAVER_SERVICE_NAME,
                             SCREENSAVER_SERVICE_PATH,
                             SCREENSAVER_SERVICE_INTERFACE,
                             QDBusConnection::sessionBus());
    if (interface.isValid()) {
        QDBusReply<bool> reply = interface.call("GetLockState");

        if (reply.isValid()) {
            m_screenLockState = reply.value();
        }

        QDBusReply<bool> sessionToolsReply = interface.call("GetSessionToolsState");
        if (sessionToolsReply.isValid()) {
            m_sessionToolsState = sessionToolsReply.value();
        }
    }
    Q_EMIT screenLockStateChanged(m_screenLockState);
    Q_EMIT sessionToolsStateChanged(m_sessionToolsState);

    QDBusConnection::sessionBus().connect(SCREENSAVER_SERVICE_NAME,
                                          SCREENSAVER_SERVICE_PATH,
                                          SCREENSAVER_SERVICE_INTERFACE,
                                         "lock",this,SLOT(screenLockSlot()));

    QDBusConnection::sessionBus().connect(SCREENSAVER_SERVICE_NAME,
                                         SCREENSAVER_SERVICE_PATH,
                                         SCREENSAVER_SERVICE_INTERFACE,
                                         "unlock",this,SLOT(screenUnLockSlot()));

    QDBusConnection::sessionBus().connect(SCREENSAVER_SERVICE_NAME,
                                          SCREENSAVER_SERVICE_PATH,
                                          SCREENSAVER_SERVICE_INTERFACE,
                                          "SessionToolsChanged", this, SLOT(sessionToolsStateChangedSlot(bool)));
}

void ScreenMonitor::primaryScreenChangedSlot(QScreen *screen)
{
    if (!screen) {
        return;
    }

    if (m_primaryScreen) {
        m_primaryScreen->disconnect(this);
    }

    m_primaryScreen = screen;
    connect(m_primaryScreen, &QScreen::geometryChanged, this, &ScreenMonitor::geometryChanged);

    Q_EMIT primaryScreenChanged();
    Q_EMIT geometryChanged(m_primaryScreen->geometry());
}

void ScreenMonitor::screenRemovedSlot(QScreen *screen)
{
    qDebug() << "ScreenMonitor::onScreenRemoved" << screen;
    Q_EMIT screenRemoved(screen);
//    if (screen == m_primaryScreen) {
//        m_primaryScreen = QApplication::primaryScreen();
//    }
}

void ScreenMonitor::screenAddedSlot(QScreen *screen)
{
    Q_UNUSED(screen)
    qDebug() << "ScreenMonitor::onScreenAdded" << screen;
}

void ScreenMonitor::panelPropertyChangedSlot(const QString& key)
{
    if (key == UKUI_PANEL_POSITION_KEY || key == UKUI_PANEL_SIZE_KEY) {
        if (key == UKUI_PANEL_POSITION_KEY) {
            m_panelProperty.position = m_panelSetting->get(UKUI_PANEL_POSITION_KEY).toInt();
        } else {
            m_panelProperty.size = m_panelSetting->get(UKUI_PANEL_SIZE_KEY).toInt();
        }

        Q_EMIT panelPropertyChanged();
    }
}

void ScreenMonitor::layoutDirectionChangedSlot(Qt::LayoutDirection direction)
{
    if (m_direction == direction) return;
    m_direction = direction;
    Q_EMIT layoutDirectionChanged();
}

void ScreenMonitor::screenLockSlot()
{
    m_screenLockState = true;
    Q_EMIT screenLockStateChanged(m_screenLockState);
}

void ScreenMonitor::screenUnLockSlot()
{
    m_screenLockState = false;
    Q_EMIT screenLockStateChanged(m_screenLockState);
}

void ScreenMonitor::sessionToolsStateChangedSlot(bool visible)
{
    m_sessionToolsState = visible;
    Q_EMIT sessionToolsStateChanged(m_sessionToolsState);
}

QScreen *ScreenMonitor::getPrimaryScreen()
{
    return m_primaryScreen;
}

QRect ScreenMonitor::getGeometry()
{
    if (m_primaryScreen) {
        return m_primaryScreen->geometry();
    }
    return {};
}

Qt::LayoutDirection ScreenMonitor::getLayoutDirection()
{
    return m_direction;
}

qreal ScreenMonitor::screenRatio()
{
    qreal ratio = 1;
    if (!m_primaryScreen) {
        return ratio;
    }

    qreal height = 825 + (m_primaryScreen->devicePixelRatio() > 1 ? 8*m_primaryScreen->devicePixelRatio() : 8);

    if (m_primaryScreen->geometry().height() < height) {
        ratio = qreal(m_primaryScreen->geometry().height()) / height;
        if (ratio < 0.5) {
            ratio = 0.5;
        }
    }

    return ratio;
}
