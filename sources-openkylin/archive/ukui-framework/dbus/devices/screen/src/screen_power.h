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

#ifndef __SCREEN_POWER_H__
#define __SCREEN_POWER_H__

#include "screen_abstract.h"

class ScreenBrightNode: public  ScreenAbstract
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "ukui.framework.devices.screen")
public:

    explicit ScreenBrightNode(QObject *parent = nullptr);
    ~ScreenBrightNode();

public:
    /**
     * @brief setScreenBrightness 设置显示器亮度，需要先获取所有显示器的亮度再使用。
     * @param name 显示器名称
     * @param screenBrightness 指定亮度
     * @return
     */
     AbstractResult setScreenBrightness(QString name, uint screenBrightness);

    /**
     * @brief getScreenBrightness 获取显示器的亮度
     * @param name
     * @return 指定显示器的亮度
     */
     AbstractResult getScreenBrightness(QString name);

    /**
     * @brief setAllScreenBrightness 分别设置显示器的亮度
     * @param screenMap 显示器亮度与其对应的显示器亮度
     * @return 成功标记
     */
     AbstractResult setScreensBrightness(OutputGammaInfoList outputsInfo);

    /**
     * @brief setAllScreenSameBrightness 设置所有显示为同一个亮度
     * @param brighenss 全部亮度
     * @return 与参数一致
     */
     AbstractResult setAllScreenSameBrightness(uint brighenss);

    /**
     * @brief getAllScreenBrightness
     * @return 与参数一致
     */
     OutputGammaInfoList getAllScreenBrightness();

     /**
      * @brief getPrimaryScreenBrightness
      * @return 主屏亮度
      */
     uint getPrimaryScreenBrightness();
private:
     QHash<QString, OnKeyChanged> PowerChangedHandler();

private:
     GsettingsManager *m_pPowerGsettingsManager = nullptr;
     QHash<QString, OnKeyChanged> m_powerSettingsChangedHandlers;
};

#endif
