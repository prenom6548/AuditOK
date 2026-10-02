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

#ifndef DISPLAYHELPER_H
#define DISPLAYHELPER_H

#include <QObject>
#include <mutex>
#include <atomic>

class DisplayWidget;

class DisplayHelper : public QObject
{
    Q_OBJECT
public:
    DisplayHelper(DisplayWidget* widget, QObject *parent = 0);
    virtual ~DisplayHelper();

    bool setDisplaySize(int width, int height);
    bool isReadyForRender() {return mIsReadyForRender;}
    void enableUpdate(bool enable) {mUpdateEnabled = enable;}
    void enableBlur(bool enable) {mBlurEnabled = enable;}
    bool blurEnabled() { return mBlurEnabled; }

protected:
    virtual int importBuffer(int name, int width, int height, int stride, int bpp, int orientation) = 0;
    virtual void initDisplay() = 0;
    virtual void destroyDisplay() = 0;
    virtual void redraw() = 0;
    virtual void update(int name, int width, int height, int stride, int bpp, int orientation) = 0;

signals:
    void orientationChanged(int displayId, int orientation);

protected:
    uint32_t mNativeHandle;
    std::atomic_bool mUpdateEnabled;
    std::atomic_bool mBlurEnabled;
    bool mIsReadyForRender;
    DisplayWidget *mWidget;
};

#endif // DISPLAYHELPER_H
