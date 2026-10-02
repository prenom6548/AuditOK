/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef DEFAULTAPPUI_H
#define DEFAULTAPPUI_H

#include <QObject>
#include <QWidget>

#include "shell/interface.h"
#include "mthread.h"
#include "comboxwidget.h"
#include "settinggroup.h"
#include "pushbuttonwidget.h"
#include <klabel.h>

using namespace kdk;

class DefaultAppUi : public QWidget
{
    Q_OBJECT
public:
    explicit DefaultAppUi(QWidget *parent = nullptr);
    void initUi();

    ComboxWidget *getBrowserWidget()
    {
        return mBrowserFrame;
    }
    ComboxWidget *getMailWidget()
    {
        return mMailFrame;
    }
    ComboxWidget *getImageWidget()
    {
        return mImageFrame;
    }
    ComboxWidget *getAudioWidget()
    {
        return mAudioFrame;
    }
    ComboxWidget *getVideoWidget()
    {
        return mVideoFrame;
    }
    ComboxWidget *getTextWidget()
    {
        return mTextFrame;
    }
    PushButtonWidget *getResetWidget()
    {
        return mResetFrame;
    }

private:
    ComboxWidget *mBrowserFrame = nullptr;
    ComboxWidget *mMailFrame = nullptr;
    ComboxWidget *mImageFrame = nullptr;
    ComboxWidget *mAudioFrame = nullptr;
    ComboxWidget *mVideoFrame = nullptr;
    ComboxWidget *mTextFrame = nullptr;
    PushButtonWidget *mResetFrame = nullptr;

signals:

};

#endif // DEFAULTAPPUI_H
