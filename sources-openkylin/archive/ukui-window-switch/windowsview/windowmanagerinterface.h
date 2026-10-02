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

#ifndef WINDOWMANAGERINTERFACE_H
#define WINDOWMANAGERINTERFACE_H

#include <QObject>
#include <QList>
#include <QRect>
#include <QVariantList>
#include <QString>
#include "screens-manager.h"

class QGSettings;

class WindowManagerInterface : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int logicalScreenCount READ logicalScreenCount NOTIFY screenCountChanged)
    Q_PROPERTY(bool isTabletMode READ isTabletMode NOTIFY tabletModeChanged)
    Q_PROPERTY(bool isTouchState READ isTouchState NOTIFY touchStateChanged)

public:
    static WindowManagerInterface *getInstance();
    ~WindowManagerInterface() = default;

    Q_INVOKABLE QString getCurrentLanguage() const;
    void setIsOpenGLBackend(bool isOpenGLBackend);

    int logicalScreenCount() const;
    bool isTabletMode() const;
    bool isTouchState() const;

    ScreensManager *getScreensManager();

Q_SIGNALS:
    void screenCountChanged();
    void screenSizeChanged();
    void tabletModeChanged(bool);
    void touchStateChanged(bool);
    void logicalScreenCountChanged();
    void screenAdd(Screen *screen);
    void screenRemove(Screen *screen);

private Q_SLOTS:
    void onMouseCountChanged(const int count);
    void onTouchpadCountChanged(const int count);
    void onScreenAdd(Screen *screen);
    void onScreenRemove(Screen *screen);

private:
    explicit WindowManagerInterface(QObject *parent = nullptr);
    bool isExtensionMode() const;
    bool validateScreenIndex(int screenIndex) const;
    void connectSignals();

    /**
     * @brief "TouchState"是否处于无鼠标触摸屏场景
     */
    void initTouchState();
    void updateTouchState();

    /**
     * @brief 异步获取指针类设备数量
     * @param 获取某类设备的dbus方法名称
     * @param callback
     */
    void asyncGetGenericPointerCount(const QString &destination, const QString &path,
                                     const QString &interface, const QString &method);

private:
    bool m_isOpenGLBackend = true;
    bool m_isTouchState = false;
    bool m_mouseAvailable = true;
    bool m_touchpadAvailable = true;
    bool m_flagNeedGetMouse = true;
    bool m_flagNeedGetTouchpad = true;
    ScreensManager *m_screenManager = nullptr;
};

#endif // WINDOWMANAGERINTERFACE_H
