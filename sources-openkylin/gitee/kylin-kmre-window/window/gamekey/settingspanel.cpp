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

#include "settingspanel.h"
#include "utils.h"
#include "preferences.h"
#include "gamekeymanager.h"
#include "kmrewindow.h"
#include "sessionsettings.h"

#include <sys/syslog.h>

SettingsPanel::SettingsPanel(KmreWindow* window) 
    : QWidget(window)
{
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    setAttribute(Qt::WA_NoMousePropagation);
#if defined(__x86_64) || defined(__x86_64__) || defined(__amd64) || defined(__amd64__) || \
    defined(__i386) || defined(__i386__) || defined(__i686) || defined(__i686__)
    if (SessionSettings::getInstance().windowUsePlatformX11()) {
        setAttribute(Qt::WA_NativeWindow);
    }
#endif

    mAddSingleKeyBtn = new QToolButton(this);
    mAddSingleKeyBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    mAddSingleKeyBtn->setIcon(QIcon(":/res/key_click.png"));
    mAddSingleKeyBtn->setIconSize({36, 36});
    mAddSingleKeyBtn->setMinimumSize({100, 70});
    mAddSingleKeyBtn->setText(tr("Game key"));
    mAddSingleKeyBtn->setToolTip(tr("Click the button to create a custom game button"));
    mAddSingleKeyBtn->setFocusPolicy(Qt::NoFocus);
    connect(mAddSingleKeyBtn, &QToolButton::clicked, this, [this] {
        emit addSingleKey();
    });

    mAddSteelingWheelBtn = new QToolButton(this);
    mAddSteelingWheelBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    mAddSteelingWheelBtn->setIcon(QIcon(":/res/key_walk.png"));
    mAddSteelingWheelBtn->setIconSize({36, 36});
    mAddSteelingWheelBtn->setMinimumSize({100, 70});
    mAddSteelingWheelBtn->setText(tr("Direction key"));
    mAddSteelingWheelBtn->setToolTip(tr("Click the button to create a custom game button"));
    mAddSteelingWheelBtn->setFocusPolicy(Qt::NoFocus);
    connect(mAddSteelingWheelBtn, &QToolButton::clicked, this, [this] {
        emit addSteelingWheel();
    });
    
    mTransparencySlider = new QSlider(this);
    mTransparencySlider->setMinimum(0);
    mTransparencySlider->setMaximum(100);
    mTransparencySlider->setValue(window->getGameKeyManager()->getTransparency() * 100);
    mTransparencySlider->setTracking(true);
    mTransparencySlider->setOrientation(Qt::Horizontal);
    connect(mTransparencySlider, &QSlider::valueChanged, this, [this] {
        emit updateTransparency(mTransparencySlider->value() / 100.0);
    });

    mTransparencyLabel = new QLabel(this);
    mTransparencyLabel->setText(tr("transparency"));
    mTransparencyLabel->setAlignment(Qt::AlignCenter);

    mSaveBtn = new QPushButton(this);
    mSaveBtn->setText(tr("Save"));
    mSaveBtn->setFocusPolicy(Qt::NoFocus);
    connect(mSaveBtn, &QPushButton::clicked, this, [this] {
        emit saveGameKeys();
    });

    mHideBtn = new QPushButton(this);
    mHideBtn->setText(tr("Hide"));
    mHideBtn->setFocusPolicy(Qt::NoFocus);
    connect(mHideBtn, &QPushButton::clicked, this, [this] {
        emit hideGameKeys();
    });

    mClearBtn = new QPushButton(this);
    mClearBtn->setText(tr("Clear"));
    mClearBtn->setFocusPolicy(Qt::NoFocus);
    connect(mClearBtn, &QPushButton::clicked, this, [this] {
        emit clearGameKeys();
    });

    mLayout = new QGridLayout(this);
    mLayout->setMargin(5);
    mLayout->setSpacing(5);
    mLayout->addWidget(mAddSingleKeyBtn, 0, 0, 2, 6, Qt::AlignRight);
    mLayout->addWidget(mAddSteelingWheelBtn, 0, 6, 2, 6, Qt::AlignRight);
    mLayout->addWidget(mTransparencySlider, 0, 13, 1, 4, Qt::AlignCenter);
    mLayout->addWidget(mTransparencyLabel, 0, 18, 1, 3, Qt::AlignLeft);
    mLayout->addWidget(mClearBtn, 1, 12, 1, 3, Qt::AlignLeft);
    mLayout->addWidget(mHideBtn, 1, 15, 1, 3, Qt::AlignLeft);
    mLayout->addWidget(mSaveBtn, 1, 18, 1, 3, Qt::AlignLeft);
 
    setLayout(mLayout);

    resize(DEFAULT_SETTINGS_PANEL_WIDTH, DEFAULT_SETTINGS_PANEL_HEIGHT);
}

SettingsPanel::~SettingsPanel()
{
    delete mLayout;
    delete mAddSingleKeyBtn;
    delete mAddSteelingWheelBtn;
    delete mSaveBtn;
    delete mHideBtn;
    delete mClearBtn;
}

void SettingsPanel::enableAddSteelingWheelBtn(bool enable)
{
    mAddSteelingWheelBtn->setEnabled(enable);
}

