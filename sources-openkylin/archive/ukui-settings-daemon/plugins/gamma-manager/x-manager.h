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

#ifndef XMANAGER_H
#define XMANAGER_H

#include "abstract-gamma-manager.h"
#include "gamma-color-info.h"
#include "gamma-manager-adaptor.h"

class xManager :public AbstractGammaManager
{
    Q_OBJECT
public:
    xManager();
    ~xManager();
    static xManager *GammaManagerNew();
    virtual bool Start();
    virtual void Stop();
    /**
     * @brief setPrimaryBrightness 设置主屏背光
     * @param brightness 背光值
     * @return
     */
    virtual int setPrimaryBrightness(int brightness);

    /**
     * @brief getPrimaryBrightness 设置主屏背光值
     * @param brightness
     * @return
     */
    virtual int getPrimaryBrightness();


    /**
     * @brief setTemperature 设置屏幕色温
     * @param brightness
     * @return
     */
    virtual int setTemperature(int temp);

    /**
     * @brief enablePrimarySignal 使能主屏背光改变的信号
     * @param brightness
     * @return
     */
    virtual int enablePrimarySignal(int state);


    /**
     * @brief getScreensInfo 获取屏幕信息
     * @param brightness
     * @return
     */
    OutputGammaInfo getScreensInfo();

    /**
     * @brief getScreensInfoList 获取所有屏幕的gamma配置信息
     * @param brightness
     * @return
     */
    OutputGammaInfoList getScreensInfoList();

public Q_SLOTS:
    /**
     * @brief doSetScreenBrightness
     * @param outputName
     * @param outputBrightness
     */
    void doScreenBrightnessChanged(QString outputName, int outputBrightness);
private:
    /**
     * @brief setBrightness 设置所有显示器的亮度
     * @param value
     */
    virtual void setBrightness(const uint value);
private:
    static xManager *m_gammaManager;
    GmAdaptor   *m_pGmAdaptor;
    GmDbus      *m_pGmDbus;
    GmWorkThread    *m_pGmThread;
};

#endif // XMANAGER_H
