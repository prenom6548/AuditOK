#include <QApplication>
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

#include "ukuicheckbox.h"
#include "qdebug.h"
using namespace UKUIQQC2Style;

UKUICheckBox::UKUICheckBox(QQuickItem *parent) : QQuickItem(parent)
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

UKUICheckBox::~UKUICheckBox()
{

}

UKUICheckBox* UKUICheckBox::qmlAttachedProperties(QObject* parent)
{
    auto p = qobject_cast<QQuickItem*>(parent);
    return new UKUICheckBox(p);
}

void UKUICheckBox::initParam(UKUIGlobalDTConfig::GlobalDTConfig* instance)
{
    setNormalTextColor(instance->kFontPrimary());
    setDisableTextColor(instance->kFontPrimaryDisable());
    setNormalIndicatorColor(instance->kComponentNormal());
    setHoverIndicatorColor(instance->kComponentHover());
    setClickIndicatorColor(instance->kComponentClick());
    setDisableIndicatorColor(instance->kComponentDisable());
    setNormalIndicatorBorderColor(instance->kLineSelectboxNormal());
    setHoverIndicatorBorderColor(instance->kLineSelectboxHover());
    setClickIndicatorBorderColor(instance->kLineSelectboxClick());
    setDisableIndicatorBorderColor(instance->kLineSelectboxDisable());

    setChecked_NormalIndicatorColor(instance->kBrandNormal());
    setChecked_HoverIndicatorColor(instance->kBrandHover());
    setChecked_ClickIndicatorColor(instance->kBrandClick());
    setChecked_DisableIndicatorColor(instance->kBrandDisable());
    setChecked_NormalIndicatorBorderColor(instance->kLineSelectboxNormal());
    setChecked_HoverIndicatorBorderColor(instance->kLineSelectboxHover());
    setChecked_ClickIndicatorBorderColor(instance->kLineSelectboxClick());
    setChecked_DisableIndicatorBorderColor(instance->kLineSelectboxDisable());

    setChecked_NormalChildrenColor(instance->kFontWhite());
    setChecked_HoverChildrenColor(instance->kFontWhite());
    setChecked_ClickChildrenColor(instance->kFontWhite());
    setChecked_DisableChildrenColor(instance->kFontWhiteDisable());

    setNormalTransparentIndicatorColor(instance->kComponentAlphaNormal());
    setHoverTransparentIndicatorColor(instance->kComponentAlphaHover());
    setClickTransparentIndicatorColor(instance->kComponentAlphaClick());
    setDisableTransparentIndicatorColor(instance->kComponentAlphaDisable());
    setNormalTransparentIndicatorBorderColor(instance->kLineSelectboxNormal());
    setHoverTransparentIndicatorBorderColor(instance->kLineSelectboxNormal());
    setClickTransparentIndicatorBorderColor(instance->kLineSelectboxNormal());
    setDisableTransparentIndicatorBorderColor(instance->kLineSelectboxDisable());

    setBorderWidth(instance->normalline());
    setFocusBorderWidth(instance->focusline());
    setSpace(8);
    setIndicatorWidth(16);
    setChildrenWidth(8);
    setRadius(instance->kradiusMin());
    setLeftRightMargin(0);

    emit parametryChanged();

}



double UKUICheckBox::leftRightMargin() const
{
    return m_leftRightMargin;
}

void UKUICheckBox::setLeftRightMargin(double newLeftRightMargin)
{
    if (qFuzzyCompare(m_leftRightMargin, newLeftRightMargin))
        return;
    m_leftRightMargin = newLeftRightMargin;
    emit leftRightMarginChanged();
}

double UKUICheckBox::upDownMargin() const
{
    return m_upDownMargin;
}

void UKUICheckBox::setUpDownMargin(double newUpDownMargin)
{
    if (qFuzzyCompare(m_upDownMargin, newUpDownMargin))
        return;
    m_upDownMargin = newUpDownMargin;
    emit upDownMarginChanged();
}

double UKUICheckBox::space() const
{
    return m_space;
}

void UKUICheckBox::setSpace(double newSpace)
{
    if (qFuzzyCompare(m_space, newSpace))
        return;
    m_space = newSpace;
    emit spaceChanged();
}

QBrush UKUICheckBox::disableTextColor() const
{
    return m_disableTextColor;
}

void UKUICheckBox::setDisableTextColor(QBrush newDisableTextColor)
{
    if (m_disableTextColor != newDisableTextColor){
        m_disableTextColor = newDisableTextColor;
    emit disableTextColorChanged();
    }
}

QBrush UKUICheckBox::normalTextColor() const
{
    return m_normalTextColor;
}

void UKUICheckBox::setNormalTextColor(QBrush newNormalTextColor)
{
    if (m_normalTextColor != newNormalTextColor){
    m_normalTextColor = newNormalTextColor;
    emit normalTextColorChanged();
    }
}

QBrush UKUICheckBox::normalIndicatorColor() const
{
    return m_normalIndicatorColor;
}

void UKUICheckBox::setNormalIndicatorColor(QBrush newNormalIndicatorColor)
{
    if (m_normalIndicatorColor != newNormalIndicatorColor){
    m_normalIndicatorColor = newNormalIndicatorColor;
    emit normalIndicatorColorChanged();
    }
}

QBrush UKUICheckBox::hoverIndicatorColor() const
{
    return m_hoverIndicatorColor;
}

void UKUICheckBox::setHoverIndicatorColor(QBrush newHoverIndicatorColor)
{
    if (m_hoverIndicatorColor != newHoverIndicatorColor){
    m_hoverIndicatorColor = newHoverIndicatorColor;
    emit hoverIndicatorColorChanged();
    }
}

QBrush UKUICheckBox::clickIndicatorColor() const
{
    return m_clickIndicatorColor;
}

void UKUICheckBox::setClickIndicatorColor(QBrush newClickIndicatorColor)
{
    if (m_clickIndicatorColor != newClickIndicatorColor){
    m_clickIndicatorColor = newClickIndicatorColor;
    emit clickIndicatorColorChanged();
    }
}

QBrush UKUICheckBox::disableIndicatorColor() const
{
    return m_disableIndicatorColor;
}

void UKUICheckBox::setDisableIndicatorColor(QBrush newDisableIndicatorColor)
{
    if (m_disableIndicatorColor != newDisableIndicatorColor){
    m_disableIndicatorColor = newDisableIndicatorColor;
    emit disableIndicatorColorChanged();
    }
}

QBrush UKUICheckBox::normalIndicatorBorderColor() const
{
    return m_normalIndicatorBorderColor;
}

void UKUICheckBox::setNormalIndicatorBorderColor(QBrush newNormalIndicatorBorderColor)
{
    if (m_normalIndicatorBorderColor != newNormalIndicatorBorderColor){
    m_normalIndicatorBorderColor = newNormalIndicatorBorderColor;
    emit normalIndicatorBorderColorChanged();
    }
}

QBrush UKUICheckBox::hoverIndicatorBorderColor() const
{
    return m_hoverIndicatorBorderColor;
}

void UKUICheckBox::setHoverIndicatorBorderColor(QBrush newHoverIndicatorBorderColor)
{
    if (m_hoverIndicatorBorderColor != newHoverIndicatorBorderColor){
    m_hoverIndicatorBorderColor = newHoverIndicatorBorderColor;
    emit hoverIndicatorBorderColorChanged();
    }
}

QBrush UKUICheckBox::clickIndicatorBorderColor() const
{
    return m_clickIndicatorBorderColor;
}

void UKUICheckBox::setClickIndicatorBorderColor(QBrush newClickIndicatorBorderColor)
{
    if (m_clickIndicatorBorderColor != newClickIndicatorBorderColor){
    m_clickIndicatorBorderColor = newClickIndicatorBorderColor;
    emit clickIndicatorBorderColorChanged();
    }
}

QBrush UKUICheckBox::disableIndicatorBorderColor() const
{
    return m_disableIndicatorBorderColor;
}

void UKUICheckBox::setDisableIndicatorBorderColor(QBrush newDisableIndicatorBorderColor)
{
    if (m_disableIndicatorBorderColor != newDisableIndicatorBorderColor){
    m_disableIndicatorBorderColor = newDisableIndicatorBorderColor;
    emit disableIndicatorBorderColorChanged();
    }
}

QBrush UKUICheckBox::checked_normalIndicatorColor() const
{
    return m_checked_normalIndicatorColor;
}

QBrush UKUICheckBox::checked_hoverIndicatorColor() const
{
    return m_checked_hoverIndicatorColor;
}

QBrush UKUICheckBox::checked_clickIndicatorColor() const
{
    return m_checked_clickIndicatorColor;
}

QBrush UKUICheckBox::checked_disableIndicatorColor() const
{
    return m_checked_disableIndicatorColor;
}

QBrush UKUICheckBox::checked_normalIndicatorBorderColor() const
{
    return m_checked_normalIndicatorBorderColor;
}

QBrush UKUICheckBox::checked_hoverIndicatorBorderColor() const
{
    return m_checked_hoverIndicatorBorderColor;
}

QBrush UKUICheckBox::checked_clickIndicatorBorderColor() const
{
    return m_checked_clickIndicatorBorderColor;
}

QBrush UKUICheckBox::checked_disableIndicatorBorderColor() const
{
    return m_checked_disableIndicatorBorderColor;
}

QBrush UKUICheckBox::checked_normalChildrenColor() const
{
    return m_checked_normalChildrenColor;
}

QBrush UKUICheckBox::checked_hoverChildrenColor() const
{
    return m_checked_hoverChildrenColor;
}

QBrush UKUICheckBox::checked_clickChildrenColor() const
{
    return m_checked_clickChildrenColor;
}


QBrush UKUICheckBox::checked_disableChildrenColor() const
{
    return m_checked_disableChildrenColor;
}

QBrush UKUICheckBox::checked_normalChildrenBorderColor() const
{
    return m_checked_normalChildrenBorderColor;
}

QBrush UKUICheckBox::checked_hoverChildrenBorderColor() const
{
    return m_checked_hoverChildrenBorderColor;
}

QBrush UKUICheckBox::checked_clickChildrenBorderColor() const
{
    return m_checked_clickChildrenBorderColor;
}


QBrush UKUICheckBox::checked_disableChildrenBorderColor() const
{
    return m_checked_disableChildrenBorderColor;
}


void UKUICheckBox::setChecked_NormalIndicatorColor(QBrush newChecked_normalIndicatorColor)
{
    if (m_checked_normalIndicatorColor != newChecked_normalIndicatorColor){
    m_checked_normalIndicatorColor = newChecked_normalIndicatorColor;
    emit checked_normalIndicatorColorChanged();
    }
}

void UKUICheckBox::setChecked_HoverIndicatorColor(QBrush newChecked_hoverIndicatorColor)
{
    if (m_checked_hoverIndicatorColor != newChecked_hoverIndicatorColor){
    m_checked_hoverIndicatorColor = newChecked_hoverIndicatorColor;
    emit checked_hoverIndicatorColorChanged();
    }
}

void UKUICheckBox::setChecked_ClickIndicatorColor(QBrush newChecked_clickIndicatorColor)
{
    if (m_checked_clickIndicatorColor != newChecked_clickIndicatorColor){
    m_checked_clickIndicatorColor = newChecked_clickIndicatorColor;
    emit checked_clickIndicatorColorChanged();
    }
}

void UKUICheckBox::setChecked_DisableIndicatorColor(QBrush newChecked_disableIndicatorColor)
{
    if (m_checked_disableIndicatorColor != newChecked_disableIndicatorColor){
    m_checked_disableIndicatorColor = newChecked_disableIndicatorColor;
    emit checked_disableIndicatorColorChanged();
    }
}

void UKUICheckBox::setChecked_NormalIndicatorBorderColor(QBrush newChecked_normalIndicatorBorderColor)
{
    if (m_checked_normalIndicatorBorderColor != newChecked_normalIndicatorBorderColor){
    m_checked_normalIndicatorBorderColor = newChecked_normalIndicatorBorderColor;
    emit checked_normalIndicatorBorderColorChanged();
    }
}

void UKUICheckBox::setChecked_HoverIndicatorBorderColor(QBrush newChecked_hoverIndicatorBorderColor)
{
    if (m_checked_hoverIndicatorBorderColor != newChecked_hoverIndicatorBorderColor){
    m_checked_hoverIndicatorBorderColor = newChecked_hoverIndicatorBorderColor;
    emit checked_hoverIndicatorBorderColorChanged();
    }
}

void UKUICheckBox::setChecked_ClickIndicatorBorderColor(QBrush newChecked_clickIndicatorBorderColor)
{
    if (m_checked_clickIndicatorBorderColor != newChecked_clickIndicatorBorderColor){
    m_checked_clickIndicatorBorderColor = newChecked_clickIndicatorBorderColor;
    emit checked_clickIndicatorBorderColorChanged();
    }
}

void UKUICheckBox::setChecked_DisableIndicatorBorderColor(QBrush newChecked_disableIndicatorBorderColor)
{
    if (m_checked_disableIndicatorBorderColor != newChecked_disableIndicatorBorderColor){
    m_checked_disableIndicatorBorderColor = newChecked_disableIndicatorBorderColor;
    emit checked_disableIndicatorBorderColorChanged();
    }
}

void UKUICheckBox::setChecked_NormalChildrenColor(QBrush newChecked_normalChildrenColor)
{
    if (m_checked_normalChildrenColor != newChecked_normalChildrenColor){
    m_checked_normalChildrenColor = newChecked_normalChildrenColor;
    emit checked_normalChildrenColorChanged();
    }
}

void UKUICheckBox::setChecked_HoverChildrenColor(QBrush newChecked_hoverChildrenColor)
{
    if (m_checked_hoverChildrenColor != newChecked_hoverChildrenColor){
    m_checked_hoverChildrenColor = newChecked_hoverChildrenColor;
    emit checked_hoverChildrenColorChanged();
    }
}

void UKUICheckBox::setChecked_ClickChildrenColor(QBrush newChecked_clickChildrenColor)
{
    if (m_checked_clickChildrenColor != newChecked_clickChildrenColor){
    m_checked_clickChildrenColor = newChecked_clickChildrenColor;
    emit checked_clickChildrenColorChanged();
    }
}

void UKUICheckBox::setChecked_DisableChildrenColor(QBrush newChecked_disableChildrenColor)
{
    if (m_checked_disableChildrenColor != newChecked_disableChildrenColor){
    m_checked_disableChildrenColor = newChecked_disableChildrenColor;
    emit checked_disableChildrenColorChanged();
    }
}

void UKUICheckBox::setChecked_NormalChildrenBorderColor(QBrush newChecked_normalChildrenBorderColor)
{
    if (m_checked_normalChildrenBorderColor != newChecked_normalChildrenBorderColor){
    m_checked_normalChildrenBorderColor = newChecked_normalChildrenBorderColor;
    emit checked_normalChildrenBorderColorChanged();
    }
}

void UKUICheckBox::setChecked_HoverChildrenBorderColor(QBrush newChecked_hoverChildrenBorderColor)
{
    if (m_checked_hoverChildrenBorderColor != newChecked_hoverChildrenBorderColor){
    m_checked_hoverChildrenBorderColor = newChecked_hoverChildrenBorderColor;
    emit checked_hoverChildrenBorderColorChanged();
    }
}

void UKUICheckBox::setChecked_ClickChildrenBorderColor(QBrush newChecked_clickChildrenBorderColor)
{
    if (m_checked_clickChildrenBorderColor != newChecked_clickChildrenBorderColor){
    m_checked_clickChildrenBorderColor = newChecked_clickChildrenBorderColor;
    emit checked_clickChildrenBorderColorChanged();
    }
}

void UKUICheckBox::setChecked_DisableChildrenBorderColor(QBrush newChecked_disableChildrenBorderColor)
{
    if (m_checked_disableChildrenBorderColor != newChecked_disableChildrenBorderColor){
    m_checked_disableChildrenBorderColor = newChecked_disableChildrenBorderColor;
    emit checked_disableChildrenBorderColorChanged();
    }
}

int UKUICheckBox::borderWidth() const
{
    return m_borderWidth;
}

void UKUICheckBox::setBorderWidth(int newBorderWidth)
{
    if (m_borderWidth == newBorderWidth)
        return;
    m_borderWidth = newBorderWidth;
    emit borderWidthChanged();
}

int UKUICheckBox::indicatorWidth() const
{
    return m_indicatorWidth;
}

void UKUICheckBox::setIndicatorWidth(int newIndicatorWidth)
{
    if (m_indicatorWidth == newIndicatorWidth)
        return;
    m_indicatorWidth = newIndicatorWidth;
    emit indicatorWidthChanged();
}

int UKUICheckBox::childrenWidth() const
{
    return m_childrenWidth;
}

void UKUICheckBox::setChildrenWidth(int newChildrenWidth)
{
    if (m_childrenWidth == newChildrenWidth)
        return;
    m_childrenWidth = newChildrenWidth;
    emit childrenWidthChanged();
}

int UKUICheckBox::radius() const
{
    return m_radius;
}

void UKUICheckBox::setRadius(int newRadius)
{
    if (m_radius == newRadius)
        return;
    m_radius = newRadius;
    emit radiusChanged();
}

const QBrush &UKUICheckBox::normalTransparentIndicatorColor() const
{
    return m_normalTransparentIndicatorColor;
}

void UKUICheckBox::setNormalTransparentIndicatorColor(const QBrush &newNormalTransparentIndicatorColor)
{
    if (m_normalTransparentIndicatorColor == newNormalTransparentIndicatorColor)
        return;
    m_normalTransparentIndicatorColor = newNormalTransparentIndicatorColor;
    emit normalTransparentIndicatorColorChanged();
}

const QBrush &UKUICheckBox::hoverTransparentIndicatorColor() const
{
    return m_hoverTransparentIndicatorColor;
}

void UKUICheckBox::setHoverTransparentIndicatorColor(const QBrush &newHoverTransparentIndicatorColor)
{
    if (m_hoverTransparentIndicatorColor == newHoverTransparentIndicatorColor)
        return;
    m_hoverTransparentIndicatorColor = newHoverTransparentIndicatorColor;
    emit hoverTransparentIndicatorColorChanged();
}

const QBrush &UKUICheckBox::clickTransparentIndicatorColor() const
{
    return m_clickTransparentIndicatorColor;
}

void UKUICheckBox::setClickTransparentIndicatorColor(const QBrush &newClickTransparentIndicatorColor)
{
    if (m_clickTransparentIndicatorColor == newClickTransparentIndicatorColor)
        return;
    m_clickTransparentIndicatorColor = newClickTransparentIndicatorColor;
    emit clickTransparentIndicatorColorChanged();
}

const QBrush &UKUICheckBox::disableTransparentIndicatorColor() const
{
    return m_disableTransparentIndicatorColor;
}

void UKUICheckBox::setDisableTransparentIndicatorColor(const QBrush &newDisableTransparentIndicatorColor)
{
    if (m_disableTransparentIndicatorColor == newDisableTransparentIndicatorColor)
        return;
    m_disableTransparentIndicatorColor = newDisableTransparentIndicatorColor;
    emit disableTransparentIndicatorColorChanged();
}

const QBrush &UKUICheckBox::normalTransparentIndicatorBorderColor() const
{
    return m_normalTransparentIndicatorBorderColor;
}

void UKUICheckBox::setNormalTransparentIndicatorBorderColor(const QBrush &newNormalTransparentIndicatorBorderColor)
{
    if (m_normalTransparentIndicatorBorderColor == newNormalTransparentIndicatorBorderColor)
        return;
    m_normalTransparentIndicatorBorderColor = newNormalTransparentIndicatorBorderColor;
    emit normalTransparentIndicatorBorderColorChanged();
}

const QBrush &UKUICheckBox::hoverTransparentIndicatorBorderColor() const
{
    return m_hoverTransparentIndicatorBorderColor;
}

void UKUICheckBox::setHoverTransparentIndicatorBorderColor(const QBrush &newHoverTransparentIndicatorBorderColor)
{
    if (m_hoverTransparentIndicatorBorderColor == newHoverTransparentIndicatorBorderColor)
        return;
    m_hoverTransparentIndicatorBorderColor = newHoverTransparentIndicatorBorderColor;
    emit hoverTransparentIndicatorBorderColorChanged();
}

const QBrush &UKUICheckBox::clickTransparentIndicatorBorderColor() const
{
    return m_clickTransparentIndicatorBorderColor;
}

void UKUICheckBox::setClickTransparentIndicatorBorderColor(const QBrush &newClickTransparentIndicatorBorderColor)
{
    if (m_clickTransparentIndicatorBorderColor == newClickTransparentIndicatorBorderColor)
        return;
    m_clickTransparentIndicatorBorderColor = newClickTransparentIndicatorBorderColor;
    emit clickTransparentIndicatorBorderColorChanged();
}

const QBrush &UKUICheckBox::disableTransparentIndicatorBorderColor() const
{
    return m_disableTransparentIndicatorBorderColor;
}

void UKUICheckBox::setDisableTransparentIndicatorBorderColor(const QBrush &newDisableTransparentIndicatorBorderColor)
{
    if (m_disableTransparentIndicatorBorderColor == newDisableTransparentIndicatorBorderColor)
        return;
    m_disableTransparentIndicatorBorderColor = newDisableTransparentIndicatorBorderColor;
    emit disableTransparentIndicatorBorderColorChanged();
}

int UKUICheckBox::focusBorderWidth() const
{
    return m_focusBorderWidth;
}

void UKUICheckBox::setFocusBorderWidth(int newFocusBorderWidth)
{
    if (m_focusBorderWidth == newFocusBorderWidth)
        return;
    m_focusBorderWidth = newFocusBorderWidth;
    emit focusBorderWidthChanged();
}
