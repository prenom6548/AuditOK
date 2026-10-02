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

#include "kalertdialog.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>

KAlertDialog::KAlertDialog(QWidget *parent) : KDialog(parent)
{
    this->setFixedSize(424, 164);

    minimumButton()->setVisible(false);
    maximumButton()->setVisible(false);
    connect(closeButton(), &QPushButton::clicked, [this]() {
        this->close();
    });

    QLabel *mIcon = new QLabel(this);
    mIcon->setFixedSize(24, 24);
    if (QIcon::hasThemeIcon("dialog-warning")) {
        mIcon->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(QSize(24, 24)));
    } else {
        qWarning() << "KAlertDialog::KAlertDialog 无系统图标！";
    }

    mText = new QLabel(this);
    mText->resize(126, 22);
    mText->adjustSize();
    mText->setWordWrap(true);
    mText->setText(tr("The file you selected is a directory or does not have permissions, cannot be shredded!"));

    QPushButton *mBtn = new QPushButton(this);
    mBtn->setText(tr("sure"));
    mBtn->setFixedSize(96, 36);
    connect(mBtn, &QPushButton::clicked, [this]() {
        this->close();
        Q_EMIT sigCloseDialog();
    });

    QHBoxLayout *mHLayout = new QHBoxLayout;
    mHLayout->setContentsMargins(0, 0, 0, 0);
    mHLayout->setSpacing(0);
    mHLayout->addWidget(mIcon);
    mHLayout->addSpacing(9);
    mHLayout->addWidget(mText);
    mHLayout->addStretch();

    QHBoxLayout *mHLayoutBtn = new QHBoxLayout;
    mHLayoutBtn->setContentsMargins(0, 0, 0, 0);
    mHLayoutBtn->setSpacing(0);
    mHLayoutBtn->addStretch();
    mHLayoutBtn->addWidget(mBtn);

    QVBoxLayout *mVLayout = new QVBoxLayout;
    mVLayout->setContentsMargins(24, 0, 24, 24);
    mVLayout->setSpacing(0);
    mVLayout->addLayout(mHLayout);
    mVLayout->addStretch();
    mVLayout->addLayout(mHLayoutBtn);

    mainWidget()->setLayout(mVLayout);
}

void KAlertDialog::setAlertText(QString str)
{
    if (mText) {
        mText->setText(str);
    }
}
