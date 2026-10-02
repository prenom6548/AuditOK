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

#ifndef WAYLANDMANAGER_H
#define WAYLANDMANAGER_H

#include "abstract-gamma-manager.h"
#include "gamma-color-info.h"

class WaylandManager: public AbstractGammaManager
{
    Q_OBJECT
public:
    WaylandManager();
public:
    ~WaylandManager();

    static WaylandManager *GammaManagerNew();
    virtual bool Start();
    virtual void Stop();

    /**
     * @brief setTemperature 设置屏幕色温
     * @param brightness
     * @return
     */
    virtual int setTemperature(int temp);

public Q_SLOTS:
    void doDisableKwinTimerOut();
private:

    /**
     * @brief disableNightColorWithOutSignal 禁用色温，避免下次切换x时与usd冲突
     */
    void disableNightColorWithOutSignal();
    void sendConfigChangedSignal(QStringList list);
    void syncColorSetToKwinWithSetSingleProp(int temp);
private:
    QDBusInterface *m_kwinItr;
    QTimer *m_disableKwin;
    static WaylandManager *m_gammaManager;
};

#endif // WAYLANDMANAGER_H
