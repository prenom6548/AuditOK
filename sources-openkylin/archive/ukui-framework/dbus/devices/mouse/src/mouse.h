/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2025 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _DEVICE_MOUSE_MANAGER_H_
#define _DEVICE_MOUSE_MANAGER_H_

#include <memory>
#include <QObject>
#include <QProcess>
#include <QMetaEnum>
#include <QMetaObject>
#include <QtDBus/QtDBus>
#include <QDBusInterface>
#include <QtCore/QCoreApplication>

#include "dbus_define.h"
#include "qgsettings.h"
#include "gsettings_manager.h"
#include "usd-common.h"
using namespace  UKUI;

using OnKeyChanged =  std::function<void()>;
class MouseManager :public QObject, QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.Framework.Devices.Mouse")
    DEFINE_PROPERTY(QString,    CursorTheme)
    DEFINE_PROPERTY(int,        LocatePointer)
    DEFINE_PROPERTY(double,     MotionAcceleration)
    DEFINE_PROPERTY(bool,       MouseAccel)
    DEFINE_PROPERTY(int,        WheelSpeed)
    DEFINE_PROPERTY(int,        CursorSize)
    DEFINE_PROPERTY(int,        DoubleClick)
    DEFINE_PROPERTY(bool,       LeftHanded)
    DEFINE_PROPERTY(bool,       MiddleButtonEnabled)
    DEFINE_PROPERTY(bool,       NaturalScroll)
public:
    explicit MouseManager(QObject *parent = nullptr);
    ~MouseManager();

public Q_SLOTS:

    int GetMouseCount();

    /**
     * @brief GetCursorTheme
     * 获取光标主题名称
     * @return
     */
    QString GetCursorTheme();

    /**
     * @brief GetLocatePointer
     * 获取定位指标方式
     * @return 0：不定位，1，按下ctrl定位，2，晃动鼠标定位
     */
    int GetLocatePointer();


    /**
     * @brief GetMotionAcceleration
     * 获取鼠标指针移动速度
     * @return
     */
    double GetMotionAcceleration();

    /**
     * @brief GetMouseAccel
     * 获取启用鼠标加速状态
     * @return 0：不加速，1：加速
     */
    bool GetMouseAccel();
    /**
     * @brief GetWheelSpeed
     * 获取滚轮速度
     * @return
     */
    int GetWheelSpeed();

    /**
     * @brief GetCursorSize
     * 获取光标大小
     * @return
     */
    int GetCursorSize();

    /**
     * @brief GetDoubleClick
     * 获取按两下速度
     * @return
     */
    int GetDoubleClick();
    /**
     * @brief GetLeftHanded
     * 获取是否为左手模式：
     * 鼠标主按键位置：false，在左，true在右
     * @return
     */
    bool GetLeftHanded();

    /**
     * @brief GetMiddleButtonEnabled
     * 模拟鼠标中间按键
     * @return
     */
    bool GetMiddleButtonEnabled();

//    /**
//     * @brief GetMotionThreshold
//     * @return
//     */
//    double GetMotionThreshold() const;

    /**
     * @brief GetNaturalScroll
     * 滚动方向
     * @return false:正向，  true:反向
     */
    bool GetNaturalScroll();

    // Setter methods
    void SetCursorTheme(const QString &cursorTheme);
    void SetLocatePointer(const int locatePointer);
    void SetMotionAcceleration(const double motionAcceleration);
    void SetMouseAccel(const bool mouseAccel);
    void SetWheelSpeed(const int wheelSpeed);
    void SetCursorSize(const int cursorSize);
    void SetDoubleClick(const int doubleClick);
    void SetLeftHanded(const bool leftHanded);
    void SetMiddleButtonEnabled(const bool middleButtonEnabled);
    void SetMotionThreshold(const double motionThreshold);
    void SetNaturalScroll(const bool naturalScroll);
private:
    enum MouseShowType {
        none = 0,
        CTRL,
        SHARK};

private Q_SLOTS:
    void onDeviceNumChanged();

Q_SIGNALS:
    void CursorThemeChanged(const QString &cursorTheme);
    void LocatePointerChanged(int locatePointer);
    void MotionAccelerationChanged(double motionAcceleration);
    void MouseAccelChanged(bool mouseAccel);
    void WheelSpeedChanged(int wheelSpeed);
    void CursorSizeChanged(int cursorSize);
    void DoubleClickChanged(int doubleClick);
    void LeftHandedChanged(bool leftHanded);
    void MiddleButtonEnabledChanged(bool middleButtonEnabled);
    void MotionThresholdChanged(double motionThreshold);
    void NaturalScrollChanged(bool naturalScroll);
    void MouseCountChanged(int count);

private:
    QHash<QString, OnKeyChanged> setChangedHandler();
    void initSignals();


private:
    QHash<QString, OnKeyChanged> m_settingsChangedHandlers;

    const QByteArray C_MOUSE_SCHEMA = "org.ukui.peripherals-mouse";

    // 加速度配置
    const QString C_ACCEL_PROFILE = "accel-profile";

    // 光标主题
    const QString C_CURSOR_THEME = "cursor-theme";

    // 拖动阈值
    const QString C_DRAG_THRESHOLD = "drag-threshold";

    // ctrl指针定位
    const QString C_LOCATE_POINTER = "locate-pointer";


    // 晃动指针定位
    const QString C_SHAKE_POINTER = "shake-cursor";

    // 运动加速度
    const QString C_MOTION_ACCELERATION = "motion-acceleration";

    // 鼠标加速度
    const QString C_MOUSE_ACCEL = "mouse-accel";

    // 滚轮速度
    const QString C_WHEEL_SPEED = "wheel-speed";

    // 光标大小
    const QString C_CURSOR_SIZE = "cursor-size";

    // 双击间隔
    const QString C_DOUBLE_CLICK = "double-click";

    // 左手模式
    const QString C_LEFT_HANDED = "left-handed";

    // 中键启用killall fcitx5
    const QString C_MIDDLE_BUTTON_ENABLED = "middle-button-enabled";

    // 运动阈值
    const QString C_MOTION_THRESHOLD = "motion-threshold";

    // 自然滚动
    const QString C_NATURAL_SCROLL = "natural-scroll";

    // 错误string
    const QString C_ERROR_STRING = "";

    // 错误数字
    const int C_ERROR_NUMBER = 0;

    GsettingsManager *m_pGsettingsManager = nullptr;
};

#endif

