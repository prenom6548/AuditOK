/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 * Zero Liu    liuzenghui1@kylinos.cn
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

#include "nojoystickwidget.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

NoJoystickWidget::NoJoystickWidget(QWidget *parent) : QWidget(parent)
{

    QVBoxLayout* mVBLayout = new QVBoxLayout();

    QLabel* mIconLabel = new QLabel(this);
    mIconLabel->setFixedSize(96,96);
    mIconLabel->setStyleSheet("QLabel{background:transparent;border-image:url(:/new/res/no_joystick.svg);}");
    mIconLabel->setAlignment(Qt::AlignCenter);

    QLabel* mTextLabel = new QLabel(this);
    mTextLabel->setText(tr("No handle detected"));
    QFont font;
    font.setPointSize(14);
    mTextLabel->setFont(font);

    mVBLayout->addStretch(1);
    mVBLayout->addWidget(mIconLabel);
    mVBLayout->addWidget(mTextLabel);
    mVBLayout->addStretch(2);

    mVBLayout->setAlignment(mIconLabel,Qt::AlignHCenter);
    mVBLayout->setAlignment(mTextLabel,Qt::AlignHCenter);

    this->setLayout(mVBLayout);

}

NoJoystickWidget::~NoJoystickWidget(){

}
