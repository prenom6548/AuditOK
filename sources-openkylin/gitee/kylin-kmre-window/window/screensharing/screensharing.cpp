/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 * Alan Xie    xiehuijun@kylinos.cn
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

#include "windowmanager.h"
#include "sharingselectwindow.h"
#include "maskwindow.h"
#include "screensharing.h"
#include "utils.h"
#include "sessionsettings.h"
#include <syslog.h>

using DisplayPlatform = SessionSettings::DisplayPlatform;

ScreenSharing::ScreenSharing()
    : mIsSharingSupported(false)
    , mSharingScreenNum(0)
    , mSharingStatus(SharingStatus::eNotReady)
    , mGrabberHelper(nullptr)
    , mMaskWindow(nullptr)
    , mSelectWindow(nullptr)
{
    checkScreenSharing();
    
    if (mIsSharingSupported) {
        std::thread([&] () {
            syslog(LOG_DEBUG, "Start screen grab thread...");
            mGrabberHelper = std::make_unique<kmre::GrabberHelper>();
            mGrabberHelper->doGrab(); 
            // grabber exited
            std::lock_guard<std::mutex> lk(mGrabberHelperMtx);
            mGrabberHelper.reset();
        }).detach();

        //atexit(exitGrabThread);
    }
}

ScreenSharing::~ScreenSharing()
{
    if (mIsSharingSupported) {
        std::unique_lock<std::mutex> lk(mGrabberHelperMtx);
        if (mGrabberHelper) {
            mGrabberHelper->exitGrab();
            lk.unlock();
            // sleep for grab thread exit
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}


void ScreenSharing::addBo(uint32_t name)
{
    std::lock_guard<std::mutex> lk(mGrabberHelperMtx);
    if (mGrabberHelper) {
        mGrabberHelper->AddBoOwnedByCaptureScreen(name);
    }
}

int32_t ScreenSharing::grabNextBo(uint32_t at_name, uint32_t name, int32_t width, int32_t height, int32_t stride, int32_t ver)
{
    std::lock_guard<std::mutex> lk(mGrabberHelperMtx);
    if (mGrabberHelper) {
        return mGrabberHelper->NextBo(at_name, name, width, height, stride, ver);
    }
    return -1;
}

int32_t ScreenSharing::updateSharingScreenDisplay(uint32_t bo_name, int32_t width, int32_t height, int32_t stride, int32_t bpp, int32_t swap)
{
    std::lock_guard<std::mutex> lk(mGrabberHelperMtx);
    if (mGrabberHelper) {
        return mGrabberHelper->updateSharingScreenDisplay(bo_name, width, height, stride, bpp, swap);
    }
    return -1;
}

void ScreenSharing::checkScreenSharing()
{
    if (SessionSettings::getInstance().displayPlatform() == DisplayPlatform::X11) {
        mIsSharingSupported = true;
    }
    syslog(LOG_INFO, "[%s]Screen sharing is %s supported!", __func__, mIsSharingSupported ? "" : "not");
}

void ScreenSharing::onShowSharingSelectDialog(int winNum)
{
    mSharingStatus = SharingStatus::eNotReady;

    syslog(LOG_DEBUG, "[%s] Show sharing select dialog...", __func__);
    QWidget* mainWindow = KmreWindowManager::getInstance()->getCurrentFocusedWindow();
    if (mainWindow) {
        // SharingSelectWindow will auto delete when it closed
        mSelectWindow = new SharingSelectWindow(winNum, mainWindow);
        mSelectWindow->open();// 'Window Modality' and nonblock
    }
    else {
        syslog(LOG_ERR, "[%s]Can't get current focused window!", __func__);
    }
}

void ScreenSharing::updateSharingArea(const QRect& rect)
{
    if (!mMaskWindow) {
        mMaskWindow = std::make_unique<MaskWindow>();
    }
    //syslog(LOG_INFO, "[%s]Show MaskWindow for screen sharing", __func__);
    mMaskWindow->show(rect.x(), rect.y(), rect.width(), rect.height());
}

void ScreenSharing::stopSharing()
{
    mSharingStatus = SharingStatus::eNotReady;

    if (mMaskWindow) {
        syslog(LOG_INFO, "[%s]Hide MaskWindow for screen sharing", __func__);
        mMaskWindow->hide();
    }
}

void ScreenSharing::setSharingStatus(SharingStatus status) 
{
    mSharingStatus = status;
    syslog(LOG_INFO, "[%s] Set sharing status: '%d'", __func__, status);

    if (mSharingStatus == SharingStatus::ePaused) {
        if (mMaskWindow) {
            syslog(LOG_INFO, "[%s]Hide MaskWindow for screen sharing", __func__);
            mMaskWindow->hide();
        }
    }
    else if (mSharingStatus == SharingStatus::eReady) {
    }
}