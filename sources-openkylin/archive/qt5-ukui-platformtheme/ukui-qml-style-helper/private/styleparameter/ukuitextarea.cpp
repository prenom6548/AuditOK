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

#include "ukuitextarea.h"
#include "qdebug.h"
using namespace UKUIQQC2Style;

UKUITextArea::UKUITextArea(QQuickItem *parent) : QQuickItem(parent)
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

UKUITextArea::~UKUITextArea()
{

}

UKUITextArea* UKUITextArea::qmlAttachedProperties(QObject* parent)
{
    auto p = qobject_cast<QQuickItem*>(parent);
    return new UKUITextArea(p);
}

void UKUITextArea::initParam(UKUIGlobalDTConfig::GlobalDTConfig* instance)
{
    setLeftRightPadding(16);
    setRadius(instance->kradiusNormal());
    setBorderWidth(instance->normalline());
    setFocusBorderWidth(instance->focusline());
    setNormalWidth(180);
    setNormalHeight(36);

    setInputNormalBC(instance->kComponentInput());
    setNormalBC(instance->kContainSecondaryNormal());
    setHoveredBC(instance->kContainSecondaryNormal());
    setClickedBC(instance->kContainSecondaryNormal());
    setDisableBC(instance->kContainSecondaryNormal());

    setNormalBorderColor(instance->kLineNormal());
    setHoverBorderColor(instance->kLineNormal());
    setDisableBorderColor(instance->kLineDisable());
    setFocusBorderColor(instance->kBrandNormal());

    setPlaceHolderNormalTextColor(instance->kFontPlaceholdertext());
    setPlaceHolderDisableTextColor(instance->kFontPlaceholdertextDisable());
    setNormalTextColor(instance->kFontPrimary());
    setDisableTextColor(instance->kFontPlaceholdertextDisable());

    setInputNormalTransparentBC(instance->kComponentInputAlpha());
    setNormalTransparentBC(instance->kContainSecondaryAlphaNormal());
    setHoveredTransparentBC(instance->kContainSecondaryAlphaNormal());
    setClickedTransparentBC(instance->kContainSecondaryAlphaNormal());
    setDisableTransparentBC(instance->kContainSecondaryAlphaNormal());

    emit parametryChanged();

}

double UKUITextArea::radius() const
{
    return m_radius;
}

void UKUITextArea::setRadius(double newRadius)
{
    if (qFuzzyCompare(m_radius, newRadius))
        return;
    m_radius = newRadius;
    emit radiusChanged();
}

double UKUITextArea::leftRightPadding() const
{
    return m_leftRightPadding;
}

void UKUITextArea::setLeftRightPadding(double newLeftRightPadding)
{
    if (qFuzzyCompare(m_leftRightPadding, newLeftRightPadding))
        return;
    m_leftRightPadding = newLeftRightPadding;
    emit leftRightPaddingChanged();
}

double UKUITextArea::upDownMargin() const
{
    return m_upDownMargin;
}

void UKUITextArea::setUpDownMargin(double newUpDownMargin)
{
    if (qFuzzyCompare(m_upDownMargin, newUpDownMargin))
        return;
    m_upDownMargin = newUpDownMargin;
    emit upDownMarginChanged();
}

double UKUITextArea::space() const
{
    return m_space;
}

void UKUITextArea::setSpace(double newSpace)
{
    if (qFuzzyCompare(m_space, newSpace))
        return;
    m_space = newSpace;
    emit spaceChange();
}

int UKUITextArea::normalWidth() const
{
    return m_normalWidth;
}

void UKUITextArea::setNormalWidth(int newNormalWidth)
{
    if (m_normalWidth == newNormalWidth)
        return;
    m_normalWidth = newNormalWidth;
    emit normalWidthChanged();
}

int UKUITextArea::normalHeight() const
{
    return m_normalHeight;
}

void UKUITextArea::setNormalHeight(int newNormalHeight)
{
    if (m_normalHeight == newNormalHeight)
        return;
    m_normalHeight = newNormalHeight;
    emit normalHeightChanged();
}

const QBrush &UKUITextArea::normalBC() const
{
    return m_normalBC;
}

void UKUITextArea::setNormalBC(const QBrush &newNormalBC)
{
    if (m_normalBC == newNormalBC)
        return;
    m_normalBC = newNormalBC;
    emit normalBCChanged();
}

const QBrush &UKUITextArea::clickedBC() const
{
    return m_clickedBC;
}

void UKUITextArea::setClickedBC(const QBrush &newClickedBC)
{
    if (m_clickedBC == newClickedBC)
        return;
    m_clickedBC = newClickedBC;
    emit clickedBCChanged();
}

const QBrush &UKUITextArea::hoveredBC() const
{
    return m_hoveredBC;
}

void UKUITextArea::setHoveredBC(const QBrush &newHoveredBC)
{
    if (m_hoveredBC == newHoveredBC)
        return;
    m_hoveredBC = newHoveredBC;
    emit hoveredBCChanged();
}

const QBrush &UKUITextArea::disableBC() const
{
    return m_disableBC;
}

void UKUITextArea::setDisableBC(const QBrush &newDisableBC)
{
    if (m_disableBC == newDisableBC)
        return;
    m_disableBC = newDisableBC;
    emit disableBCChanged();
}

const QBrush &UKUITextArea::disableTextColor() const
{
    return m_disableTextColor;
}

void UKUITextArea::setDisableTextColor(const QBrush &newDisableTextColor)
{
    if (m_disableTextColor == newDisableTextColor)
        return;
    m_disableTextColor = newDisableTextColor;
    emit disableTextColorChanged();
}

const QBrush &UKUITextArea::normalTextColor() const
{
    return m_normalTextColor;
}

void UKUITextArea::setNormalTextColor(const QBrush &newNormalTextColor)
{
    if (m_normalTextColor == newNormalTextColor)
        return;
    m_normalTextColor = newNormalTextColor;
    emit normalTextColorChanged();
}

int UKUITextArea::borderWidth() const
{
    return m_borderWidth;
}

void UKUITextArea::setBorderWidth(int newBorderWidth)
{
    if (m_borderWidth == newBorderWidth)
        return;
    m_borderWidth = newBorderWidth;
    emit borderWidthChanged();
}

int UKUITextArea::focusBorderWidth() const
{
    return m_focusBorderWidth;
}

void UKUITextArea::setFocusBorderWidth(int newFocusBorderWidth)
{
    if (m_focusBorderWidth == newFocusBorderWidth)
        return;
    m_focusBorderWidth = newFocusBorderWidth;
    emit focusBorderWidthChanged();
}

const QBrush &UKUITextArea::normalBorderColor() const
{
    return m_normalBorderColor;
}

void UKUITextArea::setNormalBorderColor(const QBrush &newNormalBorderColor)
{
    if (m_normalBorderColor == newNormalBorderColor)
        return;
    m_normalBorderColor = newNormalBorderColor;
    emit normalBorderColorChanged();
}

const QBrush &UKUITextArea::clickBorderColor() const
{
    return m_clickBorderColor;
}

void UKUITextArea::setClickBorderColor(const QBrush &newClickBorderColor)
{
    if (m_clickBorderColor == newClickBorderColor)
        return;
    m_clickBorderColor = newClickBorderColor;
    emit clickBorderColorChanged();
}

const QBrush &UKUITextArea::disableBorderColor() const
{
    return m_disableBorderColor;
}

void UKUITextArea::setDisableBorderColor(const QBrush &newDisableBorderColor)
{
    if (m_disableBorderColor == newDisableBorderColor)
        return;
    m_disableBorderColor = newDisableBorderColor;
    emit disableBorderColorChanged();
}

const QBrush &UKUITextArea::focusBorderColor() const
{
    return m_focusBorderColor;
}

void UKUITextArea::setFocusBorderColor(const QBrush &newFocusBorderColor)
{
    if (m_focusBorderColor == newFocusBorderColor)
        return;
    m_focusBorderColor = newFocusBorderColor;
    emit focusBorderColorChanged();
}

const QBrush &UKUITextArea::hoverBorderColor() const
{
    return m_hoverBorderColor;
}

void UKUITextArea::setHoverBorderColor(const QBrush &newHoverBorderColor)
{
    if (m_hoverBorderColor == newHoverBorderColor)
        return;
    m_hoverBorderColor = newHoverBorderColor;
    emit hoverBorderColorChanged();
}

const QBrush &UKUITextArea::placeHolderNormalTextColor() const
{
    return m_placeHolderNormalTextColor;
}

void UKUITextArea::setPlaceHolderNormalTextColor(const QBrush &newPlaceHolderNormalTextColor)
{
    if (m_placeHolderNormalTextColor == newPlaceHolderNormalTextColor)
        return;
    m_placeHolderNormalTextColor = newPlaceHolderNormalTextColor;
    emit placeHolderNormalTextColorChanged();
}

const QBrush &UKUITextArea::placeHolderDisableTextColor() const
{
    return m_placeHolderDisableTextColor;
}

void UKUITextArea::setPlaceHolderDisableTextColor(const QBrush &newPlaceHolderDisableTextColor)
{
    if (m_placeHolderDisableTextColor == newPlaceHolderDisableTextColor)
        return;
    m_placeHolderDisableTextColor = newPlaceHolderDisableTextColor;
    emit placeHolderDisableTextColorChanged();
}

const QBrush &UKUITextArea::inputNormalBC() const
{
    return m_inputNormalBC;
}

void UKUITextArea::setInputNormalBC(const QBrush &newInputNormalBC)
{
    if (m_inputNormalBC == newInputNormalBC)
        return;
    m_inputNormalBC = newInputNormalBC;
    emit inputNormalBCChanged();
}

const QBrush &UKUITextArea::normalTransparentBC() const
{
    return m_normalTransparentBC;
}

void UKUITextArea::setNormalTransparentBC(const QBrush &newNormalTransparentBC)
{
    if (m_normalTransparentBC == newNormalTransparentBC)
        return;
    m_normalTransparentBC = newNormalTransparentBC;
    emit normalTransparentBCChanged();
}

const QBrush &UKUITextArea::inputNormalTransparentBC() const
{
    return m_inputNormalTransparentBC;
}

void UKUITextArea::setInputNormalTransparentBC(const QBrush &newInputNormalTransparentBC)
{
    if (m_inputNormalTransparentBC == newInputNormalTransparentBC)
        return;
    m_inputNormalTransparentBC = newInputNormalTransparentBC;
    emit inputNormalTransparentBCChanged();
}

const QBrush &UKUITextArea::clickedTransparentBC() const
{
    return m_clickedTransparentBC;
}

void UKUITextArea::setClickedTransparentBC(const QBrush &newClickedTransparentBC)
{
    if (m_clickedTransparentBC == newClickedTransparentBC)
        return;
    m_clickedTransparentBC = newClickedTransparentBC;
    emit clickedTransparentBCChanged();
}

const QBrush &UKUITextArea::hoveredTransparentBC() const
{
    return m_hoveredTransparentBC;
}

void UKUITextArea::setHoveredTransparentBC(const QBrush &newHoveredTransparentBC)
{
    if (m_hoveredTransparentBC == newHoveredTransparentBC)
        return;
    m_hoveredTransparentBC = newHoveredTransparentBC;
    emit hoveredTransparentBCChanged();
}

const QBrush &UKUITextArea::disableTransparentBC() const
{
    return m_disableTransparentBC;
}

void UKUITextArea::setDisableTransparentBC(const QBrush &newDisableTransparentBC)
{
    if (m_disableTransparentBC == newDisableTransparentBC)
        return;
    m_disableTransparentBC = newDisableTransparentBC;
    emit disableTransparentBCChanged();
}
