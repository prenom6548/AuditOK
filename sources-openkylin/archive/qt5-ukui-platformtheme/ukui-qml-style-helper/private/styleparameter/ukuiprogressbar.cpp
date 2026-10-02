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
 */

#include "ukuiprogressbar.h"
#include "qdebug.h"

using namespace UKUIQQC2Style;

UKUIProgressBar::UKUIProgressBar(QQuickItem *parent) : QQuickItem(parent)
{
    if(!qApp || !qApp->property("qqc2-globaltoken").isValid())
        return;
    TokenParameter * token = qApp->property("qqc2-globaltoken").value<TokenParameter *>();
    if(token && token->getInstance()){
        m_instance = token->getInstance();
        initParam(m_instance);
        connect(m_instance, &UKUIGlobalDTConfig::GlobalDTConfig::tokenChanged, this, [=](){
            initParam(m_instance);
        }, Qt::UniqueConnection);
    }
}

UKUIProgressBar::~UKUIProgressBar()
{

}

void UKUIProgressBar::initParam(UKUIGlobalDTConfig::GlobalDTConfig* instance)
{
    setRadius(instance->kradiusNormal());
    setNormalColor(instance->kComponentNormal());
    setChildrenColor(instance->kBrandNormal());
    setNormalWidth(243);
    setNormalHeight(24);
    setIndeterminateChildrenWidth(48);
    setBorderWidth(1);
    setBorderColor(instance->kLineComponentNormal());
    setHightlightTextColor(instance->kFontWhite());
    setNormalTransparentColor(instance->kComponentAlphaNormal());
    setTransparentBorderColor(instance->kLineComponentNormal());


    QColor baseColor = instance->kBrandNormal().color();
    // 获取颜色分量
    int r = baseColor.red();
    int g = baseColor.green();
    int b = baseColor.blue();

    r = calculateHighColor(r);
    g = calculateHighColor(g);
    b = calculateHighColor(b);

    QColor color;
    color.setRed(r);
    color.setGreen(g);
    color.setBlue(b);

    QLinearGradient linearGradient;
    linearGradient.setColorAt(1, baseColor);
    linearGradient.setColorAt(0, color);
    setIndeterminateColor(QBrush(linearGradient));

    emit parametryChanged();


}

UKUIProgressBar* UKUIProgressBar::qmlAttachedProperties(QObject *parent)
{
    auto p = qobject_cast<QQuickItem*>(parent);
    return new UKUIProgressBar(p);
}

const QBrush &UKUIProgressBar::normalColor() const
{
    return m_normalColor;
}

void UKUIProgressBar::setNormalColor(const QBrush &newNormalColor)
{
    if (m_normalColor == newNormalColor)
        return;
    m_normalColor = newNormalColor;
    emit normalColorChanged();
}

int UKUIProgressBar::radius() const
{
    return m_radius;
}

void UKUIProgressBar::setRadius(int newRadius)
{
    if (m_radius == newRadius)
        return;
    m_radius = newRadius;
    emit radiusChanged();
}

const QBrush &UKUIProgressBar::childrenColor() const
{
    return m_childrenColor;
}

void UKUIProgressBar::setChildrenColor(const QBrush &newChildrenColor)
{
    if (m_childrenColor == newChildrenColor)
        return;
    m_childrenColor = newChildrenColor;
    emit childrenColorChanged();
}

int UKUIProgressBar::normalWidth() const
{
    return m_normalWidth;
}

void UKUIProgressBar::setNormalWidth(int newNormalWidth)
{
    if (m_normalWidth == newNormalWidth)
        return;
    m_normalWidth = newNormalWidth;
    emit normalWidthChanged();
}

int UKUIProgressBar::normalHeight() const
{
    return m_normalHeight;
}

void UKUIProgressBar::setNormalHeight(int newNormalHeight)
{
    if (m_normalHeight == newNormalHeight)
        return;
    m_normalHeight = newNormalHeight;
    emit normalHeightChanged();
}

int UKUIProgressBar::indeterminateChildrenWidth() const
{
    return m_indeterminateChildrenWidth;
}

void UKUIProgressBar::setIndeterminateChildrenWidth(int newIndeterminateChildrenWidth)
{
    if (m_indeterminateChildrenWidth == newIndeterminateChildrenWidth)
        return;
    m_indeterminateChildrenWidth = newIndeterminateChildrenWidth;
    emit indeterminateChildrenWidthChanged();
}

const QBrush &UKUIProgressBar::indeterminateChildrenStartColor() const
{
    return m_indeterminateChildrenStartColor;
}

void UKUIProgressBar::setIndeterminateChildrenStartColor(const QBrush &newIndeterminateChildrenStartColor)
{
    if (m_indeterminateChildrenStartColor == newIndeterminateChildrenStartColor)
        return;
    m_indeterminateChildrenStartColor = newIndeterminateChildrenStartColor;
    emit indeterminateChildrenStartColorChanged();
}

const QBrush &UKUIProgressBar::indeterminateChildrenEndColor() const
{
    return m_indeterminateChildrenEndColor;
}

void UKUIProgressBar::setIndeterminateChildrenEndColor(const QBrush &newIndeterminateChildrenEndColor)
{
    if (m_indeterminateChildrenEndColor == newIndeterminateChildrenEndColor)
        return;
    m_indeterminateChildrenEndColor = newIndeterminateChildrenEndColor;
    emit indeterminateChildrenEndColorChanged();
}

int UKUIProgressBar::borderWidth() const
{
    return m_borderWidth;
}

void UKUIProgressBar::setBorderWidth(int newBorderWidth)
{
    if (m_borderWidth == newBorderWidth)
        return;
    m_borderWidth = newBorderWidth;
    emit borderWidthChanged();
}

const QBrush &UKUIProgressBar::borderColor() const
{
    return m_borderColor;
}

void UKUIProgressBar::setBorderColor(const QBrush &newBorderColor)
{
    if (m_borderColor == newBorderColor)
        return;
    m_borderColor = newBorderColor;
    emit borderColorChanged();
}

const QBrush &UKUIProgressBar::hightlightTextColor() const
{
    return m_hightlightTextColor;
}

void UKUIProgressBar::setHightlightTextColor(const QBrush &newHightlightTextColor)
{
    if (m_hightlightTextColor == newHightlightTextColor)
        return;
    m_hightlightTextColor = newHightlightTextColor;
    emit hightlightTextColorChanged();
}

const QBrush &UKUIProgressBar::normalTransparentColor() const
{
    return m_normalTransparentColor;
}

void UKUIProgressBar::setNormalTransparentColor(const QBrush &newNormalTransparentColor)
{
    if (m_normalTransparentColor == newNormalTransparentColor)
        return;
    m_normalTransparentColor = newNormalTransparentColor;
    emit normalTransparentColorChanged();
}

const QBrush &UKUIProgressBar::transparentborderColor() const
{
    return m_transparentborderColor;
}

void UKUIProgressBar::setTransparentBorderColor(const QBrush &newTransparentborderColor)
{
    if (m_transparentborderColor == newTransparentborderColor)
        return;
    m_transparentborderColor = newTransparentborderColor;
    emit transparentBorderColorChanged();
}

const QBrush &UKUIProgressBar::indeterminateColor() const
{
    return m_indeterminateColor;
}

void UKUIProgressBar::setIndeterminateColor(const QBrush &newIndeterminateColor)
{
    m_indeterminateColor = newIndeterminateColor;
    emit indeterminateColorChanged();

}

int UKUIProgressBar::calculateHighColor(int c)
{
    if(c > 128)
        return 255 - (255 - c) * (255 - 255) * 1.0 / 128.0;
    else
        return c * 255 * 1.0 / 128 * 1.0;
}
