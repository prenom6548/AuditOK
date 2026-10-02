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

#include "config-radiobutton-animator.h"

#include <QRadioButton>
#include <QDebug>

using namespace UKUIConfigStyleSpace;

ConfigRadioButtonAnimator::ConfigRadioButtonAnimator(QObject *parent) : QParallelAnimationGroup (parent)
{

}

ConfigRadioButtonAnimator::~ConfigRadioButtonAnimator()
{
    if (m_off) {
        m_off->deleteLater();
        m_off = nullptr;
    }
    if (m_sunkenon) {
        m_sunkenon->deleteLater();
        m_sunkenon = nullptr;
    }

    if (m_off_hover) {
        m_off_hover->deleteLater();
        m_off_hover = nullptr;
    }

    if (m_on_hover) {
        m_on_hover->deleteLater();
        m_on_hover = nullptr;
    }
    if (m_sunken_onhover) {
        m_sunken_onhover->deleteLater();
        m_sunken_onhover = nullptr;
    }
}

bool ConfigRadioButtonAnimator::bindWidget(QWidget *w)
{
    if (w->property("doNotAnimate").toBool())
        return false;

    if (qobject_cast<QRadioButton*>(w)) {
        m_widget = w;
    } else {
        return false;
    }

    m_sunkenon = new QVariantAnimation(this);
    m_sunkenon->setStartValue(0.0);
    m_sunkenon->setEndValue(1.0);
    m_sunkenon->setDuration(200);
    m_sunkenon->setEasingCurve(QEasingCurve::Linear);


    addAnimation(m_sunkenon);

    connect(m_sunkenon, &QVariantAnimation::valueChanged, w, [=](const QVariant &value) {
       w->update();
    });
    connect(m_sunkenon, &QVariantAnimation::finished, w, [=]() {
        w->update();
    });

    m_off = new QVariantAnimation(this);
    m_off->setStartValue(1.0);
    m_off->setEndValue(0.0);
    m_off->setDuration(100);
    m_off->setEasingCurve(QEasingCurve::Linear);

    addAnimation(m_off);

    connect(m_off, &QVariantAnimation::valueChanged, w, [=](const QVariant &value) {
       w->update();
    });
    connect(m_off, &QVariantAnimation::finished, w, [=]() {
        w->update();
    });

    //OffHover 动画初始化
    m_off_hover = new QVariantAnimation(this);
    m_off_hover->setStartValue(0.0);
    m_off_hover->setEndValue(1.0);
    m_off_hover->setDuration(200);
    m_off_hover->setEasingCurve(QEasingCurve::Linear);

    addAnimation(m_off_hover);

    connect(m_off_hover, &QVariantAnimation::valueChanged, w, [=](const QVariant &value) {
       w->update();
    });
    connect(m_off_hover, &QVariantAnimation::finished, w, [=]() {
        w->update();
    });

    //OnHover 动画初始化
    m_on_hover = new QVariantAnimation(this);
    m_on_hover->setStartValue(1.0);
    m_on_hover->setEndValue(1.4);
    m_on_hover->setDuration(200);
    m_on_hover->setEasingCurve(QEasingCurve::Linear);

    addAnimation(m_on_hover);

    connect(m_on_hover, &QVariantAnimation::valueChanged, w, [=](const QVariant &value) {
       w->update();
    });
    connect(m_on_hover, &QVariantAnimation::finished, w, [=]() {
        w->update();
    });

    //m_sunken_onhover动画初始化
    m_sunken_onhover = new QVariantAnimation(this);
    m_sunken_onhover->setStartValue(1.0);
    m_sunken_onhover->setEndValue(1.2);
    m_sunken_onhover->setDuration(200);
    m_sunken_onhover->setEasingCurve(QEasingCurve::Linear);

    addAnimation(m_sunken_onhover);

    connect(m_sunken_onhover, &QVariantAnimation::valueChanged, w, [=](const QVariant &value) {
       w->update();
    });
    connect(m_sunken_onhover, &QVariantAnimation::finished, w, [=]() {
        w->update();
    });


    return true;
}

bool ConfigRadioButtonAnimator::unboundWidget()
{
    this->stop();
    this->setDirection(QAbstractAnimation::Forward);
    if (m_off) {
        m_off->deleteLater();
        m_off = nullptr;
    }
    if (m_sunkenon) {
        m_sunkenon->deleteLater();
        m_sunkenon = nullptr;
    }

    if (m_off_hover) {
        m_off_hover->deleteLater();
        m_off_hover = nullptr;
    }
    if (m_on_hover) {
        m_on_hover->deleteLater();
        m_on_hover = nullptr;
    }
    if (m_sunken_onhover) {
        m_sunken_onhover->deleteLater();
        m_sunken_onhover = nullptr;
    }

    if (m_widget) {
        this->setParent(nullptr);
        return true;
    }
    return false;

}

QVariant ConfigRadioButtonAnimator::value(const QString &property)
{
    if (property == "SunKenOn") {
        return m_sunkenon->currentValue();
    } else if (property == "Off") {
        return m_off->currentValue();
    } else if (property == "OffHover") {
        return m_off_hover->currentValue();
    } else if (property == "OnHover") {
        return m_on_hover->currentValue();
    } else if (property == "SunkenOnHover") {
        return m_sunken_onhover->currentValue();
    }

    else {
        return QVariant();
    }
}

bool ConfigRadioButtonAnimator::isRunning(const QString &property)
{
    if (property == "SunKenOn") {
        return m_sunkenon->state() == Running;
    } else if (property == "Off") {
        return m_off->state() == Running;
    } else if (property == "OffHover") {
        return m_off_hover->state() == Running;
    } else if (property == "OnHover") {
        return m_on_hover->state() == Running;
    } else if (property == "SunkenOnHover") {
        return m_sunken_onhover->state() == Running;
    }
    else {
        return this->state() == Running;
    }
}
bool ConfigRadioButtonAnimator::isStopped(const QString &property)
{
    if (property == "SunKenOn") {
        return m_sunkenon->state() == Stopped;
    } else if (property == "Off") {
        return m_off->state() == Stopped;
    } else if (property == "OffHover") {
        return m_off_hover->state() == Stopped;
    } else if (property == "OnHover") {
        return m_on_hover->state() == Stopped;
    } else if (property == "SunkenOnHover") {
        return m_sunken_onhover->state() == Stopped;
    }
    else {
        return this->state() == Stopped;
    }
}

bool ConfigRadioButtonAnimator::setAnimatorStartValue(const QString &property, const QVariant &value)
{
    if (property == "SunKenOn") {
        m_sunkenon->setStartValue(value);
        return true;
    } else if (property == "Off") {
        m_off->setStartValue(value);
        return true;
    } else if (property == "OffHover") {
        m_off_hover->setStartValue(value);
        return true;
    } else if (property == "OnHover") {
        m_on_hover->setStartValue(value);
        return true;
    } else if (property == "SunkenOnHover") {
        m_sunken_onhover->setStartValue(value);
        return true;
    }


    else {
        return false;
    }
}

bool ConfigRadioButtonAnimator::setAnimatorEndValue(const QString &property, const QVariant &value)
{
    if (property == "SunKenOn") {
        m_sunkenon->setEndValue(value);
        return true;
    } else if (property == "Off") {
        m_off->setEndValue(value);
        return true;
    } else if (property == "OffHover") {
        m_off_hover->setEndValue(value);
        return true;
    } else if (property == "OnHover") {
        m_on_hover->setEndValue(value);
        return true;
    } else if (property == "SunkenOnHover") {
        m_sunken_onhover->setEndValue(value);
        return true;
    }

    else {
        return false;
    }
}

bool ConfigRadioButtonAnimator::setAnimatorDuration(const QString &property, int duration)
{
    if (property == "SunKenOn") {
        m_sunkenon->setDuration(duration);
        return true;
    } else if (property == "Off") {
        m_off->setDuration(duration);
        return true;
    } else if (property == "OffHover") {
        m_off_hover->setDuration(duration);
        return true;
    } else if (property == "OnHover") {
        m_on_hover->setDuration(duration);
        return true;
    } else if (property == "SunkenOnHover") {
        m_sunken_onhover->setDuration(duration);

        return true;
    }
    else {
        return false;
    }
}

void ConfigRadioButtonAnimator::setAnimatorDirectionForward(const QString &property, bool forward)
{
    auto d = forward? QAbstractAnimation::Forward: QAbstractAnimation::Backward;
    if (property == "SunKenOn") {
        m_sunkenon->setDirection(d);
    } else if (property == "Off") {
        m_off->setDirection(d);
    } else if (property == "OffHover") {
        m_off_hover->setDirection(d);
    } else if (property == "OnHover") {
        m_on_hover->setDirection(d);
    } else if (property == "SunkenOnHover") {
        m_sunken_onhover->setDirection(d);
    }
    else {
        return;
    }
}
void ConfigRadioButtonAnimator::startAnimator(const QString &property)
{
    if (property == "SunKenOn") {
        m_sunkenon->start();
    } else if (property == "Off") {
        m_off->start();
    } else if (property == "OffHover") {
        m_off_hover->start();
    } else if (property == "OnHover") {
        m_on_hover->start();
    } else if (property == "SunkenOnHover") {
        m_sunken_onhover->start();
    }
    else {
        this->start();
    }
}

void ConfigRadioButtonAnimator::stopAnimator(const QString &property)
{
    if (property == "SunKenOn") {
        m_sunkenon->stop();
        m_sunkenon->setCurrentTime(0);
    } else if (property == "Off") {
        m_off->stop();
        m_off->setCurrentTime(0);
    } else if (property == "OffHover") {
        m_off_hover->stop();
        m_off_hover->setCurrentTime(0);
    } else if (property == "OnHover") {
        m_on_hover->stop();
        m_on_hover->setCurrentTime(0);
    } else if (property == "SunkenOnHover") {
        m_sunken_onhover->stop();
        m_sunken_onhover->setCurrentTime(0);
    }
    else {
        this->stop();
        this->setCurrentTime(0);
    }
}

int ConfigRadioButtonAnimator::currentAnimatorTime(const QString &property)
{
    if (property == "SunKenOn") {
        return m_sunkenon->currentTime();
    }
    if (property == "Off") {
        return m_off->currentTime();
    }
    if (property == "OffHover") {
        return m_off_hover->currentTime();
    }
    if (property == "OnHover") {
        return m_on_hover->currentTime();
    }
    if (property == "SunkenOnHover") {
        return m_sunken_onhover->currentTime();
    }
    else {
        return this->currentTime();
    }
}

void ConfigRadioButtonAnimator::setAnimatorCurrentTime(const QString &property, const int msecs)
{
    if ("SunKenOn" == property) {
        m_sunkenon->setCurrentTime(msecs);
    } else if ("Off" == property) {
        m_off->setCurrentTime(msecs);
    } else if ("OffHover" == property) {
        m_off_hover->setCurrentTime(msecs);
    } else if ("OnHover" == property) {
        m_on_hover->setCurrentTime(msecs);
    } else if ("SunkenOnHover" == property) {
        m_sunken_onhover->setCurrentTime(msecs);
    }
}

int ConfigRadioButtonAnimator::totalAnimationDuration(const QString &property)
{
    if (property == "SunKenOn") {
        return m_sunkenon->duration();
    }
    if (property == "Off") {
        return m_off->duration();
    }
    if (property == "OffHover") {
        return m_off_hover->duration();
    }
    if (property == "OnHover") {
        return m_on_hover->duration();
    }
    if (property == "SunkenOnHover") {
        return m_sunken_onhover->duration();
    } else {
        return this->duration();
    }

}

