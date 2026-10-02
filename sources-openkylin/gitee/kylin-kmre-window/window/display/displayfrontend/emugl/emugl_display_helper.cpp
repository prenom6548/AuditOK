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

#include "emugl_display_helper.h"
#include "displaymanager/android_display/displaywidget.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/syslog.h>
#include <unistd.h>

EmuGLDisplayHelper::EmuGLDisplayHelper(DisplayWidget* widget, QObject *parent)
    : DisplayHelper(widget, parent)
    , mCurrentOrientation(-1)
{
    mDisplayUtils = std::make_shared<EmuGLDisplayUtils>(widget->winId());
}

EmuGLDisplayHelper::~EmuGLDisplayHelper()
{
    
}

void EmuGLDisplayHelper::initDisplay()
{
    mIsReadyForRender = true;
}

void EmuGLDisplayHelper::destroyDisplay()
{
    mDisplayUtils->deleteWindowAttribute(mWidget->getDisplayId());
}

int EmuGLDisplayHelper::importBuffer(int nativeHandle, int width, int height, int stride, int bpp, int orientation)
{
    Q_UNUSED(stride);
    Q_UNUSED(bpp);

    mNativeHandle = nativeHandle;

    if (mCurrentOrientation != orientation) {
        mCurrentOrientation = orientation;
        emit orientationChanged(mWidget->getDisplayId(), mCurrentOrientation);
    }

    if (mWidth != width || mHeight != height) {
        mWidth = width;
        mHeight = height;
    }

    int displayId = mWidget->getDisplayId();
    QSize displaySize = mWidget->getWidgetRealSize();
    //syslog(LOG_DEBUG, "[%s] displayId = %d, mNativeHandle = %d, mWidth = %d, mHeight = %d, width = %d, height = %d", 
    //    __func__, displayId, mNativeHandle, mWidth, mHeight, displaySize.width(), displaySize.height());
    mDisplayUtils->setSupportDynamicSize(displayId, mNativeHandle, mWidth, mHeight, mWidget->isDDSSupported());
    mDisplayUtils->updateWindowAttribute(displayId, mNativeHandle, 
        displaySize.width(), displaySize.height(), orientation, false);
    
    return 0;
}

void EmuGLDisplayHelper::update(int name, int width, int height, int stride, int bpp, int orientation)
{
    if (mIsReadyForRender && mUpdateEnabled) {
        if (importBuffer(name, width, height, stride, bpp, orientation) != 0) {
            syslog(LOG_ERR, "[EmuGLDisplayHelper] importBuffer failed!");
        }
    }
}

void EmuGLDisplayHelper::redraw()
{
    int displayId = mWidget->getDisplayId();
    QSize displaySize = mWidget->getWidgetRealSize();

    if (mIsReadyForRender && mUpdateEnabled) {
        mDisplayUtils->updateWindowAttribute(displayId, mNativeHandle, 
            displaySize.width(), displaySize.height(), mCurrentOrientation, true);
    }
}

