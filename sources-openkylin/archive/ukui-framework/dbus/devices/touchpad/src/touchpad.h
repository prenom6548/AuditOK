/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2024 KylinSoft Co., Ltd.
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

#ifndef __DEVICES_TOUCHPAD_H__
#define __DEVICES_TOUCHPAD_H__

#include <memory>
#include <QObject>
#include <QProcess>
#include <QMetaEnum>
#include <QMetaObject>
#include <QtDBus/QtDBus>
#include <QDBusInterface>
#include <QtCore/QCoreApplication>
#include "usd-common.h"
#include "qgsettings.h"
#include "gsettings_manager.h"

using namespace  UKUI;

#ifndef DEFINE_PROPERTY
#define DEFINE_PROPERTY(type, name) Q_PROPERTY(type name READ Get##name WRITE Set##name NOTIFY name##Changed)
#endif

using OnKeyChanged =  std::function<void()>;
class TouchpadManager : public QObject  ,QDBusContext{

    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.Framework.Devices.Touchpad")
    DEFINE_PROPERTY(bool, TouchpadEnabled)
    DEFINE_PROPERTY(bool, DisableOnExternalMouse)
    DEFINE_PROPERTY(double, MotionAcceleration)
    DEFINE_PROPERTY(bool, DisableWhileTyping)
    DEFINE_PROPERTY(bool, TapToClick)
    DEFINE_PROPERTY(bool, NaturalScroll)
    DEFINE_PROPERTY(int, ScrollType)
public:
    explicit TouchpadManager(QObject *parent = nullptr);
    ~TouchpadManager();
    enum ScrollType {
        none = 0,
        edge_scroll,
        two_finger_scroll};

public Q_SLOTS:
    /**
     * @brief setTouchpadEnabled
     * @param enabled
     * 0：禁用触摸板，1：开启触摸板
     * @return
     */
    void SetTouchpadEnabled(bool enabled);

    /**
     * @brief setDisableOnExternalMouse
     * @param disabled
     * 0：关闭插入鼠标禁用触摸板，1：开启插入鼠标禁用触摸板
     * @return
     */

    void SetDisableOnExternalMouse(bool disabled);

    /**
     * @brief setMotionAcceleration
     * @param acceleration
     * @return
     */
    void SetMotionAcceleration(double acceleration);

    /**
     * @brief setDisableWhileTyping
     * @param disabled
     * 0：关闭打字时禁用触摸板， 1：开启打字时禁用触摸板
     * @return
     */
    void SetDisableWhileTyping(bool disabled);

    /**
     * @brief setTapToClick
     * @param enabled
     * 0：关闭轻触点击，1：开启轻触点击
     * @return
     */
    void SetTapToClick(bool enabled);

    /**
     * @brief setNaturalScroll
     * @param enabled
     * 0：不跟随手指滚动，1：跟随手指滚动
     * @return
     */
    void SetNaturalScroll(bool enabled);


    /**
     * @brief setScrollType
     * @param type 0：无滚动 1：边界滚动 2：中间区域滚动
     * @return
     */
    void SetScrollType(int type);

    /**
     * @brief setTouchpadEnabled
     * @param enabled
     * 0：禁用触摸板，1：开启触摸板
     * @return
     */
    bool GetTouchpadEnabled();

    /**
     * @brief setDisableOnExternalMouse
     * @param disabled
     * @return
     */
    bool GetDisableOnExternalMouse();

    /**
     * @brief setMotionAcceleration
     * @param acceleration
     * 0：关闭插入鼠标禁用触摸板，1：开启插入鼠标禁用触摸板
     * @return
     */
    double GetMotionAcceleration();

    /**
     * @brief setDisableWhileTyping
     * @param disabled
     * 0：关闭打字时禁用触摸板， 1：开启打字时禁用触摸板
     * @return
     */
    bool GetDisableWhileTyping();

    /**
     * @brief setTapToClick
     * @param enabled
     * 0：关闭轻触点击，1：开启轻触点击
     * @return
     */
    bool GetTapToClick();

    /**
     * @brief setNaturalScroll
     * @param enabled
     * 0：不跟随手指滚动，1：跟随手指滚动
     * @return
     */
    bool GetNaturalScroll();

    /**
     * @brief setScrollType
     * @param type 0：无滚动 1：边界滚动 2：中间区域滚动
     * @return
     */
    int GetScrollType();

    /**
     * @brief GetTouchpadCount
     * @return
     */
    int GetTouchpadCount();
private Q_SLOTS:
    void onTouchpadGsettingsChanged(QString key);
    void onDeviceNumChanged();

private:
    QHash<QString, OnKeyChanged> setChangedHandler();
    void setTouchpadGSetting(const QString &key, const QVariant &value);
    QVariant getGSetting(const QString &key);
    bool isEdgeScroll();
    bool isTwoFingerScroll();
    bool isNoneScroll();

    void setEdgeScroll(bool enable);
    void setTwoScroll(bool enable);
    void emitScrollTypeChanged();
    template <typename...Args>
    QDBusMessage callInputDbus(const QString &method, Args &&...args);

    void initSignals();


Q_SIGNALS:
    /**
     * @brief EnableChanged 触摸板使能状态改变信号
     */
    void TouchpadEnabledChanged(bool);
    bool DisableOnExternalMouseChanged(bool);
    bool MotionAccelerationChanged(double);
    bool DisableWhileTypingChanged(bool);
    bool TapToClickChanged(bool);
    bool NaturalScrollChanged(bool);
    bool ScrollTypeChanged(int);
    void TouchpadCountChanged(int count);
private:
    QHash<QString, OnKeyChanged> m_settingsChangedHandlers;
    const QByteArray C_TOUCHPAD_SCHEMA          = "org.ukui.peripherals-touchpad";
    const QString C_TOUCHPAD_ENABLE             = "touchpad-enabled";
    const QString C_DISABLE_ON_EXTERNAL_MOUSE   = "disable-on-external-mouse";
    const QString C_DISABLE_WHILE_TYPING        = "disable-while-typing";
    const QString C_MOTION_ACCELERATION         = "motion-acceleration";
    const QString C_TAP_TO_CLICK                = "tap-to-click";
    const QString C_NATURAL_SCROLL              = "natural-scroll";

    const QString C_VERTICAL_EDGE_SCROLLING     = "vertical-edge-scrolling";
    const QString C_HORIZONTAL_EDGE_SCROLLING   = "horizontal-edge-scrolling";

    const QString C_VERTICAL_TWO_FINGER_SCROLLING = "vertical-two-finger-scrolling";
    const QString C_HORIZONTAL_TWO_FINGER_SCROLLING = "horizontal-two-finger-scrolling";
    GsettingsManager *m_pGsettingsManager = nullptr;

    // 错误数字
    const int C_ERROR_NUMBER = 0;
};

#endif
