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
#include <QVariant>
#include "typedef.h"
#include "screencapturedbus.h"

using namespace kmre;

class QGSettings;
class KmreWindow;

class ScreenCapture : public QObject
{
    Q_OBJECT

public:
    ScreenCapture(KmreWindow* window);
    ~ScreenCapture();

private:
    void getScreenShotImage(const QString &filePath);
    
public slots:
    void onScreenShot();
    void onHideWindowDuringScreenShot(bool hide) {mHideWindowDuringScreenShot = hide;}
    
private slots:
    void onCaptureTaken(uint id, QByteArray rawImage);
    void onCaptureCopy(uint id);
    void onCaptureExit(uint id);
    void onCaptureFailed(uint id);

private:
    KmreWindow* mMainWindow = nullptr;
    ScreenCaptureDbus *mScreenCaptureDbus;
    up<QGSettings> mScreenShotGSetting;
    bool mHideWindowDuringScreenShot;
    bool mStartShot;
    bool mCopysig;
    QVariant mImageData;
    bool mHasImage;
    bool mIsCapturing;
};