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

#ifndef ABSTRACTGAMMAMANAGER_H
#define ABSTRACTGAMMAMANAGER_H
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDBusInterface>
#include <QObject>
#include <QTimer>

#include "clib-syslog.h"
#include "usd_base_class.h"
#include "QGSettings/qgsettings.h"
#include "clock-skew-notifier.h"
#include "plugin-manager-interface.h"

#include "gamma-manager-gtkconfig.h"
#include "gamma-manager-define.h"
#include "gamma-manager-dbus.h"
#include "gamma-manager-thread.h"
#include "gamma-manager-helper.h"
#include "gamma-manager-location.h"

#ifdef Bool
#undef Bool
#endif

class AbstractGammaManager: public ManagerInterface
{
    Q_OBJECT
public:
    AbstractGammaManager();
    ~AbstractGammaManager();
    void init(bool isWayland);

    /**
     * @brief destroyQobject 销毁对象
     * @param object
     */
    void destroyQobject(QObject *object);

    /**
     * @brief setTemperature 设置屏幕色温
     * @param brightness
     * @return
     */
    virtual int setTemperature(int temp)  = 0;

    /**
     * @brief gammaReckeck 检查重置色温
     * @param key
     */
    void gammaReckeck(QString key);

private:
    /**
     * @brief setPrimaryBrightness 设置主屏背光
     * @param brightness 背光值
     * @return
     */
    virtual int setPrimaryBrightness(int brightness) {return 0;}

    /**
     * @brief getPrimaryBrightness 设置主屏背光值
     * @param brightness
     * @return
     */
    virtual int getPrimaryBrightness()  {return 0;}


    /**
     * @brief enablePrimarySignal 使能主屏背光改变的信号
     * @param brightness
     * @return
     */
    virtual int enablePrimarySignal(int state)  {return 0;}

    /**
     * @brief doSetScreenBrightness
     * @param outputName
     * @param outputBrightness
     */
    virtual void doScreenBrightnessChanged(QString outputName, int outputBrightness) {}



    /**
     * @brief checkEyeCareMode 检查护眼模式，开护眼关闭色温，开色温关护眼
     * @param key
     */
    void checkEyeCareMode(QString key);

public Q_SLOTS:

    /**
     * @brief doQtSettingsChanged 处理主题跳变
     * @param setKey
     */
    void doQtSettingsChanged(QString key);

    /**
     * @brief doColorSettingsChanged 处理色温gsettings的changed
     * @param setKey
     */
    void doColorSettingsChanged(QString key);

    /**
     * @brief doclockSkewedChanged 处理时间跳变
     * @param value
     */
    void doclockSkewedChanged(QString value);

    /**
     * @brief setDarkMode 设置夜间模式，自己设置夜间模式时需要block信号。
     * @param darkMode
     */
    void setDarkMode(bool darkMode);

    /**
     * @brief isFracDayBetween value 是否处于 start end的时间段内
     * @param value
     * @param start
     * @param end
     * @return
     */
    bool isFracDayBetween (double  value, double start, double end);

    /**
     * @brief getFracTimeFromDt QTime转为double
     * @param dt
     * @return
     */
    double getFracTimeFromDt(QTime dt);

    /**
     * @brief linearInterpolate 差值计算va11，val2在factor时的取值，渐变色温
     * @param val1
     * @param val2
     * @param factor
     * @return
     */
    double linearInterpolate(double val1, double val2, double factor);

    /**
     * @brief hourMinuteToDouble hour,minute 转double，感觉与getFracTimeFromDt冲突了。
     * @param hour
     * @param minute
     * @return
     */
    double hourMinuteToDouble(int hour, int minute);


    /**
     * @brief getEyeCareDate 获取护眼模式的数据
     * @param temperature  护眼色温
     * @param interpolateStart 渐变开始
     * @param scheduleFrom  开始时间
     * @param scheduleTo 截止时间
     */
    void getEyeCareDate(int& temperature, int& interpolateStart, double& scheduleFrom, double& scheduleTo);

    void doCheckTimeout();
protected:
    QGSettings *m_pColorSettings;
    QGSettings *m_pQtSettings;
    QGSettings *m_pGtkSettings;

    QTimer      *m_pCheckTimer;
    GmLocation  *m_pGmLocation;
    UkuiGtkConfig *m_pukuiGtkConfig;
    int  m_cachedTemperature;

    bool m_inDark;
    bool m_darkModeChangedBySelf;

    USD::ClockSkewNotifier *m_pclockNotifier;
};

#endif // ABSTRACTGAMMAMANAGER_H
