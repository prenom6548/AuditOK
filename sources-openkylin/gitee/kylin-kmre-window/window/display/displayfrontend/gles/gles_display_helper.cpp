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

#include "gles_display_helper.h"
#include "displaymanager/android_display/displaywidget.h"
#include "gles_display_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/syslog.h>
#include <unistd.h>

GLESDisplayHelper::GLESDisplayHelper(DisplayWidget* widget, QObject *parent)
    : DisplayHelper(widget, parent)
    , mCurrentOrientation(-1)
    , mTryInitCounter(0)
    , mWid(widget->winId()) // fixed bug#228265, the 'winId' must get at this moment
{
    mDisplayUtils = std::make_shared<GLESDisplayUtils>();
}

GLESDisplayHelper::~GLESDisplayHelper()
{
    
}

void GLESDisplayHelper::initDisplay()
{
    if (mTryInitCounter < 3) {
        if (mDisplayUtils->initGles(mWid)) {// fixed bug#228265, will crash if getting 'winId' at this moment on some platform, why ?
            syslog(LOG_DEBUG, "[%s] Initialize GLES succeed.", __func__);
            mIsReadyForRender = true;
        }
        else {
            syslog(LOG_ERR, "[%s] Failed to initialize GLES!", __func__);
            mDisplayUtils->destroyGles();
        }
        ++mTryInitCounter;
    }
}

void GLESDisplayHelper::destroyDisplay()
{
    syslog(LOG_DEBUG, "[%s] Destroy GLES context.", __func__);
    if (mIsReadyForRender) {
        mDisplayUtils.reset();// delete GLES
    }
}

int GLESDisplayHelper::importBuffer(int name, int width, int height, int stride, int bpp, int orientation)
{
    mImageWidth = width;
    mImageHeight = height;
    mDisplayUtils->updateImage(name, width, height, stride, bpp);

    if (mCurrentOrientation != orientation) {
        mCurrentOrientation = orientation;
        mDisplayUtils->setOrientation(mCurrentOrientation);
        emit orientationChanged(mWidget->getDisplayId(), mCurrentOrientation);
    }

    return 0;
}

void GLESDisplayHelper::drawBuffer()
{
    if (mWidget->isDDSSupported()) {
        QSize widgetSize = mWidget->size();

        if (mBlurEnabled) {
            QSize minSize = mWidget->getWidgetMinSize();
            if (minSize != QSize(0, 0)) {
                widgetSize = minSize;
            }
        }

        mDisplayUtils->enableBlur(mBlurEnabled);
        // syslog(LOG_DEBUG, "[%s] widget size: width = %d, height = %d, mCurrentOrientation = %d", 
        //     __func__, widgetSize.width(), widgetSize.height(), mCurrentOrientation);
        mDisplayUtils->updateShowRegion(mImageWidth, mImageHeight, widgetSize.width(), widgetSize.height());
    }

    QSize displaySize = mWidget->getWidgetRealSize();
    // syslog(LOG_DEBUG, "[%s] widget real size: width = %d, height = %d, mCurrentOrientation = %d", 
    //     __func__, displaySize.width(), displaySize.height(), mCurrentOrientation);
    mDisplayUtils->drawDisplay(displaySize.width(), displaySize.height());
}

void GLESDisplayHelper::update(int name, int width, int height, int stride, int bpp, int orientation)
{
    if (!mIsReadyForRender) {
        initDisplay();
    }

    if (mIsReadyForRender && mUpdateEnabled) {
        if (importBuffer(name, width, height, stride, bpp, orientation) == 0 ) {
            drawBuffer();
        }
    }
}

void GLESDisplayHelper::redraw()
{
    if (mIsReadyForRender && mUpdateEnabled) {
        drawBuffer();
    }
}

