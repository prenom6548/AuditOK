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

#ifndef PROGRESSBARANIMATIONHELPER_H
#define PROGRESSBARANIMATIONHELPER_H

#include <QObject>
#include <QVariantAnimation>
#include <QHash>
#include "config-progressbar-animation.h"

namespace UKUIConfigStyleSpace {

class ConfigProgressBarAnimationHelper : public QObject
{
    Q_OBJECT
public:
    ConfigProgressBarAnimationHelper(QObject *parent = nullptr);
    virtual ~ConfigProgressBarAnimationHelper();

    void initAnimationMap(QWidget* w, QVariantAnimation *animation);
    void stopAnimation(QWidget *target);
    QVariantAnimation* getAnimation(QWidget* w);

private:
    //进度条normal动画滑片的一些参数
    // const int baseDuration = 1000;
    // const int baseMinWidth = 200;
    // const int baseMaxWidth = 1000;
    // const int baseMaxDuration = 4000;
    // const int baseMinDuration = 1000;

    const int sliderFixedSpan = 80;//滑片本身固定宽度
    const int startAnimationBaseWidth = 150;//进度条达到150宽度才开始光片移动动画

    int totalBarWidth;//进度条整体宽度
    int totalBarHeight;//进度条整体高度

    const int startThreshold = 200;
public:
    bool canEnableNormalAnimation(int width);

    int getSliderFixedSpan() const;
    void cacheTotalBarInfo(const int& width,const int& height);//存储进度条信息，宽，高
    void getTotalBarInfo(int& width,int& height) const;//获取进度条信息，宽，高
    bool meetCondition(const int& conditionWidth) const;

public:
    void setAnimationDuration(QVariantAnimation *animation, int curPbWidth, double speed);

public Q_SLOTS:

    void _q_removeAnimation();

private:
    QHash<QWidget*, QVariantAnimation*> *animationsMap = nullptr;
};
}
#endif // PROGRESSBARANIMATIONHELPER_H
