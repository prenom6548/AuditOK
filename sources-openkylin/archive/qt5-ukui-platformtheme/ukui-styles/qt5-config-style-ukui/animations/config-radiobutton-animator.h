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

#ifndef RADIOANIMATOR_H
#define RADIOANIMATOR_H

#include <QObject>
#include <QVariantAnimation>
#include <QParallelAnimationGroup>

#include "animations/animator-iface.h"


namespace UKUIConfigStyleSpace {

class ConfigRadioButtonAnimator : public QParallelAnimationGroup, public AnimatorIface
{
    Q_OBJECT
private:
    QWidget* m_widget = nullptr;

public:
    explicit ConfigRadioButtonAnimator(QObject *parent = nullptr);
    ~ConfigRadioButtonAnimator();

    bool bindWidget(QWidget *w);
    bool unboundWidget();
    QWidget *boundedWidget() {return m_widget;}
    QVariant value(const QString &property);
    bool setAnimatorStartValue(const QString &property, const QVariant &value);
    bool setAnimatorEndValue(const QString &property, const QVariant &value);
    bool setAnimatorDuration(const QString &property, int duration);
    void setAnimatorDirectionForward(const QString &property = nullptr, bool forward = true);
    bool isRunning(const QString &property = nullptr);
    void startAnimator(const QString &property = nullptr);
    void stopAnimator(const QString &property = nullptr);
    int currentAnimatorTime(const QString &property = nullptr);
    void setAnimatorCurrentTime(const QString &property = nullptr, const int msecs = 0);
    int totalAnimationDuration(const QString &property = nullptr);

    bool isStopped(const QString &property = nullptr);

Q_SIGNALS:


private:
    QVariantAnimation *m_off = nullptr;
    QVariantAnimation *m_sunkenon = nullptr;

    QVariantAnimation* m_off_hover = nullptr;//用于按钮off(unchecked)状态下的hover的动画对象
    QVariantAnimation* m_on_hover = nullptr;//用于按钮on(checked)状态下hover的动画对象

    QVariantAnimation* m_sunken_onhover = nullptr;//用于按钮on状态下，鼠标按压-松开过程的动画，动画内容和普通onhover基本上一样
};
}

#endif // RADIOANIMATOR_H
