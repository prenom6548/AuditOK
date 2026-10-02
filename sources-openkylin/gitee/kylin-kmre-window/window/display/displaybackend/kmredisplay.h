/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Alan Xie    xiehuijun@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <QObject>
#include <QTimer>
#include "typedef.h"

class DisplayWidget;
class DisplayWorkManager;

class KmreDisplay : public QObject
{
    Q_OBJECT
public:
    KmreDisplay(DisplayWidget* widget, kmre::sp<DisplayWorkManager> displayWorkManager);
    virtual ~KmreDisplay();

    int getDisplayId() { return mDisplayId; }
    bool connectDisplay(DisplayWidget* widget);
    kmre::sp<DisplayWorkManager> getDisplayWorkManager() { return mDisplayWorkManager; }
    void update(int name, int width, int height, int stride, int bpp, int orientation);
    void setDelayUpdateTime(int time) { mDelayUpdateTime = time; }
    void enableUpdate(bool enable);
    void redraw();
    void blurUpdate(int msecond);
    
signals:
    void sigStartDelayTimer();

private:
    void readyUpdate();

private:
    DisplayWidget* mWidget;
    kmre::sp<DisplayWorkManager> mDisplayWorkManager;
    int mDisplayId;
    int mDelayUpdateTime;
    bool mDisplayReady;
    bool mUpdateReady;
    bool mUpdateEnabled;
    bool mUpdateEnableLater;

    struct {
        int name = 0;
        int width = 0;
        int height = 0;
        int stride = 0;
        int bpp = 0;
        int orientation = 0;
    }mMissedDisplayInfo;
};