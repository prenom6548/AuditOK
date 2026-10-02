/*
 * kylin-os-manager
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "netcheckhomepage.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <config_win.h>
#include "frame.h"

NetCheckHomePage::NetCheckHomePage(QWidget *parent) : QWidget(parent)
{

    mTitleLabel = new QLabel(this);
    mTitleLabel->setText(tr("Check and Repair"));
    mTitleLabel->setFixedSize(472, 48);
    QFont font;
    font.setPixelSize(36);
    font.setBold(true);
    mTitleLabel->setFont(font);
    mDescribLabel = new CustomLabel(this);
    mDescribLabel->setText(tr("Detection and repair of computer problems"));
    mDescribLabel->setFixedSize(472, 22);
    font.setBold(false);
    font.setPixelSize(16);
    mDescribLabel->setFont(font);

    mIconLabel = new QLabel(this);
    mIconLabel->setFixedSize(36, 36);
    QIcon ico("/data/item-icon-dark.svg");
    QPixmap pix = ico.pixmap(ico.actualSize(QSize(36, 36)));
    mIconLabel->setPixmap(pix);

    mItemLabel = new QLabel(this);
    mItemLabel->setText(tr("NetCheck"));
    mItemLabel->setFixedSize(424, 22);

    QHBoxLayout *mItemLayout = new QHBoxLayout;
    mItemLayout->setSpacing(0);
    mItemLayout->setContentsMargins(0, 0, 0, 0);
    mItemLayout->addWidget(mIconLabel);
    mItemLayout->addSpacing(12);
    mItemLayout->addWidget(mItemLabel);
    mItemLayout->addStretch();

    mStartBtn = new QPushButton(this);
    mStartBtn->setFixedSize(180, 52);
    mStartBtn->setProperty("isImportant", true);
    font.setPixelSize(24);
    font.setBold(false);
    mStartBtn->setFont(font);
    mStartBtn->setText(tr("Start"));
    connect(mStartBtn, &QPushButton::clicked, this, &NetCheckHomePage::slotStartBtnClicked);

    QVBoxLayout *mLeftLayout = new QVBoxLayout;
    mLeftLayout->setSpacing(0);
    mLeftLayout->setContentsMargins(0, 0, 0, 0);
    mLeftLayout->addWidget(mTitleLabel);
    mLeftLayout->addSpacing(8);
    mLeftLayout->addWidget(mDescribLabel);
    mLeftLayout->addStretch();
    mLeftLayout->addLayout(mItemLayout);
    mLeftLayout->addStretch();
    mLeftLayout->addWidget(mStartBtn);


    mPicLabel = new QLabel(this);
    mPicLabel->setFixedSize(256, 256);
    QIcon ico1(":/data/issue-check-light.png");
    QPixmap pix1 = ico1.pixmap(ico1.actualSize(QSize(256, 256)));
    mPicLabel->setPixmap(pix1);

    QHBoxLayout *mMidLayout = new QHBoxLayout;
    mMidLayout->setSpacing(0);
    mMidLayout->setContentsMargins(0, 0, 0, 0);
    mMidLayout->addLayout(mLeftLayout);
    mMidLayout->addSpacing(16);
    mMidLayout->addWidget(mPicLabel);

    mSetButton = new KBorderlessButton(this);
    //    mSetButton->setFixedSize(130,36);
    connect(mSetButton, &KBorderlessButton::clicked, this, &NetCheckHomePage::showConfigureWin);
    mSetButton->setText(tr("IntraNetSet"));

    QHBoxLayout *mBtnLayout = new QHBoxLayout;
    mBtnLayout->setSpacing(0);
    mBtnLayout->setContentsMargins(0, 0, 0, 0);
    mBtnLayout->addStretch();
    mBtnLayout->addWidget(mSetButton);

    QVBoxLayout *mAllLayout = new QVBoxLayout;
    mAllLayout->setSpacing(0);
    mAllLayout->setContentsMargins(40, 40, 40, 40);
    mAllLayout->addStretch();
    mAllLayout->addLayout(mMidLayout);
    mAllLayout->addStretch();
    mAllLayout->addLayout(mBtnLayout);

    this->setLayout(mAllLayout);
}

void NetCheckHomePage::slotStartBtnClicked()
{
    //隐藏当前界面
    emit sigChangeStackWidgetHome(1);
    //开始检测
    emit sigStartCheck();
}
void NetCheckHomePage::changeThemeColor(int color)
{
    if (color) {
        QString icon4 = ":/data/item-icon-dark.svg";
        QPixmap pix(icon4);
        mIconLabel->setPixmap(pix);

        QIcon ico1(":/data/issue-check-dark.png");
        QPixmap pix1 = ico1.pixmap(ico1.actualSize(QSize(256, 256)));
        mPicLabel->setPixmap(pix1);
    } else {
        QString icon4 = ":/data/itme-icon-light.svg";
        QPixmap pix(icon4);
        mIconLabel->setPixmap(pix);

        QIcon ico1(":/data/issue-check-light.png");
        QPixmap pix1 = ico1.pixmap(ico1.actualSize(QSize(256, 256)));
        mPicLabel->setPixmap(pix1);
    }
}
void NetCheckHomePage::changeSystemSize(int size)
{
    mFontSize = size;
    QFont font;

    int labelSize = 16 * mFontSize / 11;
    font.setPixelSize(labelSize);
    mDescribLabel->setFont(font);

    //    int btnSize = 14 * mFontSize / 11;
    //    font.setPixelSize(btnSize);
    //    mSetButton->setFont(font);
}

void NetCheckHomePage::showConfigureWin()
{
    ConfigWin configureWin;
    connect(&configureWin, SIGNAL(sigUpdateConfigFile()), this, SIGNAL(sigUpdateMainConfig()));

    // 移动到界面中间
    auto frameRect = Frame::geometry();
    auto x = frameRect.x() + frameRect.width() / 2 - configureWin.width() / 2;
    auto y = frameRect.y() + frameRect.height() / 2 - configureWin.height() / 2;
    configureWin.move(x, y);

    configureWin.activateWindow();
    configureWin.showWin();
}
