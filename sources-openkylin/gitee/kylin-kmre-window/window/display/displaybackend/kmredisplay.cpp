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

#include "kmredisplay.h"
#include <sys/syslog.h>
#include <QTimer>
#include "android_display/displaywidget.h"
#include "displayworkmanager.h"


using namespace kmre;
KmreDisplay::KmreDisplay(DisplayWidget* widget, sp<DisplayWorkManager> displayWorkManager)
    : mWidget(widget)
    , mDisplayWorkManager(displayWorkManager)
    , mDisplayId(-1)
    , mDelayUpdateTime(0)
    , mDisplayReady(false)
    , mUpdateReady(false)
    , mUpdateEnabled(false)
    , mUpdateEnableLater(false)
{
    connect(this, &KmreDisplay::sigStartDelayTimer, this, [this] {
        QTimer::singleShot(mDelayUpdateTime, this, [this] {
            readyUpdate();
            update(mMissedDisplayInfo.name, mMissedDisplayInfo.width, mMissedDisplayInfo.height, 
                    mMissedDisplayInfo.stride, mMissedDisplayInfo.bpp, mMissedDisplayInfo.orientation);
        });
    }, Qt::QueuedConnection);
}

KmreDisplay::~KmreDisplay()
{
    syslog(LOG_DEBUG, "[KmreDisplay]Destory KmreDisplay: %d", mDisplayId);
}

bool KmreDisplay::connectDisplay(DisplayWidget* widget)
{
    if (!widget) {
        return false;
    }

    int displayId = widget->getDisplayId();
    if ((displayId >= 0) && (widget == mWidget)) {
        mDisplayId = displayId;
        syslog(LOG_DEBUG, "[KmreDisplay] Connected display: %d", displayId);
        return true;
    }

    return false;
}

void KmreDisplay::update(int name, int width, int height, int stride, int bpp, int orientation)
{
    if (mDisplayId >= 0) {
        if (!mDisplayReady) {// first update
            mDisplayReady = true;

            if (mDelayUpdateTime > 0) {
                syslog(LOG_DEBUG, "[KmreDisplay]Delay updating for display '%d', delay time = %d, tid = %d", 
                        mDisplayId, mDelayUpdateTime, gettid());
                emit sigStartDelayTimer();// start delay timer at main thread
            }
            else {
                readyUpdate();
            }
        }

        if (mUpdateReady) {
            if (mUpdateEnabled && (name > 0)) {
                mDisplayWorkManager->forceUpdate(mDisplayId, name, width, height, stride, bpp, orientation);
            }
        }
        else {
            mMissedDisplayInfo = {name, width, height, stride, bpp, orientation};
        }
    }
}

void KmreDisplay::readyUpdate()
{
    syslog(LOG_DEBUG, "[KmreDisplay]Display '%d' ready, goto updating.", mDisplayId);
    mUpdateReady = true;
    if (mUpdateEnableLater) {
        mUpdateEnableLater = false;
        mUpdateEnabled = true;
        mDisplayWorkManager->enableDisplayUpdate(true);
    }
}

void KmreDisplay::blurUpdate(int msecond)
{
    if (mUpdateReady && mUpdateEnabled) {
        //syslog(LOG_DEBUG, "[KmreDisplay]Blur update '%d'", mDisplayId);
        mDisplayWorkManager->blurUpdate(msecond);
    }
}

void KmreDisplay::enableUpdate(bool enable)
{
    if (enable) {
        if (mUpdateReady) {
            mUpdateEnabled = true;
            mUpdateEnableLater = false;
            //syslog(LOG_DEBUG, "[KmreDisplay]Enable update '%d'", mDisplayId);
            mDisplayWorkManager->enableDisplayUpdate(true);
        }
        else {
            mUpdateEnableLater = true;
            //syslog(LOG_DEBUG, "[KmreDisplay]Enable update '%d' later", mDisplayId);
        }
    }
    else {
        mUpdateEnabled = false;
        mUpdateEnableLater = false;
        //syslog(LOG_DEBUG, "[KmreDisplay]Disable update '%d'", mDisplayId);
        mDisplayWorkManager->enableDisplayUpdate(false);
    }    
}

void KmreDisplay::redraw()
{
    if (mUpdateReady && mUpdateEnabled) {
        //syslog(LOG_DEBUG, "[KmreDisplay]Redraw '%d'", mDisplayId);
        mDisplayWorkManager->forceRedraw();
    }
}