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
#include "defaultappui.h"

DefaultAppUi::DefaultAppUi(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    initUi();
}

void DefaultAppUi::initUi()
{
    QVBoxLayout *vLyt = new QVBoxLayout(this);
    vLyt->setSpacing(8);
    vLyt->setContentsMargins(0, 0, 0, 0);

    KLabel *mTitleLabel = new KLabel(this);
    mTitleLabel->setContentsMargins(16, 0, 0, 0);
    //~ contents_path /Defaultapp/Select Default Application
    mTitleLabel->setText(tr("DefaultAppWindow", "Select Default Application"));
    SettingGroup *mDefaultFrame = new SettingGroup(this);

    //~ contents_path /Defaultapp/Browser
    mBrowserFrame = new ComboxWidget(tr("Browser"));
    //~ contents_path /Defaultapp/Mail
    mMailFrame = new ComboxWidget(tr("Mail"));
    //~ contents_path /Defaultapp/Image Viewer
    mImageFrame = new ComboxWidget(tr("Image Viewer"));
    //~ contents_path /Defaultapp/Audio Player
    mAudioFrame = new ComboxWidget(tr("Audio Player"));
    //~ contents_path /Defaultapp/Video Player
    mVideoFrame = new ComboxWidget(tr("Video Player"));
    //~ contents_path /Defaultapp/Text Editor
    mTextFrame = new ComboxWidget(tr("Text Editor"));

    // 恢复默认
    //~ contents_path /Defaultapp/Reset default apps to system recommended apps
    mResetFrame = new PushButtonWidget(tr("Reset default apps to system recommended apps"));
    mResetFrame->setButtonText(tr("Reset"));

    mDefaultFrame->addWidget(mBrowserFrame);
    mDefaultFrame->addWidget(mMailFrame);
    mDefaultFrame->addWidget(mImageFrame);
    mDefaultFrame->addWidget(mAudioFrame);
    mDefaultFrame->addWidget(mVideoFrame);
    mDefaultFrame->addWidget(mTextFrame);
    mDefaultFrame->addWidget(mResetFrame);

    vLyt->addWidget(mTitleLabel);
    vLyt->addWidget(mDefaultFrame);
    vLyt->addStretch();
}
