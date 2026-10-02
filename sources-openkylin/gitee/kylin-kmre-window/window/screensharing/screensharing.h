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

#pragma once

#include <QObject>
#include <QRect>
#include "grabberhelper.h"
#include "singleton.h"
#include <thread>
#include <mutex>
#include <atomic>

class MaskWindow;
class SharingSelectWindow;

enum class SharingStatus {
    eNotReady,
    eReady,
    ePaused,
};

class ScreenSharing : public QObject, public kmre::SingletonP<ScreenSharing>
{
    Q_OBJECT
private:
    ScreenSharing();
    ~ScreenSharing();
    void checkScreenSharing();

    bool mIsSharingSupported;
    int mSharingScreenNum;
    SharingStatus mSharingStatus;
    
    std::mutex mGrabberHelperMtx;
    std::unique_ptr<kmre::GrabberHelper> mGrabberHelper;
    std::unique_ptr<MaskWindow> mMaskWindow;
    SharingSelectWindow* mSelectWindow;

public slots:
    void onShowSharingSelectDialog(int winNum);

public:
    bool isSharingSupported() {return mIsSharingSupported;}
    void addBo(uint32_t name);
    int32_t grabNextBo(uint32_t at_name, uint32_t name, int32_t width, int32_t height, int32_t stride, int32_t ver);
    int32_t updateSharingScreenDisplay(uint32_t bo_name, int32_t width, int32_t height, int32_t stride, int32_t bpp, int32_t swap);
    
    void setSharingScreenNum(int num) {mSharingScreenNum = num;}
    int  getSharingScreenNum() {return mSharingScreenNum;}
    void setSharingStatus(SharingStatus status);
    SharingStatus  getSharingStatus() {return mSharingStatus;}
    bool isSharingReady() {return mSharingStatus == SharingStatus::eReady;}
    bool isSharingPaused() {return mSharingStatus == SharingStatus::ePaused;}
    
    void updateSharingArea(const QRect& rect);
    void stopSharing();

    friend SingletonP<ScreenSharing>;
};