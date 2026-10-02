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

#include "ukuiradiobutton.h"
#include "qdebug.h"
using namespace UKUIQQC2Style;

UKUIRadioButton::UKUIRadioButton(QQuickItem *parent) : QQuickItem(parent)
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

UKUIRadioButton::~UKUIRadioButton()
{

}

UKUIRadioButton* UKUIRadioButton::qmlAttachedProperties(QObject* parent)
{
    auto p = qobject_cast<QQuickItem*>(parent);
    return new UKUIRadioButton(p);
}

void UKUIRadioButton::initParam(UKUIGlobalDTConfig::GlobalDTConfig* instance)
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
    setHoverTransparentIndicatorBorderColor(instance->kLineSelectboxHover());
    setClickTransparentIndicatorBorderColor(instance->kLineSelectboxClick());
    setDisableTransparentIndicatorBorderColor(instance->kLineSelectboxDisable());

    setBorderWidth(instance->normalline());
    setSpace(8);
    setIndicatorWidth(16);
    setChildrenWidth(6);

    emit parametryChanged();

}



double UKUIRadioButton::leftRightMargin() const
{
    return m_leftRightMargin;
}

void UKUIRadioButton::setLeftRightMargin(double newLeftRightMargin)
{
    if (qFuzzyCompare(m_leftRightMargin, newLeftRightMargin))
        return;
    m_leftRightMargin = newLeftRightMargin;
    emit leftRightMarginChanged();
}

double UKUIRadioButton::upDownMargin() const
{
    return m_upDownMargin;
}

void UKUIRadioButton::setUpDownMargin(double newUpDownMargin)
{
    if (qFuzzyCompare(m_upDownMargin, newUpDownMargin))
        return;
    m_upDownMargin = newUpDownMargin;
    emit upDownMarginChanged();
}

double UKUIRadioButton::space() const
{
    return m_space;
}

void UKUIRadioButton::setSpace(double newSpace)
{
    if (qFuzzyCompare(m_space, newSpace))
        return;
    m_space = newSpace;
    emit spaceChanged();
}

const QBrush &UKUIRadioButton::disableTextColor() const
{
    return m_disableTextColor;
}

void UKUIRadioButton::setDisableTextColor(const QBrush &newDisableTextColor)
{
    if (m_disableTextColor == newDisableTextColor)
        return;
    m_disableTextColor = newDisableTextColor;
    emit disableTextColorChanged();
}

const QBrush &UKUIRadioButton::normalTextColor() const
{
    return m_normalTextColor;
}

void UKUIRadioButton::setNormalTextColor(const QBrush &newNormalTextColor)
{
    if (m_normalTextColor == newNormalTextColor)
        return;
    m_normalTextColor = newNormalTextColor;
    emit normalTextColorChanged();
}

const QBrush &UKUIRadioButton::normalIndicatorColor() const
{
    return m_normalIndicatorColor;
}

void UKUIRadioButton::setNormalIndicatorColor(const QBrush &newNormalIndicatorColor)
{
    if (m_normalIndicatorColor == newNormalIndicatorColor)
        return;
    m_normalIndicatorColor = newNormalIndicatorColor;
    emit normalIndicatorColorChanged();
}

const QBrush &UKUIRadioButton::hoverIndicatorColor() const
{
    return m_hoverIndicatorColor;
}

void UKUIRadioButton::setHoverIndicatorColor(const QBrush &newHoverIndicatorColor)
{
    if (m_hoverIndicatorColor == newHoverIndicatorColor)
        return;
    m_hoverIndicatorColor = newHoverIndicatorColor;
    emit hoverIndicatorColorChanged();
}

const QBrush &UKUIRadioButton::clickIndicatorColor() const
{
    return m_clickIndicatorColor;
}

void UKUIRadioButton::setClickIndicatorColor(const QBrush &newClickIndicatorColor)
{
    if (m_clickIndicatorColor == newClickIndicatorColor)
        return;
    m_clickIndicatorColor = newClickIndicatorColor;
    emit clickIndicatorColorChanged();
}

const QBrush &UKUIRadioButton::disableIndicatorColor() const
{
    return m_disableIndicatorColor;
}

void UKUIRadioButton::setDisableIndicatorColor(const QBrush &newDisableIndicatorColor)
{
    if (m_disableIndicatorColor == newDisableIndicatorColor)
        return;
    m_disableIndicatorColor = newDisableIndicatorColor;
    emit disableIndicatorColorChanged();
}

const QBrush &UKUIRadioButton::normalIndicatorBorderColor() const
{
    return m_normalIndicatorBorderColor;
}

void UKUIRadioButton::setNormalIndicatorBorderColor(const QBrush &newNormalIndicatorBorderColor)
{
    if (m_normalIndicatorBorderColor == newNormalIndicatorBorderColor)
        return;
    m_normalIndicatorBorderColor = newNormalIndicatorBorderColor;
    emit normalIndicatorBorderColorChanged();
}

const QBrush &UKUIRadioButton::hoverIndicatorBorderColor() const
{
    return m_hoverIndicatorBorderColor;
}

void UKUIRadioButton::setHoverIndicatorBorderColor(const QBrush &newHoverIndicatorBorderColor)
{
    if (m_hoverIndicatorBorderColor == newHoverIndicatorBorderColor)
        return;
    m_hoverIndicatorBorderColor = newHoverIndicatorBorderColor;
    emit hoverIndicatorBorderColorChanged();
}

const QBrush &UKUIRadioButton::clickIndicatorBorderColor() const
{
    return m_clickIndicatorBorderColor;
}

void UKUIRadioButton::setClickIndicatorBorderColor(const QBrush &newClickIndicatorBorderColor)
{
    if (m_clickIndicatorBorderColor == newClickIndicatorBorderColor)
        return;
    m_clickIndicatorBorderColor = newClickIndicatorBorderColor;
    emit clickIndicatorBorderColorChanged();
}

const QBrush &UKUIRadioButton::disableIndicatorBorderColor() const
{
    return m_disableIndicatorBorderColor;
}

void UKUIRadioButton::setDisableIndicatorBorderColor(const QBrush &newDisableIndicatorBorderColor)
{
    if (m_disableIndicatorBorderColor == newDisableIndicatorBorderColor)
        return;
    m_disableIndicatorBorderColor = newDisableIndicatorBorderColor;
    emit disableIndicatorBorderColorChanged();
}

const QBrush &UKUIRadioButton::checked_normalIndicatorColor() const
{
    return m_checked_normalIndicatorColor;
}

const QBrush &UKUIRadioButton::checked_hoverIndicatorColor() const
{
    return m_checked_hoverIndicatorColor;
}

const QBrush &UKUIRadioButton::checked_clickIndicatorColor() const
{
    return m_checked_clickIndicatorColor;
}

const QBrush &UKUIRadioButton::checked_disableIndicatorColor() const
{
    return m_checked_disableIndicatorColor;
}

const QBrush &UKUIRadioButton::checked_normalIndicatorBorderColor() const
{
    return m_checked_normalIndicatorBorderColor;
}

const QBrush &UKUIRadioButton::checked_hoverIndicatorBorderColor() const
{
    return m_checked_hoverIndicatorBorderColor;
}

const QBrush &UKUIRadioButton::checked_clickIndicatorBorderColor() const
{
    return m_checked_clickIndicatorBorderColor;
}

const QBrush &UKUIRadioButton::checked_disableIndicatorBorderColor() const
{
    return m_checked_disableIndicatorBorderColor;
}

const QBrush &UKUIRadioButton::checked_normalChildrenColor() const
{
    return m_checked_normalChildrenColor;
}

const QBrush &UKUIRadioButton::checked_hoverChildrenColor() const
{
    return m_checked_hoverChildrenColor;
}

const QBrush &UKUIRadioButton::checked_clickChildrenColor() const
{
    return m_checked_clickChildrenColor;
}


const QBrush &UKUIRadioButton::checked_disableChildrenColor() const
{
    return m_checked_disableChildrenColor;
}

const QBrush &UKUIRadioButton::checked_normalChildrenBorderColor() const
{
    return m_checked_normalChildrenBorderColor;
}

const QBrush &UKUIRadioButton::checked_hoverChildrenBorderColor() const
{
    return m_checked_hoverChildrenBorderColor;
}

const QBrush &UKUIRadioButton::checked_clickChildrenBorderColor() const
{
    return m_checked_clickChildrenBorderColor;
}


const QBrush &UKUIRadioButton::checked_disableChildrenBorderColor() const
{
    return m_checked_disableChildrenBorderColor;
}


void UKUIRadioButton::setChecked_NormalIndicatorColor(const QBrush &newChecked_normalIndicatorColor)
{
    if (m_checked_normalIndicatorColor == newChecked_normalIndicatorColor)
        return;
    m_checked_normalIndicatorColor = newChecked_normalIndicatorColor;
    emit checked_normalIndicatorColorChanged();
}

void UKUIRadioButton::setChecked_HoverIndicatorColor(const QBrush &newChecked_hoverIndicatorColor)
{
    if (m_checked_hoverIndicatorColor == newChecked_hoverIndicatorColor)
        return;
    m_checked_hoverIndicatorColor = newChecked_hoverIndicatorColor;
    emit checked_hoverIndicatorColorChanged();
}

void UKUIRadioButton::setChecked_ClickIndicatorColor(const QBrush &newChecked_clickIndicatorColor)
{
    if (m_checked_clickIndicatorColor == newChecked_clickIndicatorColor)
        return;
    m_checked_clickIndicatorColor = newChecked_clickIndicatorColor;
    emit checked_clickIndicatorColorChanged();
}

void UKUIRadioButton::setChecked_DisableIndicatorColor(const QBrush &newChecked_disableIndicatorColor)
{
    if (m_checked_disableIndicatorColor == newChecked_disableIndicatorColor)
        return;
    m_checked_disableIndicatorColor = newChecked_disableIndicatorColor;
    emit checked_disableIndicatorColorChanged();
}

void UKUIRadioButton::setChecked_NormalIndicatorBorderColor(const QBrush &newChecked_normalIndicatorBorderColor)
{
    if (m_checked_normalIndicatorBorderColor == newChecked_normalIndicatorBorderColor)
        return;
    m_checked_normalIndicatorBorderColor = newChecked_normalIndicatorBorderColor;
    emit checked_normalIndicatorBorderColorChanged();
}

void UKUIRadioButton::setChecked_HoverIndicatorBorderColor(const QBrush &newChecked_hoverIndicatorBorderColor)
{
    if (m_checked_hoverIndicatorBorderColor == newChecked_hoverIndicatorBorderColor)
        return;
    m_checked_hoverIndicatorBorderColor = newChecked_hoverIndicatorBorderColor;
    emit checked_hoverIndicatorBorderColorChanged();
}

void UKUIRadioButton::setChecked_ClickIndicatorBorderColor(const QBrush &newChecked_clickIndicatorBorderColor)
{
    if (m_checked_clickIndicatorBorderColor == newChecked_clickIndicatorBorderColor)
        return;
    m_checked_clickIndicatorBorderColor = newChecked_clickIndicatorBorderColor;
    emit checked_clickIndicatorBorderColorChanged();
}

void UKUIRadioButton::setChecked_DisableIndicatorBorderColor(const QBrush &newChecked_disableIndicatorBorderColor)
{
    if (m_checked_disableIndicatorBorderColor == newChecked_disableIndicatorBorderColor)
        return;
    m_checked_disableIndicatorBorderColor = newChecked_disableIndicatorBorderColor;
    emit checked_disableIndicatorBorderColorChanged();
}

void UKUIRadioButton::setChecked_NormalChildrenColor(const QBrush &newChecked_normalChildrenColor)
{
    if (m_checked_normalChildrenColor == newChecked_normalChildrenColor)
        return;
    m_checked_normalChildrenColor = newChecked_normalChildrenColor;
    emit checked_normalChildrenColorChanged();
}

void UKUIRadioButton::setChecked_HoverChildrenColor(const QBrush &newChecked_hoverChildrenColor)
{
    if (m_checked_hoverChildrenColor == newChecked_hoverChildrenColor)
        return;
    m_checked_hoverChildrenColor = newChecked_hoverChildrenColor;
    emit checked_hoverChildrenColorChanged();
}

void UKUIRadioButton::setChecked_ClickChildrenColor(const QBrush &newChecked_clickChildrenColor)
{
    if (m_checked_clickChildrenColor == newChecked_clickChildrenColor)
        return;
    m_checked_clickChildrenColor = newChecked_clickChildrenColor;
    emit checked_clickChildrenColorChanged();
}

void UKUIRadioButton::setChecked_DisableChildrenColor(const QBrush &newChecked_disableChildrenColor)
{
    if (m_checked_disableChildrenColor == newChecked_disableChildrenColor)
        return;
    m_checked_disableChildrenColor = newChecked_disableChildrenColor;
    emit checked_disableChildrenColorChanged();
}

void UKUIRadioButton::setChecked_NormalChildrenBorderColor(const QBrush &newChecked_normalChildrenBorderColor)
{
    if (m_checked_normalChildrenBorderColor == newChecked_normalChildrenBorderColor)
        return;
    m_checked_normalChildrenBorderColor = newChecked_normalChildrenBorderColor;
    emit checked_normalChildrenBorderColorChanged();
}

void UKUIRadioButton::setChecked_HoverChildrenBorderColor(const QBrush &newChecked_hoverChildrenBorderColor)
{
    if (m_checked_hoverChildrenBorderColor == newChecked_hoverChildrenBorderColor)
        return;
    m_checked_hoverChildrenBorderColor = newChecked_hoverChildrenBorderColor;
    emit checked_hoverChildrenBorderColorChanged();
}

void UKUIRadioButton::setChecked_ClickChildrenBorderColor(const QBrush &newChecked_clickChildrenBorderColor)
{
    if (m_checked_clickChildrenBorderColor == newChecked_clickChildrenBorderColor)
        return;
    m_checked_clickChildrenBorderColor = newChecked_clickChildrenBorderColor;
    emit checked_clickChildrenBorderColorChanged();
}

void UKUIRadioButton::setChecked_DisableChildrenBorderColor(const QBrush &newChecked_disableChildrenBorderColor)
{
    if (m_checked_disableChildrenBorderColor == newChecked_disableChildrenBorderColor)
        return;
    m_checked_disableChildrenBorderColor = newChecked_disableChildrenBorderColor;
    emit checked_disableChildrenBorderColorChanged();
}

int UKUIRadioButton::borderWidth() const
{
    return m_borderWidth;
}

void UKUIRadioButton::setBorderWidth(int newBorderWidth)
{
    if (m_borderWidth == newBorderWidth)
        return;
    m_borderWidth = newBorderWidth;
    emit borderWidthChanged();
}

int UKUIRadioButton::indicatorWidth() const
{
    return m_indicatorWidth;
}

void UKUIRadioButton::setIndicatorWidth(int newIndicatorWidth)
{
    if (m_indicatorWidth == newIndicatorWidth)
        return;
    m_indicatorWidth = newIndicatorWidth;
    emit indicatorWidthChanged();
}

int UKUIRadioButton::childrenWidth() const
{
    return m_childrenWidth;
}

void UKUIRadioButton::setChildrenWidth(int newChildrenWidth)
{
    if (m_childrenWidth == newChildrenWidth)
        return;
    m_childrenWidth = newChildrenWidth;
    emit childrenWidthChanged();
}

const QBrush &UKUIRadioButton::normalTransparentIndicatorColor() const
{
    return m_normalTransparentIndicatorColor;
}

void UKUIRadioButton::setNormalTransparentIndicatorColor(const QBrush &newNormalTransparentIndicatorColor)
{
    if (m_normalTransparentIndicatorColor == newNormalTransparentIndicatorColor)
        return;
    m_normalTransparentIndicatorColor = newNormalTransparentIndicatorColor;
    emit normalTransparentIndicatorColorChanged();
}

const QBrush &UKUIRadioButton::hoverTransparentIndicatorColor() const
{
    return m_hoverTransparentIndicatorColor;
}

void UKUIRadioButton::setHoverTransparentIndicatorColor(const QBrush &newHoverTransparentIndicatorColor)
{
    if (m_hoverTransparentIndicatorColor == newHoverTransparentIndicatorColor)
        return;
    m_hoverTransparentIndicatorColor = newHoverTransparentIndicatorColor;
    emit hoverTransparentIndicatorColorChanged();
}

const QBrush &UKUIRadioButton::clickTransparentIndicatorColor() const
{
    return m_clickTransparentIndicatorColor;
}

void UKUIRadioButton::setClickTransparentIndicatorColor(const QBrush &newClickTransparentIndicatorColor)
{
    if (m_clickTransparentIndicatorColor == newClickTransparentIndicatorColor)
        return;
    m_clickTransparentIndicatorColor = newClickTransparentIndicatorColor;
    emit clickTransparentIndicatorColorChanged();
}

const QBrush &UKUIRadioButton::disableTransparentIndicatorColor() const
{
    return m_disableTransparentIndicatorColor;
}

void UKUIRadioButton::setDisableTransparentIndicatorColor(const QBrush &newDisableTransparentIndicatorColor)
{
    if (m_disableTransparentIndicatorColor == newDisableTransparentIndicatorColor)
        return;
    m_disableTransparentIndicatorColor = newDisableTransparentIndicatorColor;
    emit disableTransparentIndicatorColorChanged();
}

const QBrush &UKUIRadioButton::normalTransparentIndicatorBorderColor() const
{
    return m_normalTransparentIndicatorBorderColor;
}

void UKUIRadioButton::setNormalTransparentIndicatorBorderColor(const QBrush &newNormalTransparentIndicatorBorderColor)
{
    if (m_normalTransparentIndicatorBorderColor == newNormalTransparentIndicatorBorderColor)
        return;
    m_normalTransparentIndicatorBorderColor = newNormalTransparentIndicatorBorderColor;
    emit normalTransparentIndicatorBorderColorChanged();
}

const QBrush &UKUIRadioButton::hoverTransparentIndicatorBorderColor() const
{
    return m_hoverTransparentIndicatorBorderColor;
}

void UKUIRadioButton::setHoverTransparentIndicatorBorderColor(const QBrush &newHoverTransparentIndicatorBorderColor)
{
    if (m_hoverTransparentIndicatorBorderColor == newHoverTransparentIndicatorBorderColor)
        return;
    m_hoverTransparentIndicatorBorderColor = newHoverTransparentIndicatorBorderColor;
    emit hoverTransparentIndicatorBorderColorChanged();
}

const QBrush &UKUIRadioButton::clickTransparentIndicatorBorderColor() const
{
    return m_clickTransparentIndicatorBorderColor;
}

void UKUIRadioButton::setClickTransparentIndicatorBorderColor(const QBrush &newClickTransparentIndicatorBorderColor)
{
    if (m_clickTransparentIndicatorBorderColor == newClickTransparentIndicatorBorderColor)
        return;
    m_clickTransparentIndicatorBorderColor = newClickTransparentIndicatorBorderColor;
    emit clickTransparentIndicatorBorderColorChanged();
}

const QBrush &UKUIRadioButton::disableTransparentIndicatorBorderColor() const
{
    return m_disableTransparentIndicatorBorderColor;
}

void UKUIRadioButton::setDisableTransparentIndicatorBorderColor(const QBrush &newDisableTransparentIndicatorBorderColor)
{
    if (m_disableTransparentIndicatorBorderColor == newDisableTransparentIndicatorBorderColor)
        return;
    m_disableTransparentIndicatorBorderColor = newDisableTransparentIndicatorBorderColor;
    emit disableTransparentIndicatorBorderColorChanged();
}
