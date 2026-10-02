/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
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
#include "bootui.h"

BootUi::BootUi(QWidget *parent, Qt::WindowFlags f) : QWidget(parent, f)
{
    setAttribute(Qt::WA_DeleteOnClose);
    mVlayout = new QVBoxLayout(this);
    mVlayout->setContentsMargins(0, 0, 0, 0);
    mVlayout->setSpacing(8);
    initUI();
}

BootUi::~BootUi()
{

}


void BootUi::initUI()
{
    mBootTitleLabel = new TitleLabel(this);
    mBootTitleLabel->setText(tr("boot"));
    mBootTitleLabel->setContentsMargins(14,0,0,0);

    mBootFrame = new QFrame(this);
    mBootFrame->setFrameShape(QFrame::Shape::Box);
    mBootFrame->setMinimumSize(550, 80);
    mBootFrame->setMaximumSize(16777215, 80);

    QHBoxLayout *bootHLayout = new QHBoxLayout();
    QVBoxLayout *leftVlayout = new QVBoxLayout();

    //~ contents_path /Boot/Grub verify
    mGrubLabel = new QLabel(tr("Grub verify"));
    mGrubLabel->setAlignment(Qt::AlignBottom);
    mGrubLabel->setMinimumWidth(140);
    mNeedPasswdLabel = new LightLabel(tr("Password required for Grub editing after enabling"), this);
    mNeedPasswdLabel->setAlignment(Qt::AlignTop);
    mResetPasswdBtn = new QPushButton(tr("Reset password"));
    mGrubBtn = new KSwitchButton();

    leftVlayout->addWidget(mGrubLabel);
    leftVlayout->addWidget(mNeedPasswdLabel);
    leftVlayout->setContentsMargins(0, 0, 0, 0);

    bootHLayout->addLayout(leftVlayout);
    bootHLayout->addStretch();
    bootHLayout->addWidget(mResetPasswdBtn);
    bootHLayout->addSpacing(7);
    bootHLayout->addWidget(mGrubBtn);

    bootHLayout->setContentsMargins(12, 0, 14, 0);

    mBootFrame->setLayout(bootHLayout);

    mVlayout->addWidget(mBootTitleLabel);
    mVlayout->addWidget(mBootFrame);
    mVlayout->addStretch();
}
