/*
 * libkysdk-qtwidgets's Library
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
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#include "kswitchbutton.h"
#include "themeController.h"
#include <QDebug>
#include <QMouseEvent>
#include <QPainterPath>
#include <QStyle>
#include <QStyleOption>
#include <QVariantAnimation>
#include <QtMath>

namespace kdk
{
class KSwitchButtonPrivate : public QObject, public ThemeController
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KSwitchButton)

public:
    KSwitchButtonPrivate(KSwitchButton *parent)
        : q_ptr(parent)
        , m_animation(new QVariantAnimation(parent))
    {
        setParent(parent);
        m_animation->setDuration(200);
        m_animation->setStartValue(0.0);
        m_animation->setEndValue(1.0);
        m_animation->setEasingCurve(QEasingCurve::Linear);
        m_animation->setLoopCount(1);
    }

protected:
    void changeTheme();

private:
    KSwitchButton *q_ptr;

    QVariantAnimation *m_animation;
    int m_space; // 滑块离背景间隔
    int m_rectRadius; // 圆角角度
    int m_startX; // 滑块开始X轴坐标
    int m_endX; // 滑块结束X轴坐标
    QColor m_sliderColor;
    QColor m_bkgStartColor;
    QColor m_bkgEndColor;
    QColor m_borderColor;
    bool m_isHoverd;
    bool m_isTranslucent;
    bool m_isTranslucentFlag;
    QLinearGradient m_linearGradient;
    void updateColor(const QStyleOptionButton &opt);
    void drawBackground(QPainter *painter);
    void drawSlider(QPainter *painter);
    void stepChanged();
};

KSwitchButton::KSwitchButton(QWidget *parent)
    : QPushButton(parent)
    , d_ptr(new KSwitchButtonPrivate(this))

{
    Q_D(KSwitchButton);
    d->m_isHoverd = false;
    d->m_isTranslucent = false;
    d->m_isTranslucentFlag = false;
    this->setCheckable(true);

    d->changeTheme();
    connect(d->m_gsetting,&QGSettings::changed,this,[=](QString key){
        if(key == "styleName" || key == "iconThemeName" || key == "widgetThemeName")
        {
            d->changeTheme();
        }
    });
    connect(d->m_animation, &QVariantAnimation::valueChanged, d, &KSwitchButtonPrivate::stepChanged);

    connect(this, &KSwitchButton::toggled, this, [=](bool flag) {
        d->m_endX = width() > height() ? width() - height() : 0;
        d->m_animation->start();
        Q_EMIT stateChanged(flag);
    });
    d->m_space = 4;
}

KSwitchButton::~KSwitchButton()
{
}

void KSwitchButton::setCheckable(bool flag)
{
    QPushButton::setCheckable(flag);
    this->update();
}

bool KSwitchButton::isCheckable() const
{
    return QPushButton::isCheckable();
}

bool KSwitchButton::isChecked() const
{
    return QPushButton::isChecked();
}

void KSwitchButton::setChecked(bool flag)
{
    Q_D(KSwitchButton);
    QPushButton::setChecked(flag);
    if (signalsBlocked()) {
        d->m_animation->start();
    }
    this->update();
}

void KSwitchButton::setTranslucent(bool flag)
{
    Q_D(KSwitchButton);
    d->m_isTranslucentFlag = flag;
    d->m_isTranslucent = flag;
    d->changeTheme();
}

bool KSwitchButton::isTranslucent()
{
    Q_D(KSwitchButton);
    return d->m_isTranslucent;
}

void KSwitchButton::paintEvent(QPaintEvent *event)
{
    Q_D(KSwitchButton);

    d->m_linearGradient = QLinearGradient(this->width() / 2, 0, this->width() / 2, this->height());
    QStyleOptionButton opt;
    initStyleOption(&opt);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    d->updateColor(opt);
    d->drawBackground(&painter);
    d->drawSlider(&painter);
    painter.drawText(0, 0, width(), height(), Qt::AlignCenter, text());
}

void KSwitchButton::resizeEvent(QResizeEvent *event)
{
    Q_D(KSwitchButton);
    QPushButton::resizeEvent(event);
    if (this->isChecked())
        d->m_startX = width() > height() ? width() - height() : 0;
    else
        d->m_startX = 0;
    d->m_rectRadius = height() / 2;
    d->m_endX = width() > height() ? width() - height() : 0;
}

QSize KSwitchButton::sizeHint() const
{
    return QSize(50, 24);
}

void KSwitchButtonPrivate::changeTheme()
{
    Q_Q(KSwitchButton);
    if (m_isTranslucentFlag) {
        if (ThemeController::widgetTheme() == ClassicTheme)
            m_isTranslucent = false;
        else
            m_isTranslucent = true;
    } else {
        m_isTranslucent = false;
    }
    initThemeStyle();
    q->update();
}

void KSwitchButtonPrivate::updateColor(const QStyleOptionButton &opt)
{
    Q_Q(KSwitchButton);

    // 根据option状态去指定以下两个子部件的颜色
    // m_sliderColor
    // m_linearGradient
    if(q->isChecked())
    {
        ThemeController::getGradientFromDT("kbrand-normal",m_bkgStartColor,m_bkgEndColor);
        m_linearGradient.setColorAt(0,m_bkgStartColor);
        m_linearGradient.setColorAt(1,m_bkgEndColor);
        m_borderColor = ThemeController::getCustomColorFromDT("kline-component-normal");
        m_sliderColor = ThemeController::getCustomColorFromDT("kfont-white");
        if (!opt.state.testFlag(QStyle::State_Enabled)) // disable
        {
            m_sliderColor = ThemeController::getPaletteColorFromDT("kfont-white-disable");
            // m_bkgStartColor=ThemeController::getPaletteColorFromDT("kcomponent-disable");
            ThemeController::getGradientFromDT("kbrand-disable",m_bkgStartColor,m_bkgEndColor);
            m_linearGradient.setColorAt(0,m_bkgStartColor);
            m_linearGradient.setColorAt(1,m_bkgEndColor);
            m_borderColor = ThemeController::getCustomColorFromDT("kline-component-disable");
            return;
        }
        if (opt.state.testFlag(QStyle::State_MouseOver))
        {
            if (opt.state & QStyle::State_Sunken) // clicked
            {
                m_isHoverd = false;
                ThemeController::getGradientFromDT("kbrand-click",m_bkgStartColor,m_bkgEndColor);
                m_linearGradient.setColorAt(0,m_bkgStartColor);
                m_linearGradient.setColorAt(1,m_bkgEndColor);
                m_borderColor = ThemeController::getCustomColorFromDT("kline-component-click");
            }
            else // hover
            {
                m_isHoverd = true;
                ThemeController::getGradientFromDT("kbrand-hover",m_bkgStartColor,m_bkgEndColor);
                m_linearGradient.setColorAt(0,m_bkgStartColor);
                m_linearGradient.setColorAt(1,m_bkgEndColor);
                m_borderColor = ThemeController::getCustomColorFromDT("kline-component-hover");
            }
        }
    }
    else
    {
        if(m_isTranslucent)
        {
            ThemeController::getGradientFromDT("kcomponent-alpha-normal",m_bkgStartColor,m_bkgEndColor);
            m_linearGradient.setColorAt(0,m_bkgStartColor);
            m_linearGradient.setColorAt(1,m_bkgEndColor);
            m_borderColor = ThemeController::getCustomColorFromDT("kline-component-normal");
            m_sliderColor = ThemeController::getCustomColorFromDT("kfont-white");
            if (!opt.state.testFlag(QStyle::State_Enabled)) // disable
            {
                m_sliderColor = ThemeController::getPaletteColorFromDT("kfont-white-disable");
                ThemeController::getGradientFromDT("kcomponent-alpha-disable",m_bkgStartColor,m_bkgEndColor);
                m_linearGradient.setColorAt(0,m_bkgStartColor);
                m_linearGradient.setColorAt(1,m_bkgEndColor);
                m_borderColor = ThemeController::getCustomColorFromDT("kline-component-disable");
                return;
            }
            if (opt.state.testFlag(QStyle::State_MouseOver))
            {
                if (opt.state & QStyle::State_Sunken) // clicked
                {
                    m_isHoverd = false;
                    ThemeController::getGradientFromDT("kcomponent-alpha-click",m_bkgStartColor,m_bkgEndColor);
                    m_linearGradient.setColorAt(0,m_bkgStartColor);
                    m_linearGradient.setColorAt(1,m_bkgEndColor);
                    m_borderColor = ThemeController::getCustomColorFromDT("kline-component-click");
                }
                else // hover
                {
                    m_isHoverd = true;
                    ThemeController::getGradientFromDT("kcomponent-alpha-hover",m_bkgStartColor,m_bkgEndColor);
                    m_linearGradient.setColorAt(0,m_bkgStartColor);
                    m_linearGradient.setColorAt(1,m_bkgEndColor);
                    m_borderColor = ThemeController::getCustomColorFromDT("kline-component-hover");
                }
            }
        }
        else
        {
            ThemeController::getGradientFromDT("kcomponent-normal",m_bkgStartColor,m_bkgEndColor);
            m_linearGradient.setColorAt(0,m_bkgStartColor);
            m_linearGradient.setColorAt(1,m_bkgEndColor);
            m_borderColor = ThemeController::getCustomColorFromDT("kline-component-normal");
            m_sliderColor = ThemeController::getCustomColorFromDT("kfont-white");
            if (!opt.state.testFlag(QStyle::State_Enabled)) // disable
            {
                m_sliderColor = ThemeController::getPaletteColorFromDT("kfont-white-disable");
                ThemeController::getGradientFromDT("kcomponent-disable",m_bkgStartColor,m_bkgEndColor);
                m_linearGradient.setColorAt(0,m_bkgStartColor);
                m_linearGradient.setColorAt(1,m_bkgEndColor);
                m_borderColor = ThemeController::getCustomColorFromDT("kline-component-disable");
                return;
            }
            if (opt.state.testFlag(QStyle::State_MouseOver))
            {
                if (opt.state & QStyle::State_Sunken) // clicked
                {
                    m_isHoverd = false;
                    ThemeController::getGradientFromDT("kcomponent-click",m_bkgStartColor,m_bkgEndColor);
                    m_linearGradient.setColorAt(0,m_bkgStartColor);
                    m_linearGradient.setColorAt(1,m_bkgEndColor);
                    m_borderColor = ThemeController::getCustomColorFromDT("kline-component-click");
                }
                else // hover
                {
                    m_isHoverd = true;
                    ThemeController::getGradientFromDT("kcomponent-hover",m_bkgStartColor,m_bkgEndColor);
                    m_linearGradient.setColorAt(0,m_bkgStartColor);
                    m_linearGradient.setColorAt(1,m_bkgEndColor);
                    m_borderColor = ThemeController::getCustomColorFromDT("kline-component-hover");
                }
            }
        }
    }
}

void KSwitchButtonPrivate::drawBackground(QPainter *painter)
{
    Q_Q(KSwitchButton);
    painter->save();
    painter->setPen(m_borderColor);
    painter->setBrush(m_linearGradient);
    QRect rect(0, 0, q->width() , q->height() );

    // 背景框两边是半圆，所以圆角的半径应该固定是矩形高度的一半
    int radius = rect.height() / 2;
    int circleWidth = rect.height();
    QPainterPath path;
    path.moveTo(radius, rect.top());
    path.arcTo(QRectF(rect.left(), rect.top(), circleWidth, circleWidth), 90, 180);
    path.lineTo(rect.width() - radius, rect.height());
    path.arcTo(QRectF(rect.width() - rect.height(), rect.top(), circleWidth, circleWidth), 270, 180);
    path.lineTo(radius, rect.top());
    painter->drawPath(path);
    painter->restore();
}

void KSwitchButtonPrivate::drawSlider(QPainter *painter)
{
    Q_Q(KSwitchButton);
    painter->save();
    painter->setRenderHint(QPainter::HighQualityAntialiasing);
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::TextAntialiasing);
    painter->setRenderHint(QPainter::SmoothPixmapTransform);

    painter->setPen(Qt::NoPen);
    painter->setBrush(m_sliderColor);
    // 滑块的直径
    int sliderWidth = q->height() - m_space * 2;
    QRect sliderRect(m_startX + m_space, m_space, sliderWidth, sliderWidth);
    painter->drawEllipse(sliderRect);
//    if (!q->isEnabled()) {
//        // 绘制disenable状态下的小矩形条
//        if (!q->isChecked()) {
//            int topX = q->width() - sliderWidth / 2 - 10;
//            int topY = (q->height() - sliderWidth / 4) / 2;
//            QRect lineRect(topX, topY, sliderWidth / 2, sliderWidth / 4);
//            int radius = lineRect.height() / 2;
//            painter->drawRoundedRect(lineRect, radius, radius);
//        } else {
//            int topX = 10;
//            int topY = (q->height() - sliderWidth / 4) / 2;
//            QRect lineRect(topX, topY, sliderWidth / 2, sliderWidth / 4);
//            int radius = lineRect.height() / 2;
//            painter->drawRoundedRect(lineRect, radius, radius);
//        }
//    }
    painter->restore();
}

void KSwitchButtonPrivate::stepChanged()
{
    Q_Q(KSwitchButton);
    if (q->isChecked()) {
        if (m_startX < m_endX) {
            m_startX = qCeil(m_endX * m_animation->currentValue().toDouble());
        } else {
            m_startX = m_endX;
            m_animation->stop();
        }
    } else {
        if (m_startX > 0) {
            double percent = 1.0 - m_animation->currentValue().toDouble();
            m_startX = qCeil(m_endX * percent);
        } else {
            m_startX = 0;
            m_animation->stop();
        }
    }
    q->update();
}

}
#include "kswitchbutton.moc"
#include "moc_kswitchbutton.cpp"
