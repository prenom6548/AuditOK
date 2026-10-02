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

#include "sensitivitywidget.h"
#include <QLabel>
#include <QHBoxLayout>

SensitivityWidget::SensitivityWidget(QWidget *parent) : QWidget(parent)
{

    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    QFrame* frame = new QFrame(this);
    frame->resize(this->width(),this->height());
    frame->setStyleSheet("QFrame{background:rgba(0, 0, 0, 0.85);border-radius:4px;color:rgba(255,255,255,1);}");

    QLabel* labe = new QLabel(this);
    QFont font;
    font.setPointSize(6);
    labe->setFont(font);
    labe->setStyleSheet("QLabel{background-color:transparent;}");
    labe->setAlignment(Qt::AlignCenter);
    labe->setText(tr("Sensitivity"));

    mNumberLabel = new QLabel(this);
    mNumberLabel->setFont(font);
    mNumberLabel->setAlignment(Qt::AlignCenter);
    mNumberLabel->setText("1");
    mNumberLabel->setStyleSheet("QLabel{background-color:transparent;}");

    mSlider = new QSlider(this);
    mSlider->setMaximum(10);
    mSlider->setMinimum(1);
    mSlider->setOrientation(Qt::Horizontal);
    mSlider->setValue(1);
    mSlider->setTracking(true);
    connect(mSlider,&QSlider::valueChanged,this,[this](){
        mNumberLabel->setText(QString::number(mSlider->value()));
        emit currentSliderValue(mSlider->value());
    });

    QHBoxLayout* mHLayout = new QHBoxLayout(this);
    mHLayout->setSpacing(0);
    mHLayout->setMargin(4);
    mHLayout->addWidget(labe);
    mHLayout->addWidget(mSlider);
    mHLayout->addWidget(mNumberLabel);
    mHLayout->setStretchFactor(labe,3);
    mHLayout->setStretchFactor(mSlider,4);
    mHLayout->setStretchFactor(mNumberLabel,1);

    frame->setLayout(mHLayout);

    QHBoxLayout* mAllLayout = new QHBoxLayout(this);
    mAllLayout->addWidget(frame);
    mAllLayout->setMargin(0);
    mAllLayout->setSpacing(0);
    this->setLayout(mAllLayout);

}

void SensitivityWidget::setSliderLabelDefault(int value){
    mSlider->setValue(value);
    mNumberLabel->setText(QString::number(value));
}
SensitivityWidget::~SensitivityWidget(){

}
