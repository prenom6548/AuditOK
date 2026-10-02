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

#ifndef WLCOMMANAGER_H
#define WLCOMMANAGER_H

#include "abstract-gamma-manager.h"
#include "gamma-color-info.h"
#include "gamma-manager-adaptor.h"

class WlcomManager:public AbstractGammaManager
{
    Q_OBJECT
public:
    WlcomManager();

public:
    ~WlcomManager();

    static WlcomManager *WlcomManagerNew();
    virtual bool Start();
    virtual void Stop();

    /**
     * @brief setTemperature 设置屏幕色温
     * @param brightness
     * @return
     */
    virtual int setTemperature(int temp);
public Q_SLOTS:
    void doScreensParamChanged(QString param);
    void doStartTimerOut();
private:
    void getScreenInfo(QString param);
    void disableNightColorWithOutSignal();
private:
    static WlcomManager *m_gammaManager;
    QDBusInterface *m_wlComItr;
    QDBusInterface *m_xrandrInterface;

    GmAdaptor   *m_pGmAdaptor;
    GmDbus      *m_pGmDbus;
    QTimer      m_startTimer;
    QStringList m_outputList;
    QString m_primaryOutputName;
};

#endif // WLCOMMANAGER_H
