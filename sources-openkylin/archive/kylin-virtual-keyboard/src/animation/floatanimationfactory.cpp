/*
 * Copyright (c) KylinSoft Co., Ltd. 2023.All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "animation/floatanimationfactory.h"
#include <QEasingCurve>
#include <QParallelAnimationGroup>
#include <QPointF>
#include <QPropertyAnimation>
#include <QVariant>

std::unique_ptr<QAbstractAnimation>
FloatAnimationFactory::createShowAnimation(QObject *target,
                                           const AnimationInfo &animationInfo) {
    return createYAndOpacityAnimationGroup(target, animationInfo);
}

std::unique_ptr<QAbstractAnimation>
FloatAnimationFactory::createHideAnimation(QObject *target,
                                           const AnimationInfo &animationInfo) {
    return createYAndOpacityAnimationGroup(target, animationInfo);
}

std::unique_ptr<QAbstractAnimation>
FloatAnimationFactory::createFlipAnimation(QObject *target,
                                           const AnimationInfo &animationInfo) {
    return createPropertyAnimation(target, "y", animationInfo.startY,
                                   animationInfo.endY, 200);
}

std::unique_ptr<QAbstractAnimation>
FloatAnimationFactory::createPropertyAnimation(QObject *target,
                                               const QByteArray &propertyName,
                                               const QVariant &startValue,
                                               const QVariant &endValue,
                                               int duration) {
    std::unique_ptr<QPropertyAnimation> animation(
        new QPropertyAnimation(target, propertyName));
    animation->setStartValue(startValue);
    animation->setEndValue(endValue);
    animation->setDuration(duration);
    QEasingCurve easingCurve(QEasingCurve::BezierSpline);
    easingCurve.addCubicBezierSegment(QPointF(0.25, 0.1), QPointF(0.25, 0.1),
                                      QPointF(1, 1));
    animation->setEasingCurve(easingCurve);

    return animation;
}

std::unique_ptr<QAbstractAnimation>
FloatAnimationFactory::createYAndOpacityAnimationGroup(
    QObject *target, const AnimationInfo &animationInfo) {
    std::unique_ptr<QParallelAnimationGroup> animationGroup(
        new QParallelAnimationGroup());
    animationGroup->addAnimation(
        createPropertyAnimation(target, "y", animationInfo.startY,
                                animationInfo.endY, 200)
            .release());
    animationGroup->addAnimation(
        createPropertyAnimation(
            target, "opacity",
            QVariant(static_cast<qreal>(animationInfo.startOpacity)),
            QVariant(static_cast<qreal>(animationInfo.endOpacity)), 200)
            .release());
    return animationGroup;
}
