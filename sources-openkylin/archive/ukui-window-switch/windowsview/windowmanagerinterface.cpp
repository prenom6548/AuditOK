/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "windowmanagerinterface.h"

#include <algorithm>

#include <QSettings>
#include <QGuiApplication>
#include <QScreen>
#include <QQuickWindow>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <mutex>

const QString UKUI_FRAMEWORK_SERVICE = "org.ukui.Framework";
const QString UKUI_FRAMEWORK_MOUSE_PATH = "/org/ukui/Framework/Devices/Mouse";
const QString UKUI_FRAMEWORK_MOUSE_INTERFACE = "org.ukui.Framework.Devices.Mouse";
const QString UKUI_FRAMEWORK_TOUCHPAD_PATH = "/org/ukui/Framework/Devices/Touchpad";
const QString UKUI_FRAMEWORK_TOUCHPAD_INTERFACE = "org.ukui.Framework.Devices.Touchpad";

WindowManagerInterface *WindowManagerInterface::getInstance()
{
    static WindowManagerInterface s_interface;
    return &s_interface;
}

WindowManagerInterface::WindowManagerInterface(QObject *parent) : QObject(parent)
{
    m_screenManager = ScreensManager::instance();

    initTouchState();
    connectSignals();

    QList<Screen *> screenList = m_screenManager->screens();
    for (const auto &screen : screenList) {
        connect(screen, &Screen::geometryChanged, this, &WindowManagerInterface::screenSizeChanged);
    }
    emit screenCountChanged();
}

void WindowManagerInterface::setIsOpenGLBackend(bool isOpenGLBackend)
{
    m_isOpenGLBackend = isOpenGLBackend;
}

bool WindowManagerInterface::isExtensionMode() const
{
    QRect bounding;
    QList<Screen *> screensList = m_screenManager->screens();

    for (int i = 0; i < screensList.size(); i++) {
        bounding = bounding.united(screensList.at(i)->geometry());
    }
    for (int i = 0; i < screensList.size(); i++) {
        if (bounding == screensList.at(i)->geometry()) {
            return false;
        };
    }

    return true;
}

int WindowManagerInterface::logicalScreenCount() const
{
    const auto screenCount = m_screenManager->screens().size();
    if (screenCount == 1 || !isExtensionMode()) {
        return 1;
    }
    return screenCount;
}

bool WindowManagerInterface::isTabletMode() const
{
    // default to PC mode.
    bool tabletMode = false;
    QDBusMessage message = QDBusMessage::createMethodCall("com.kylin.statusmanager.interface",
                                                          "/",
                                                          "com.kylin.statusmanager.interface",
                                                          "get_current_tabletmode");
    QDBusMessage response = QDBusConnection::sessionBus().call(message);
    if (response.type() == QDBusMessage::ReplyMessage) {
        tabletMode = response.arguments().takeFirst().toBool();
    } else {
        qDebug() << "get tablet mode failed " << response.type();
    }
    return tabletMode;
}

bool WindowManagerInterface::isTouchState() const
{
    return m_isTouchState;
}

ScreensManager *WindowManagerInterface::getScreensManager()
{
    return m_screenManager;
}

QString WindowManagerInterface::getCurrentLanguage() const
{
    // default to "Chinese".
    QLocale locale = QLocale::system();
    QString language = "Chinese";
    if (locale.language() == QLocale::Chinese) {
        language = "Chinese";
    } else if (locale.language() == QLocale::Tibetan) {
        language = "Tibetan";
    }
    return language;
}

bool WindowManagerInterface::validateScreenIndex(int screenIndex) const
{
    return screenIndex >= 0 && screenIndex < logicalScreenCount();
}

void WindowManagerInterface::connectSignals()
{
    connect(m_screenManager, &ScreensManager::screenAdded, this, &WindowManagerInterface::onScreenAdd);
    connect(m_screenManager, &ScreensManager::screenRemoved, this, &WindowManagerInterface::onScreenRemove);

    QDBusConnection::sessionBus().connect(UKUI_FRAMEWORK_SERVICE,
                                          UKUI_FRAMEWORK_MOUSE_PATH,
                                          UKUI_FRAMEWORK_MOUSE_INTERFACE,
                                          "MouseCountChanged",
                                          this,
                                          SLOT(onMouseCountChanged(const int)));
    QDBusConnection::sessionBus().connect(UKUI_FRAMEWORK_SERVICE,
                                          UKUI_FRAMEWORK_TOUCHPAD_PATH,
                                          UKUI_FRAMEWORK_TOUCHPAD_INTERFACE,
                                          "TouchpadChanged",
                                          this,
                                          SLOT(onTouchpadCountChanged(const int)));
    QDBusConnection::sessionBus().connect("com.kylin.statusmanager.interface",
                                          "/",
                                          "com.kylin.statusmanager.interface",
                                          "mode_change_signal",
                                          this,
                                          SIGNAL(tabletModeChanged(bool)));
}

void WindowManagerInterface::onMouseCountChanged(const int count)
{
    m_flagNeedGetMouse = false;
    if (count == -1) {
        // 接口返回 -1 代表获取设备数量失败
        qDebug() << UKUI_FRAMEWORK_SERVICE << "mouse count reply -1, keep default";
        return;
    }
    m_mouseAvailable = count > 0 ? true : false;
    updateTouchState();
}

void WindowManagerInterface::onTouchpadCountChanged(const int count)
{
    m_flagNeedGetTouchpad = false;
    if (count == -1) {
        // 接口返回 -1 代表获取设备数量失败
        qDebug() << UKUI_FRAMEWORK_SERVICE << "touchpad count reply -1, keep default";
        return;
    }
    m_touchpadAvailable = count > 0 ? true : false;
    updateTouchState();
}

void WindowManagerInterface::initTouchState()
{
    asyncGetGenericPointerCount(UKUI_FRAMEWORK_SERVICE,
                                UKUI_FRAMEWORK_MOUSE_PATH,
                                UKUI_FRAMEWORK_MOUSE_INTERFACE,
                                "GetMouseCount");
    asyncGetGenericPointerCount(UKUI_FRAMEWORK_SERVICE,
                                UKUI_FRAMEWORK_TOUCHPAD_PATH,
                                UKUI_FRAMEWORK_TOUCHPAD_INTERFACE,
                                "GetTouchpadCount");
}

void WindowManagerInterface::updateTouchState()
{
    bool currentState = !(m_mouseAvailable || m_touchpadAvailable);
    if (m_isTouchState == currentState) {
        return;
    }
    m_isTouchState = currentState;
    emit touchStateChanged(m_isTouchState);
}

void WindowManagerInterface::asyncGetGenericPointerCount(const QString &destination, const QString &path,
                                                         const QString &interface, const QString &method)
{
    QDBusMessage message = QDBusMessage::createMethodCall(destination, path, interface, method);
    QDBusPendingCall pendingCall = QDBusConnection::sessionBus().asyncCall(message);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pendingCall);
    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, this, [method, this](QDBusPendingCallWatcher *w) {
        QDBusPendingReply<int> reply = *w;
        w->deleteLater();
        if (!reply.isValid()) {
            qDebug() << "Error:" << reply.error().message();
            return;
        }

        if (method.contains("Mouse") && this->m_flagNeedGetMouse) {
            this->onMouseCountChanged(reply.value());
        } else if (method.contains("Touchpad") && this->m_flagNeedGetTouchpad) {
            this->onTouchpadCountChanged(reply.value());
        }
    });
}

void WindowManagerInterface::onScreenAdd(Screen *screen)
{
    if (screen != nullptr) {
        connect(screen, &Screen::geometryChanged, this, &WindowManagerInterface::screenSizeChanged);
    }
    emit screenAdd(screen);
    emit screenCountChanged();
}

void WindowManagerInterface::onScreenRemove(Screen *screen)
{
    if (screen != nullptr) {
        disconnect(screen, &Screen::geometryChanged, this, &WindowManagerInterface::screenSizeChanged);
    }
    emit screenRemove(screen);
    emit screenCountChanged();
}
