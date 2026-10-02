/*
 * Qt5-UKUI's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: xibowen <xibowen@kylinos.cn>
 *
 */

#include "config-progressbar-animation.h"

using namespace UKUIConfigStyleSpace;

// ConfigProgressBarAnimation::ConfigProgressBarAnimation(QObject *parent) : QVariantAnimation (parent)

ConfigProgressBarAnimation::ConfigProgressBarAnimation(QObject *parent, ProgressBarStatus curStatus) 
                                : QVariantAnimation (parent)
{
    // init();
    init(curStatus);
}



QWidget *ConfigProgressBarAnimation::target()
{
    return qobject_cast<QWidget*>(parent());
}

ConfigProgressBarAnimation::~ConfigProgressBarAnimation()
{

}

void ConfigProgressBarAnimation::init(ProgressBarStatus curStatus)
{
    // this->setStartValue(0.0);
    // this->setEndValue(1.0);
    // this->setDuration(2500);
    // this->setDirection(QAbstractAnimation::Forward);
    // this->setEasingCurve(QEasingCurve::InOutSine);

    if (curStatus == ProgressBarStatus::INDETERMINATE) {//用于indeterminate状态动画的初始化，往返移动
        this->setStartValue(0.0);
        this->setEndValue(1.0);
        this->setDuration(2500);
        this->setDirection(QAbstractAnimation::Forward);
        this->setEasingCurve(QEasingCurve::InOutSine);
    } else if (curStatus == ProgressBarStatus::NORMAL) {//用于normal状态动画的初始化，循环前向移动
        this->setStartValue(0.0);
        this->setEndValue(1.0);
        this->setDuration(2500);
        this->setDirection(QAbstractAnimation::Forward);
        this->setEasingCurve(QEasingCurve::Linear);
    }


    connect(this, &QVariantAnimation::valueChanged, target(), [=](QVariant value) {
        // target()->repaint();
        target()->update();//repaint会造成recursive repaint问题
    });
    connect(this, &QVariantAnimation::finished, target(), [=]() {
    //    target()->repaint();
        target()->update();
    });
}
