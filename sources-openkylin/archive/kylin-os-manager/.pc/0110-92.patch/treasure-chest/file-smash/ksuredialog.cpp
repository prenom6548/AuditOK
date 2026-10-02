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

#include "ksuredialog.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>

KSureDialog::KSureDialog(QWidget *parent) : KDialog(parent)
{
    this->setFixedSize(452, 195);

    minimumButton()->setVisible(false);
    maximumButton()->setVisible(false);
    connect(closeButton(), &QPushButton::clicked, [this]() {
        this->close();
    });

    QLabel *mIcon = new QLabel(this);
    mIcon->setFixedSize(24, 24);
    if (QIcon::hasThemeIcon("dialog-question")) {
        mIcon->setPixmap(QIcon::fromTheme("dialog-question").pixmap(QSize(24, 24)));
    } else {
        qWarning() << "KSureDialog::KSureDialog 无系统图标！";
    }

    QLabel *mText = new QLabel(this);
    mText->setFixedSize(372, 40);
    mText->setText(tr("Are you sure to start crushing?"));
    mText->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    QLabel *mTip = new QLabel(this);
    mTip->setFixedSize(372, 40);
    mTip->setAlignment(Qt::AlignLeft);
    mTip->setText(tr("Crushed files cannot be recovered!"));
    QPalette pal;
    QColor color = palette().color(QPalette::PlaceholderText);
    pal.setColor(QPalette::WindowText, color);
    mTip->setPalette(pal);

    QPushButton *mBtnSure = new QPushButton(this);
    mBtnSure->setText(tr("sure"));
    mBtnSure->setFixedSize(96, 36);
    connect(mBtnSure, &QPushButton::clicked, [this]() {
        this->close();
        emit sigCloseDialog(true);
    });

    QPushButton *mBtnCancle = new QPushButton(this);
    mBtnCancle->setText(tr("cancle"));
    mBtnCancle->setFixedSize(96, 36);
    connect(mBtnCancle, &QPushButton::clicked, [this]() {
        this->close();
        emit sigCloseDialog(false);
    });

    QHBoxLayout *mHLayout = new QHBoxLayout;
    mHLayout->setContentsMargins(0, 0, 0, 0);
    mHLayout->setSpacing(0);
    mHLayout->addWidget(mIcon);
    mHLayout->addSpacing(9);
    mHLayout->addWidget(mText);
    mHLayout->addStretch();

    QHBoxLayout *mHLayoutLabel = new QHBoxLayout;
    mHLayoutLabel->setContentsMargins(0, 0, 0, 0);
    mHLayoutLabel->setSpacing(0);
    mHLayoutLabel->addSpacing(33);
    mHLayoutLabel->addWidget(mTip);
    mHLayoutLabel->addStretch();

    QHBoxLayout *mHLayoutBtn = new QHBoxLayout;
    mHLayoutBtn->setContentsMargins(0, 0, 0, 0);
    mHLayoutBtn->setSpacing(0);
    mHLayoutBtn->addStretch();
    mHLayoutBtn->addWidget(mBtnCancle);
    mHLayoutBtn->addSpacing(8);
    mHLayoutBtn->addWidget(mBtnSure);

    QVBoxLayout *mVLayout = new QVBoxLayout;
    mVLayout->setContentsMargins(24, 0, 24, 24);
    mVLayout->setSpacing(0);
    mVLayout->addLayout(mHLayout);
    mVLayout->addSpacing(9);
    mVLayout->addLayout(mHLayoutLabel);
    mVLayout->addStretch();
    mVLayout->addLayout(mHLayoutBtn);

    mainWidget()->setLayout(mVLayout);
}
