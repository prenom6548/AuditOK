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

#include "displaybackend.h"
#include "windowmanager.h"
#include "kmredisplay.h"
#include "android_display/displaywidget.h"
#include "displaymanager.h"
#include "gles/gles_display_work_manager.h"
#include "emugl/emugl_display_work_manager.h"
#include "egl/egl_display_work_manager.h"
#include "sessionsettings.h"

#include <syslog.h>

#define DEFAULT_DELAY_MSECOND 1000

DisplayBackend::DisplayBackend() 
    : QObject(nullptr)
{
    mDirtyDisplays.clear();
    mDisplayList.clear();
    
    DisplayConnection::getInstance();
}

DisplayBackend::~DisplayBackend()
{
    DisplayConnection::destroy();
}

bool DisplayBackend::registerDisplay(DisplayManager *displayManager, DisplayWidget *displayWidget)
{
    if ((!displayManager) || (!displayWidget)) {
        return false;
    }
    
    sp<DisplayWorkManager> displayWorkManager;
    auto displayType = SessionSettings::getInstance().getDisplayType();
    if (displayType == DisplayType::TYPE_DRM) {
        if (SessionSettings::getInstance().windowUsePlatformWayland()) {
            displayWorkManager.reset(new EglDisplayWorkManager(displayWidget));
        } else {
            displayWorkManager.reset(new GLESDisplayWorkManager(displayWidget));
        }
    } 
    else if (displayType == DisplayType::TYPE_EMUGL) {
        displayWorkManager.reset(new EmuGLDisplayWorkManager(displayWidget));
    } 
    else {
        syslog(LOG_ERR, "[DisplayBackend][%s] Invalid display type:%d!", __func__, displayType);
    }

    if (displayWorkManager) {
        connect(displayWorkManager.get(), SIGNAL(orientationChanged(int, int)), displayManager, SLOT(onDisplayRotationChanged(int, int)));

        auto display = std::make_shared<KmreDisplay>(displayWidget, displayWorkManager);
        display->setDelayUpdateTime(getDelayUpdateTime(displayManager));

        if (display->connectDisplay(displayWidget)) {
            redrawMissedUpdating(display);
        }
        else {
            syslog(LOG_DEBUG, "[DisplayBackend][%s] Connect display later!", __func__);
        }

        lgm lk(mDisplayListLock);
        mDisplayList.emplace_back(display);

        return true;
    }

    return false;
}

bool DisplayBackend::connectDisplay(DisplayWidget *displayWidget)
{
    if (!displayWidget) {
        return false;
    }
    
    lgm lk(mDisplayListLock);
    for (const auto& display : mDisplayList) {
        if (display->connectDisplay(displayWidget)) {
            redrawMissedUpdating(display);
            return true;
        }
    }

    syslog(LOG_ERR, "[DisplayBackend][%s] Connect display: %d failed!", __func__, displayWidget->getDisplayId());
    return false;
}

void DisplayBackend::redrawMissedUpdating(sp<KmreDisplay> display)
{
    int displayId = display->getDisplayId();

    lgm lk(mUnupdatedDisplaysLock);
    if (mUnupdatedDisplays.count(displayId)) {
        syslog(LOG_INFO, "[DisplayBackend][%s] Update display(%d) due to missed updating before.", __func__, displayId);
        display->update(mUnupdatedDisplays[displayId].bo.name, 
                        mUnupdatedDisplays[displayId].bo.width, 
                        mUnupdatedDisplays[displayId].bo.height, 
                        mUnupdatedDisplays[displayId].bo.stride, 
                        mUnupdatedDisplays[displayId].bo.bpp, 
                        mUnupdatedDisplays[displayId].orientation);
        mUnupdatedDisplays.erase(displayId);
    }
}

void DisplayBackend::unregisterDisplay(int displayId)
{
    syslog(LOG_DEBUG, "[DisplayBackend][%s] Unregister display:%d", __func__, displayId);
    lgm lk(mDisplayListLock);
    for (const auto& display : mDisplayList) {
        if (display->getDisplayId() == displayId) {
            mDisplayList.remove(display);
            return;
        }
    }
}

sp<DisplayWorkManager> DisplayBackend::getWorkManagerForDisplay(int displayId)
{
    lgm lk(mDisplayListLock);
    for (const auto& display : mDisplayList) {
        if (display->getDisplayId() == displayId) {
            return display->getDisplayWorkManager();
        }
    }

    return nullptr;
}

void DisplayBackend::addDirtyDisplay(const char* name, uint32_t id, uint32_t bo_name, int32_t width, int32_t height, int32_t stride, int32_t bpp, int32_t orientation)
{
    //syslog(LOG_DEBUG, "[DisplayBackend][%s] Add dirty display:%d", __func__, id);
    VirtualDisplay d {
        name,
        id,
        orientation,
        {bo_name, width, height, stride, bpp},
    };
    lgm lk(mDirtyDisplaysLock);
    mDirtyDisplays.emplace_back(std::move(d));
}

void DisplayBackend::updateDisplays()
{
    //syslog(LOG_DEBUG, "[DisplayBackend][%s] Update all displays...", __func__);
    lgm lk(mDirtyDisplaysLock);
    for (auto display : mDirtyDisplays) {
        if (!updateDisplay(display.id, 
                            display.bo.name, 
                            display.bo.width, 
                            display.bo.height, 
                            display.bo.stride, 
                            display.bo.bpp, 
                            display.orientation)) {
            lgm lk(mUnupdatedDisplaysLock);
            mUnupdatedDisplays[display.id] = std::move(display);
        }
    }
    mDirtyDisplays.clear();
}

bool DisplayBackend::updateDisplay(int displayId, int name, int width, int height, int stride, int bpp, int orientation)
{
    //syslog(LOG_DEBUG, "[DisplayBackend][%s] Update display:%d", __func__, displayId);
    lgm lk(mDisplayListLock);
    for (const auto& display : mDisplayList) {
        if (display->getDisplayId() == displayId) {
            display->update(name, width, height, stride, bpp, orientation);
            return true;
        }
    }

    syslog(LOG_WARNING, "[DisplayBackend][%s] Displaywidget(%d) have not created yet! Update it later!", __func__, displayId);
    return false;
}

void DisplayBackend::enableDisplayUpdate(int displayId, bool enable)
{
    lgm lk(mDisplayListLock);
    for (const auto& display : mDisplayList) {
        if (display->getDisplayId() == displayId) {
            display->enableUpdate(enable);
            return;
        }
    }
}

void DisplayBackend::forceRedraw(int displayId)
{
    lgm lk(mDisplayListLock);
    for (const auto& display : mDisplayList) {
        if (display->getDisplayId() == displayId) {
            display->redraw();
            return;
        }
    }
}

void DisplayBackend::displayBlurUpdate(int displayId)
{
    lgm lk(mDisplayListLock);
    for (const auto& display : mDisplayList) {
        if (display->getDisplayId() == displayId) {
            display->blurUpdate(DEFAULT_DELAY_MSECOND);
            return;
        }
    }
}

// 延迟更新，解决在低性能机器上启动应用时出现黑屏或花屏的问题
int DisplayBackend::getDelayUpdateTime(DisplayManager *displayManager)
{
    int delayTime = 0;
    auto cpuArch = kmre::utils::getCpuArch();
    auto displayType = SessionSettings::getInstance().getDisplayType();

    if (displayType == DisplayType::TYPE_DRM) {
        delayTime += (cpuArch.startsWith("x86")) ? 0 : 1000;
    }
    else if (displayType == DisplayType::TYPE_EMUGL) {
        delayTime += (cpuArch.startsWith("x86")) ? 1000 : 2000;
    }
    else {
        delayTime += 2000;
    }

    if (displayManager->getInitialOrientation() == Qt::LandscapeOrientation) {
        delayTime += 1000;
    }

    syslog(LOG_DEBUG, "[DisplayBackend] Delay update time: '%d'", delayTime);
    return delayTime;
}