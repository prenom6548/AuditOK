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

#ifndef DISPLAYWORKMANAGER_H
#define DISPLAYWORKMANAGER_H

#include <QObject>
#include <QThread>

#include "displaymanager/android_display/displaywidget.h"
#include "displayhelper.h"
#include "common.h"

class DisplayWorkManager : public QObject
{
    Q_OBJECT
public:
    explicit DisplayWorkManager(DisplayWidget* widget, QObject *parent = nullptr);
    virtual ~DisplayWorkManager();

    virtual void initialize() = 0;
    virtual void enableDisplayUpdate(bool enable) = 0;
    virtual void blurUpdate(int msecond) {}
    void forceUpdate(int displayId, int name, int width, int height, int stride, int bpp, int orientation);
    void forceRedraw();

signals:
    void orientationChanged(int displayId, int orientation);
    void exitRender();
    void update(int name, int width, int height, int stride, int bpp, int orientation);
    void redraw();

protected:
    QThread* mWorkerThread;
    DisplayHelper* mDisplayHelper;
    DisplayWidget* mWidget;

private:
    struct DisplayBufferAttri {
        uint32_t bo_name;
        int32_t width;
        int32_t height;
        int32_t stride;
        int32_t bpp;
        int32_t orientation;
        int32_t appStatus;
    };

    static QMutex mLastBufferHandleMapLock;
    static QMap<int, DisplayBufferAttri> mLastBufferHandleMap;

};

#endif // DISPLAYWORKMANAGER_H
