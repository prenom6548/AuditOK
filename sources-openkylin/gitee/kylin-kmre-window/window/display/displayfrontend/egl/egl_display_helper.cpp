/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  MaChao    machao@kylinos.cn
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

#include "egl_display_helper.h"
#include "egl_display_utils.h"
#include "displaymanager/android_display/egldisplaywidget.h"

#include <sys/syslog.h>

EglDisplayHelper::EglDisplayHelper(DisplayWidget *widget, QObject* parent)
    : DisplayHelper(widget, parent),
      mCurrentOrientation(-1),
      mIsReadyForRender(false),
      mUtilsMutex(QMutex::Recursive),
      mDisplayWidget(nullptr)
{
    mDisplayUtils = std::make_shared<EglDisplayUtils>(widget, this);
    if (widget) {
        mDisplayWidget = dynamic_cast<EglDisplayWidget*>(widget);
    }
}

EglDisplayHelper::~EglDisplayHelper()
{
}

void EglDisplayHelper::lock()
{
    mUtilsMutex.lock();
}

void EglDisplayHelper::unlock()
{
    mUtilsMutex.unlock();
}

GLuint EglDisplayHelper::framebufferTextureId()
{
    if (!mDisplayUtils) {
        return 0;
    }

    return mDisplayUtils->framebufferTextureId();
}

QSize EglDisplayHelper::framebufferSize()
{
    if (!mDisplayUtils) {
        return QSize(0, 0);
    }

    return mDisplayUtils->framebufferSize();
}

void EglDisplayHelper::destroyDisplay()
{
    syslog(LOG_DEBUG, "[%s] Destroy Egl context.", __func__);
    if (mIsReadyForRender) {
        mDisplayUtils.reset();
        mIsReadyForRender = false;
    }
}

void EglDisplayHelper::update(int name, int width, int height, int stride, int bpp, int orientation)
{
    if (!mIsReadyForRender) {
        initDisplay();
    }

    if (mIsReadyForRender && mUpdateEnabled && mDisplayWidget) {
        if (importBuffer(name, width, height, stride, bpp, orientation) == 0) {
            drawBuffer();
        }
    }
}

void EglDisplayHelper::redraw()
{
    if (mIsReadyForRender && mUpdateEnabled && mDisplayWidget) {
        drawBuffer();
    }
}

void EglDisplayHelper::initDisplay()
{
    QMutexLocker lock(&mUtilsMutex);
    if (mDisplayUtils->initializeUtils()) {
        syslog(LOG_DEBUG, "[%s] EglDisplayUtils is initialized.", __func__);
        mIsReadyForRender = true;
    } else {
        syslog(LOG_ERR, "[%s] Failed to initialize EglDisplayUtils.", __func__);
    }
}

int EglDisplayHelper::importBuffer(int name, int width, int height, int stride, int bpp, int orientation)
{
    {
        QMutexLocker lock(&mUtilsMutex);
        mDisplayUtils->updateImage(name, width, height, stride, bpp);
    }

    if (mCurrentOrientation != orientation) {
        mCurrentOrientation = orientation;
        if (mDisplayWidget) {
            mDisplayWidget->setOrientation(orientation);
        }
        emit orientationChanged(mWidget->getDisplayId(), mCurrentOrientation);
    }

    return 0;
}

void EglDisplayHelper::drawBuffer()
{
    mDisplayUtils->drawDisplay();
}
