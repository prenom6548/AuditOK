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

#include "config-progressbar-animation-helper.h"
#include <QDebug>

using namespace UKUIConfigStyleSpace;

ConfigProgressBarAnimationHelper::ConfigProgressBarAnimationHelper(QObject *parent) : QObject(parent)
{
    animationsMap = new QHash<QWidget *, QVariantAnimation *>();
}



ConfigProgressBarAnimationHelper::~ConfigProgressBarAnimationHelper()
{
    for(auto item = animationsMap->begin(); item != animationsMap->end(); item++){
        if(item.value()){
//            delete item.value();
//            item.value() = nullptr;
        }
    }

    if(animationsMap){
        delete animationsMap;
        animationsMap = nullptr;
    }
}



void ConfigProgressBarAnimationHelper::initAnimationMap(QWidget* w, QVariantAnimation *animation)
{
    if (!w || !animation)
        return ;
    stopAnimation(w);
    // connect(animation, SIGNAL(destroyed()), SLOT(_q_removeAnimation()), Qt::UniqueConnection);
    // animationsMap->remove(w);
    connect(w, &QWidget::destroyed, [&, w](){
        animationsMap->remove(w);
    });
    animationsMap->insert(w, animation);
    // animation->start();
}



void ConfigProgressBarAnimationHelper::stopAnimation(QWidget *target)
{
    QVariantAnimation *animation = animationsMap->take(target);
    if (animation) {
        animation->stop();
        delete animation;
        animation = nullptr;
    }
}



QVariantAnimation* ConfigProgressBarAnimationHelper::getAnimation(QWidget* w)
{
    return animationsMap->value(w);
}



void ConfigProgressBarAnimationHelper::_q_removeAnimation()
{
    // QObject *animation = this->sender();
    // if (animation)
    //     animationsMap->remove(animation->parent());
}


void ConfigProgressBarAnimationHelper::cacheTotalBarInfo(const int& width,const int& height)
{
    totalBarWidth = width;
    totalBarHeight = height;
}
void ConfigProgressBarAnimationHelper::getTotalBarInfo(int& width,int& height) const
{
    width = totalBarWidth;
    height = totalBarHeight;
}
//是否达到绘制白色块的条件
bool ConfigProgressBarAnimationHelper::meetCondition(const int &conditionWidth) const
{
    return conditionWidth >= startThreshold;
}
int ConfigProgressBarAnimationHelper::getSliderFixedSpan() const
{
    return sliderFixedSpan;
}
bool ConfigProgressBarAnimationHelper::canEnableNormalAnimation(int width)
{
    return width >= startAnimationBaseWidth;
}

void ConfigProgressBarAnimationHelper::setAnimationDuration(QVariantAnimation *animation, int curPbWidth, double speed) {
    if (!animation) 
        return ;
    // if (speed < 0)
    //     speed = 0.0;
    // if (speed > 1)
    //     speed = 1.0;
    int baseValue = 2000;
    // int duration = qRound(1.0 * baseValue + curPbWidth * speed);
    int duration = 1.0 * baseValue + curPbWidth * speed;
    animation->setDuration(duration);
}
