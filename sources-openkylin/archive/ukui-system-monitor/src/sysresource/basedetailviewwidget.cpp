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
#include "basedetailviewwidget.h"

BaseDetailViewWidget::BaseDetailViewWidget(QWidget *parent)
    : QWidget(parent)
{
    initUI();
    initConnect();
}

BaseDetailViewWidget::~BaseDetailViewWidget()
{

}

void BaseDetailViewWidget::initUI()
{
    mMainLayout =  new QVBoxLayout();
    mMainLayout->setContentsMargins(16,0,20,16);
    mMainLayout->setSpacing(0);

    QFont ft;
    ft.setPointSize(36);

    mTitleLabel = new KLabel();
    mTitleLabel->setText(mTitleText);
    mTitleLabel->setFont(ft);
    mDetailLabel = new KLabel();
    mDetailLabel->setFontColorRole(QPalette::PlaceholderText);
    mHideBtn = new KBorderlessButton(tr("Hide Details"));

    QHBoxLayout *mTopHLayout = new QHBoxLayout();
    mTopHLayout->setContentsMargins(0,0,0,0);
    mTopHLayout->addWidget(mTitleLabel);
    mTopHLayout->addSpacing(8);
    mTopHLayout->addWidget(mDetailLabel);
    mTopHLayout->addStretch();
    mTopHLayout->addWidget(mHideBtn);
    mMainLayout->addLayout(mTopHLayout);

    this->setLayout(mMainLayout);
}

void BaseDetailViewWidget::initConnect()
{
    connect(mHideBtn, &KBorderlessButton::clicked, this, [=](){
        Q_EMIT hideDetails(0);
    });
}

void BaseDetailViewWidget::setTitle(const QString &text)
{
    mTitleText = text;
    mTitleLabel->setText(mTitleText);
}

void BaseDetailViewWidget::setDetail(const QString &text)
{
    mDetailText = text;
    mDetailLabel->setText(mDetailText);
}
